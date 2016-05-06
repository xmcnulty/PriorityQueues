#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>
#include <resolv.h>
#include <regex.h>

#include "pth.h"

#include "test_common.h"

extern int getItemNumber(char*);
/*
 * 1) spawn bootServer, 2) spawn ticker, 3) spawn client, 
 * 4) client requests, 5) bootServer spawns server
 */

/*
 * The HTTP request handler
 */

#define MAXREQLINE 1024

int itemsProduced = 0;

static void *server(void *_arg)
{
  int fd = (int)((long)_arg);
  char caLine[MAXREQLINE];
  char str[1024];
  int n;

  /* read request */
  for (;;) {
    n = pth_readline(fd, caLine, MAXREQLINE);
    
    if (n < 0) {
        fprintf(stderr, "read error: errno=%d\n", errno);
        close(fd);
        return NULL;
    }
    if (n == 0)
        break;
    if (n == 1 && caLine[0] == '\n')
        break;
    caLine[n-1] = NUL;
  }

  /* simulate a little bit of processing ;) */
  pth_yield(NULL);

  /* generate response */
  sprintf(str, "HTTP/1.0 200 Ok\r\n"
               "Server: test_httpd\r\n"
               "Connection: close\r\n"
               "Content-type: text/plain\r\n"
               "\r\n"
               "itemsProduced: %d\r\n", itemsProduced);

  
  pth_write(fd, str, strlen(str));

  /* close connection and let thread die */
  close(fd);
  
  return NULL;
}

#define MY_PORT         8080
#define SERVER_ADDR     "127.0.0.1"     /* aka localhost */
#define MAXBUF          1024

static void *client(void *arg)
{   
	int sockfd;
  struct sockaddr_in dest;
  char buffer[MAXBUF];

  /*---Open socket for streaming---*/
  if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
    perror("Socket");
    exit(errno);
  }

  /*---Initialize server address/port struct---*/
  bzero(&dest, sizeof(dest));
  dest.sin_family = AF_INET;
  dest.sin_port = htons(MY_PORT);

  /* 
   * converts the Internet host address cp from the IPv4 numbers-and-dots notation into binary form (in network byte order) 
   * and stores it in the structure that inp points to.
   */
  if (inet_aton(SERVER_ADDR, &dest.sin_addr) == 0 ) {
    perror(SERVER_ADDR);
    exit(errno);
  }

  /*---Connect to server---*/
  if ( pth_connect(sockfd, (struct sockaddr*)&dest, sizeof(dest)) != 0 ) {
    perror("Connect ");
		
    exit(errno);
  }
  
  sprintf(buffer, "GET %s HTTP/1.0\n\n", "/");
  pth_send(sockfd, buffer, strlen(buffer), 0);

  /*---Get "Hello?"---*/
  bzero(buffer, MAXBUF);
  pth_recv(sockfd, buffer, sizeof(buffer), 0);
  
  printf("Returned item: #%d\n", getItemNumber(buffer));

  /*---Clean up---*/
  close(sockfd);
    
  return 0;
}

/*
 * A useless ticker we let run just for fun in parallel
 */

static void *ticker(void *_arg)
{
	time_t now;
	char *ct;
	float avload;
	pth_attr_t attr;
	
  attr = pth_attr_new();
  pth_attr_set(attr, PTH_ATTR_NAME, "client");
  pth_attr_set(attr, PTH_ATTR_JOINABLE, FALSE);
  pth_attr_set(attr, PTH_ATTR_STACK_SIZE, 64*1024);

	for (;;) {

		pth_t tid;
	  void *val;
	  int rc;
  
	  tid = pth_spawn(attr, client, NULL);
    
		now = time(NULL);
		ct = ctime(&now);
		ct[strlen(ct)-1] = NUL;
		pth_ctrl(PTH_CTRL_GETAVLOAD, &avload);

		fprintf(stderr, "ticker woken up on %s, average load: %.2f\n", ct, avload);
    
    pth_sleep(5);
	}
	
  /* NOTREACHED */
  return NULL;
}

/*
 * And the server main procedure
 */

#if defined(FD_SETSIZE)
#define REQ_MAX FD_SETSIZE-100
#else
#define REQ_MAX 100
#endif

static int s;
pth_attr_t attr;

static void myexit(int sig)
{
  close(s);
  pth_attr_destroy(attr);
  pth_kill();
  fprintf(stderr, "**Break\n");
  exit(0);
}

void *bootServer(void *arg) {  
  struct sockaddr_in sar;
  struct protoent *pe;
  struct sockaddr_in peer_addr;
  socklen_t peer_len;
  int sr;
  int port;

  port = * (int *) arg;
  
  itemsProduced = 0;
  
  /* create TCP socket */
  if ((pe = getprotobyname("tcp")) == NULL) {
    perror("getprotobyname");
    exit(1);
  }
  if ((s = socket(AF_INET, SOCK_STREAM, pe->p_proto)) == -1) {
    perror("socket");
    exit(1);
  }

  /* bind socket to port */
  sar.sin_family      = AF_INET;
  sar.sin_addr.s_addr = INADDR_ANY;
  sar.sin_port        = htons(port);
  
  if (bind(s, (struct sockaddr *)&sar, sizeof(struct sockaddr_in)) == -1) {
    perror("socket");
    exit(1);
  }

  /* start listening on the socket with a queue of 10 */
  if (listen(s, REQ_MAX) == -1) {
    perror("listen");
    exit(1);
  }

  /* finally loop for requests */
  pth_attr_set(attr, PTH_ATTR_NAME, "server");

  for (;;) {
    /* accept next connection */
    peer_len = sizeof(peer_addr);
    
    if ((sr = pth_accept(s, (struct sockaddr *)&peer_addr, &peer_len)) == -1) {
      perror("accept");
      pth_sleep(1);
      
      continue;
    }
    
    if (pth_ctrl(PTH_CTRL_GETTHREADS) >= REQ_MAX) {
      fprintf(stderr, "currently no more connections acceptable\n");
      
      continue;
    }
    
    fprintf(stderr, "connection established (fd: %d, ip: %s, port: %d)\n", sr, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

    /* spawn new handling thread for connection */
          
    pth_spawn(attr, server, (void *)((long) sr));
    
    itemsProduced++;
    
  }
}

int getItemNumber(char* msgbuf) {
  regex_t    compiled_regex;   
  char       *pattern = "itemsProduced: \\([0-9]*\\).*";
  int        rc;     
  size_t     nmatch = 2; // pmatch[0] contains the msgbuf itself, pmatch[1] is the 1st submsgbuf
  regmatch_t pmatch[2];
  
  int        item = 0;
  
  if (0 != (rc = regcomp(&compiled_regex, pattern, 0))) {
    printf("regcomp() failed, returning nonzero (%d)\n", rc);
    exit(EXIT_FAILURE);
  }

  if (0 != (rc = regexec(&compiled_regex, msgbuf, nmatch, pmatch, 0))) {
    printf("Failed to match '%s' with '%s',returning %d.\n", msgbuf, pattern, rc);
  } else {
    char *numbuf = malloc(pmatch[1].rm_eo - pmatch[1].rm_so + 1);
    strncpy(numbuf, &msgbuf[pmatch[1].rm_so], pmatch[1].rm_eo - pmatch[1].rm_so);
    
    item = atoi(numbuf);
  }
  
  regfree(&compiled_regex);

  return item;
}    
    
int main(int argc, char *argv[])
{
  struct sockaddr_in sar;
  struct protoent *pe;
  struct sockaddr_in peer_addr;
  socklen_t peer_len;
  int sr;
  int port;

  /* initialize scheduler */
  pth_init();
  
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT,  myexit);
  signal(SIGTERM, myexit);

  /* argument line parsing */
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
  }
  port = atoi(argv[1]);
  if (port <= 0 || port >= 65535) {
    fprintf(stderr, "Illegal port: %d\n", port);
    exit(1);
  }

  attr = pth_attr_new();

  pth_attr_set(attr, PTH_ATTR_NAME, "server");
  pth_attr_set(attr, PTH_ATTR_JOINABLE, TRUE);
  pth_attr_set(attr, PTH_ATTR_STACK_SIZE, 64*1024);
  
  pth_t tid;
  void *val;
  int rc;
  
  tid = pth_spawn(attr, bootServer, (void *) &port);
	
  /* run a just for fun ticker thread */

  pth_attr_set(attr, PTH_ATTR_NAME, "ticker");
  pth_attr_set(attr, PTH_ATTR_JOINABLE, FALSE);
  pth_attr_set(attr, PTH_ATTR_STACK_SIZE, 64*1024);
  
  pth_spawn(attr, ticker, NULL);

  rc = pth_join(tid, &val);
  
  // kill ticker etc
}

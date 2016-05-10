#ifndef MY_SCHEDULERH
#define MY_SCHEDULERH

#include <signal.h>
#include "pth_acdef.h"
#include "pth_acmac.h"


#include "pth.h"
#include "pth_p.h"

#define TCB_NAMELEN 40

typedef struct xstate_st my_xstate_t;
struct xstate_st {
    sigjmp_buf jb;
    sigset_t sigs;
    int error;
};

#define my_xstate_save(xstate) \
        ( (xstate)->error = errno, \
          pth_sigsetjmp((xstate)->jb) )


#define my_xstate_restore(xstate) \
        ( errno = (xstate)->error, \
          (void)pth_siglongjmp((xstate)->jb, 1) )

#define my_xstate_switch(old,new) \
    if (my_xstate_save(old) == 0) \
        my_xstate_restore(new); \
    // pth_mctx_restored(old);

    /* the unique thread id/handle */
typedef struct my_tcb_st *my_tcb_t;
struct my_tcb_st;

/* thread priority queue */
struct my_pqueue_st {
    my_tcb_t  q_head;
    int     q_num;
};
typedef struct my_pqueue_st my_pqueue_t;

#define my_pqueue_favorite_prio(q) \
    ((q)->q_head != NULL ? (q)->q_head->q_prio + 1 : PRIO_MAX)
      
#define my_pqueue_elements(q) \
    ((q) == NULL ? (-1) : (q)->q_num)

#define my_pqueue_head(q) \
    ((q) == NULL ? NULL : (q)->q_head)

typedef void *my_event_t;
typedef void *my_time_t;
typedef void *my_ring_t;

typedef struct my_cleanup_st my_cleanup_t;
struct my_cleanup_st {
    my_cleanup_t *next;
    void (*func)(void *);
    void *arg;
};
    /* thread states */
typedef enum state_en {
    MY_STATE_SCHEDULER = 0,         /* the special scheduler thread only       */
    MY_STATE_NEW,                   /* spawned, but still not dispatched       */
    MY_STATE_READY,                 /* ready, waiting to be dispatched         */
    MY_STATE_WAITING,               /* suspended, waiting until event occurred */
    MY_STATE_DEAD                   /* terminated, waiting to be joined        */
} my_state_t;

    /* thread control block */
struct my_tcb_st {
  /* priority queue handling */
  
  my_tcb_t       q_next;               /* next thread in pool                         */
  my_tcb_t       q_prev;               /* previous thread in pool                     */
  int            q_prio;               /* (relative) priority of thread when queued   */

  /* standard thread control block ingredients */
  int            prio;                 /* base priority of thread                     */
  char           name[TCB_NAMELEN];/* name of thread (mainly for debugging)       */
  int            dispatches;           /* total number of thread dispatches           */
  my_state_t     state;                /* current state indicator for thread          */

  /* timing */
  my_time_t     spawned;              /* time point at which thread was spawned      */
  my_time_t     lastran;              /* time point at which thread was last running */
  my_time_t     running;              /* time range the thread was already running   */

  /* event handling */
  my_event_t    events;               /* events the tread is waiting for             */

  /* per-thread signal handling */
  sigset_t       sigpending;           /* set    of pending signals                   */
  int            sigpendcnt;           /* number of pending signals                   */

  /* machine context */
  my_xstate_t    xstate;               /* last saved machine state of thread          */
  char          *stack;                /* pointer to thread stack                     */
  unsigned int   stacksize;            /* size of thread stack                        */
  long          *stackguard;           /* stack overflow guard                        */
  int            stackloan;            /* stack type                                  */
  void        *(*start_func)(void *);  /* start routine                               */
  void          *start_arg;            /* start argument                              */

  /* thread joining */
  int            joinable;             /* whether thread is joinable                  */
  void          *join_arg;             /* joining argument                            */

  /* per-thread specific storage */
  const void   **data_value;           /* thread specific  values                     */
  int            data_count;           /* number of stored values                     */

  /* cancellation support */
  int            cancelreq;            /* cancellation request is pending             */
  unsigned int   cancelstate;          /* cancellation state of thread                */
  my_cleanup_t  *cleanups;             /* stack of thread cleanup handlers            */

  /* mutex ring */
  my_ring_t     mutexring;            /* ring of aquired mutex structures            */
};


    /* thread priority values */
#define MY_PRIO_MAX                 +5
#define MY_PRIO_STD                  0
#define MY_PRIO_MIN                 -5

extern my_tcb_t my_pqueue_tail(my_pqueue_t *);
extern void my_pqueue_delete(my_pqueue_t *, my_tcb_t t);
extern void my_pqueue_insert(my_pqueue_t *, int, my_tcb_t);
extern void my_pqueue_insert(my_pqueue_t *, int, my_tcb_t);
extern my_tcb_t my_pqueue_delmax(my_pqueue_t *);
extern void my_pqueue_increase(my_pqueue_t *q);

extern void my_tcb_free(my_tcb_t t);
    
void my_move_threads_from_newq_to_readyq(my_pqueue_t *nq, my_pqueue_t *rq);
my_tcb_t my_find_next_thread_to_schedule(my_pqueue_t *rq);
void my_dispatcher(my_xstate_t* schedstate, my_xstate_t* currentstate);
void my_handle_dead_thread(my_tcb_t current, my_pqueue_t *dq);
void my_handle_waiting_thread(my_tcb_t current, my_pqueue_t *wq);
void my_refresh_readyq(my_tcb_t current, my_pqueue_t *rq);

/* part 3 implementations */
int my_yield(my_tcb_t to);
int my_join(my_tcb_t tid);

#endif

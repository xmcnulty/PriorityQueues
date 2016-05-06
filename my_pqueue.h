//
// Created by Xavier McNulty on 4/28/16.
//

#ifndef PRIORITYQUEUES_MY_PQUEUE_H
#define PRIORITYQUEUES_MY_PQUEUE_H

#define MY_PRIO_MAX +5
#define MY_PRIO_STD  0
#define MY_PRIO_MIN -5

#define PTH_WALK_NEXT   _BIT(1)
#define PTH_WALK_PREV   _BIT(2)

#define _BIT(n) (1<<(n))

typedef struct my_tcb_t *my_tcb;

struct my_tcb_t {
    int my_tcb;

    int priority;

    my_tcb next;
    my_tcb prev;
};

/* thread priority queue */
struct my_pqueue_st {
    my_tcb q_head;
    int q_num;

    int size;
};
typedef struct my_pqueue_st my_pqueue_t;

/* initialize a priority queue; O(1) */
void my_pqueue_init(my_pqueue_t *q);

/* insert thread into priority queue; O(n) */
void my_pqueue_insert(my_pqueue_t *q, int prio, my_tcb t);

/* remove thread with maximum priority from priority queue; O(1) */
my_tcb my_pqueue_delmax(my_pqueue_t *q);

/* remove thread from priority queue; O(n) */
void my_pqueue_delete(my_pqueue_t *q, my_tcb t);

/* determine priority required to favorite a thread; O(1)
 * returns either greatest priority in the queue + 1 or PRIO_MAX */
#define my_pqueue_favorite_prio(q) \
    if (!q || !q->q_head) \
        MY_PRIO_MIN \
    else \
        q->q_head->priority

/* move a thread inside a queue to the top; O(n) */
int my_pqueue_favorite(my_pqueue_t *q, my_tcb t);

/* increase priority of all(!) threads in queue; O(1) */
void my_queue_increase(my_pqueue_t *q);

/* returns the number of elements in the q. if the q is empty return -1 */
#define my_queue_elements_(q) \
    if (q->size == 0) -1 \
    else q->size

/* returns teh my_tcb at the tail of the q */
#define my_pqueue_head(q) \
    if (!q) NULL \
    else q->q_head

/* returns the my_tcb at the tail of the q */
my_tcb my_pqueue_tail(my_pqueue_t *q);

/* walk to next or previous thread in queue; O(1) */
my_tcb my_pqueue_walk(my_pqueue_t *q, my_tcb t, int direction);

/* check whether a thread is in a queue: O(n) */
int my_pqueue_contains(my_pqueue_t *q, my_tcb t);


#endif //PRIORITYQUEUES_MY_PQUEUE_H

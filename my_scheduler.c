#include "my_scheduler.h"

int favornew = 1;

void my_move_threads_from_newq_to_readyq(my_pqueue_t *nq, my_pqueue_t *rq) {
    my_tcb_t cur = my_pqueue_tail(nq);

    while (cur) {
        my_pqueue_delete(nq, cur);

        if (favornew)
    }
}

/* returns next thread to schedule */
my_tcb_t my_find_next_thread_to_schedule(my_pqueue_t *rq) {
    return rq->q_head;
}

void my_dispatcher(my_xstate_t* schedstate, my_xstate_t* currentstate){
}

void my_handle_dead_thread(my_tcb_t current, my_pqueue_t *dq) {
}

void my_handle_waiting_thread(my_tcb_t current, my_pqueue_t *wq) {
}

void my_refresh_readyq(my_tcb_t current, my_pqueue_t *rq) {
}

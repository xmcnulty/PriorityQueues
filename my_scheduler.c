#include "my_scheduler.h"

int favornew = 1;

void my_move_threads_from_newq_to_readyq(my_pqueue_t *nq, my_pqueue_t *rq) {
    my_tcb_t cur = my_pqueue_tail(nq);

    while (my_pqueue_elements(nq) != 0) {
        my_pqueue_delete(nq, cur);

		int priority;
		
        if (favornew) {
			priority = my_pqueue_favorite_prio(rq);
		} else {
			priority = MY_PRIO_STD;
		}

        // set the threads state to MY_STATE_READY
        cur->state = MY_STATE_READY;

		my_pqueue_insert(rq, priority, cur);
		
		cur = cur->q_prev;
    }
}

/* returns next thread to schedule */
my_tcb_t my_find_next_thread_to_schedule(my_pqueue_t *rq) {
    if (!rq || my_pqueue_elements(rq) == 0)
        return NULL;

    my_tcb_t fav = my_pqueue_delmax(rq);

    return fav;
}

void my_dispatcher(my_xstate_t* schedstate, my_xstate_t* currentstate){
	if (!schedstate || !currentstate)
		return;
	
	my_xstate_switch(schedstate, currentstate);
}

/* Handles a dead thread. If the thread is not joinable call my_tcb_free,
	otherwise insert the thread to the dead threads queue with MY_PRIO_STD */
void my_handle_dead_thread(my_tcb_t current, my_pqueue_t *dq) {
	if (!current->joinable) {
		my_tcb_free(current);
	} else {
		my_pqueue_insert(dq, MY_PRIO_STD, current);
	}
}

/* Just insert the current thread into the waiting queue (WQ) preserving its current priority */
void my_handle_waiting_thread(my_tcb_t current, my_pqueue_t *wq) {
		my_pqueue_insert(wq, current->q_prio, current);
}

/* Increase the priority of each thread in the ready queue so it'll eventually get chosen to run.
	If the current thread isn't null it must also be inserted into the ready queue with it's current.
	priority */
void my_refresh_readyq(my_tcb_t current, my_pqueue_t *rq) {
	my_pqueue_increase(rq);
	
	if (current) {
		my_pqueue_insert(rq, current->q_prio, current);
	}
}

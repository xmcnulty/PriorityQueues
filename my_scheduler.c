#include "my_scheduler.h"

int favornew = 1;

void my_move_threads_from_newq_to_readyq(my_pqueue_t *nq, my_pqueue_t *rq) {
    my_tcb_t cur = my_pqueue_tail(nq);

    while (cur) {
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
		
		cur = my_pqueue_walk(nq, cur, MY_WALK_PREV);
    }
}

/* returns next thread to schedule */
my_tcb_t my_find_next_thread_to_schedule(my_pqueue_t *rq) {
    if (!rq)
        return NULL;

    my_tcb_t fav = my_find_next_thread_to_schedule(rq);

    if(fav)
        my_pqueue_delete(rq, fav);

    return fav;
}

void my_dispatcher(my_xstate_t* schedstate, my_xstate_t* currentstate){
	my_xstate_switch(currentstate, schedstate);
}

/* Handles a dead thread. If the thread is not joinable call my_tcb_free,
	otherwise insert the thread to the dead threads queue with MY_PRIO_STD */
void my_handle_dead_thread(my_tcb_t current, my_pqueue_t *dq) {
	if (!current->joinable) {
		my_tcb_free(current);
	} else {
		my_queue_insert(dq, MY_PRIO_STD, current);
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

/* switches execution state from the current thread to the scheduler thread */
int my_yield(my_tcb_t to) {
    my_tcb_t scheduled = my_get_thread_scheduler();
    my_tcb_t current = my_get_thread_current();

    if (current == NULL || scheduled == NULL)
        return 0;

    my_dispatcher(scheduled->xstate, current->xstate);

    // if to thread is to be scheduled
    if (to) {
        if (to->state == MY_STATE_NEW && my_pqueue_contains(my_NQ, to)) {
            int scheduleStatus = my_pqueue_favorite(my_NQ, to);

            if (!scheduleStatus)
                return 0;
        } else if (to->state == MY_STATE_READY && my_pqueue_contains(my_RQ, to)) {
            int scheduleStatus = my_pqueue_favorite(my_RQ, to);

            if (!scheduleStatus)
                return 0;
        } else {
            return 0;
        }
    }

    return 1;
}
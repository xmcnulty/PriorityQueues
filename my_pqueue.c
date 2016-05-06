//
// Created by Xavier McNulty on 4/28/16.
//

#include "my_pqueue.h"
#include <stdlib.h>
#include <stdio.h>

/* initialize a priority queue; O(1) */
void my_pqueue_init(my_pqueue_t *q) {
    q = malloc(sizeof(my_pqueue_t));

    q->size = 0;
}

/* insert thread into priority queue; O(n) */
void my_pqueue_insert(my_pqueue_t *q, int prio, my_tcb t) {
    if (q == NULL)
        return;

    t->priority = prio;

    // case: q is empty
    if (!q->q_head) {
        q->q_head = t;

        return;
    }

    my_tcb cur = q->q_head, prev = 0;

    // find where the tcb should be inserted.
    while (cur && cur->priority >= prio) {
        prev = cur;
        cur = cur->next;
    }

    // case: insert at head
    if (!prev) {
        cur->prev = t;
        t->next = cur;

        q->q_head = t;
    }
    // case: insert at tail
    else if (!cur) {
        prev->next = t;

        t->prev = prev;
    }
    // case: insert between two tcb's
    else {
        prev->next = t;
        cur->prev = t;
    }

    q->size ++;
}

/* remove thread with maximum priority from priority queue; O(1) */
my_tcb my_pqueue_delmax(my_pqueue_t *q) {
    // return empty if no queue or queue is empty
    if (!q || !q->q_head)
        return NULL;

    my_tcb head = q->q_head;

    q->q_head = head->next;

    q->size --;

    return head;
}

/* remove thread from priority queue; O(n) */
void my_pqueue_delete(my_pqueue_t *q, my_tcb t) {
    if (!q || !q->q_head)
        return;

    my_tcb cur = q->q_head;

    while (cur && cur != t)
        cur = cur->next;

    if (!cur) // t not in queue
        return;

    if (cur->next)
        cur->next->prev = cur->prev;
    if (cur->prev)
        cur->prev->next = cur->next;

    q->size --;
}

/* move a thread inside a queue to the top; O(n) */
int my_pqueue_favorite(my_pqueue_t *q, my_tcb t) {
    if (!q || !q->q_head)
        return 0;

    my_tcb cur = q->q_head;

    while (cur && cur != t)
        cur = cur->next;

    if (!cur)
        return 0;

    // remove thread from it's current position
    if (cur->next)
        cur->next->prev = cur->prev;
    if (cur->prev)
        cur->prev->next = cur->next;

    // put cur at the head of the queue
    q->q_head->prev = cur;
    cur->next = q->q_head;

    cur->prev = NULL;

    q->q_head = cur;

    return 1;
}

/* increase priority of all(!) threads in queue; O(1) */
void my_queue_increase(my_pqueue_t *q) {
    if (!q)
        return;

    my_tcb cur = q->q_head;

    while (cur) {
        if (cur->priority < MY_PRIO_MAX)
            cur->priority ++;

        cur = cur->next;
    }
}

/* returns the my_tcb at the tail of the q */
my_tcb my_pqueue_tail(my_pqueue_t *q) {
    if (!q || !q->q_head)
        return NULL;

    my_tcb cur = q->q_head;

    while (cur->next)
        cur = cur->next;

    return cur;
}

/* walk to next or previous thread in queue; O(1) */
my_tcb my_pqueue_walk(my_pqueue_t *q, my_tcb t, int direction) {
    if (!q || !q->q_head)
        return NULL;

    my_tcb cur = q->q_head;

    while (cur) {
        if (direction == PTH_WALK_PREV && cur->next == t)
            return cur;
        else if (direction == PTH_WALK_NEXT && cur == t)
            return cur->next;

        cur = cur->next;
    }

    return NULL;
}

/* check whether a thread is in a queue: O(n) */
int my_pqueue_contains(my_pqueue_t *q, my_tcb t) {
    if (!q || !q->q_head)
        return 0;

    my_tcb cur = q->q_head;

    while (cur) {
        if (cur == t)
            return 1;

        cur = cur->next;
    }

    return 0;
}
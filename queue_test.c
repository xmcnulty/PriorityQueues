//
// Created by Xavier McNulty on 5/2/16.
//

#include <stdio.h>
#include <stdlib.h>
#include "my_pqueue.h"

/* prints a tcb */
void printTCB(my_tcb tcb) {
    if (tcb)
        printf("(%d [%d])", tcb->my_tcb, tcb->priority);
    else
        printf("null");
}

/* prints a priority queue */
void printQueue (my_pqueue_t *q) {
    if (q) {
        my_tcb cur = q->q_head;

        while (cur) {
            printTCB(cur);

            printf(" <-> ");

            cur = cur->next;
        }
    }
}

/* main method */
int main(int argc, char **argv) {
    my_pqueue_t q;

    my_pqueue_init(&q);

    my_tcb tcb = malloc(sizeof(tcb));

    tcb->priority = MY_PRIO_STD;
    tcb->my_tcb = 10;

    my_pqueue_insert(&q, MY_PRIO_STD, tcb);

    printQueue(&q);
    printf("\n");
}
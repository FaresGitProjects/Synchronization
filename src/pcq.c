#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "csesem.h"
#include "pcq.h"

/* This structure must contain everything you need for an instance of a
 * PCQueue.  The given definition is ABSOLUTELY NOT COMPLETE.  You will
 * have to add several items to this structure. */
struct PCQueue {
    int slots;
    void **queue;
    int rear;
    int front;
    
    CSE_Semaphore q_lock;  // Protects queue
    CSE_Semaphore s_count; // Counts slots
    CSE_Semaphore i_count; // Counts items
};

/* The given implementation performs no error checking and simply
 * allocates the queue itself.  You will have to create and initialize
 * (appropriately) semaphores, mutexes, condition variables, flags,
 * etc. in this function. */
PCQueue pcq_create(int slots) {
    PCQueue pcq;
    
    pcq = calloc(1, sizeof(*pcq));
    pcq->queue = calloc(slots, sizeof(void *));
    pcq->slots = slots;


    pcq->rear = pcq->front = 0;
    
    pcq-> q_lock = csesem_create(1);
    pcq-> s_count = csesem_create(slots);
    pcq-> i_count = csesem_create(0);
    return pcq;
}

/* This implementation does nothing, as there is not enough information
 * in the given struct PCQueue to even usefully insert a pointer into
 * the data structure. */

void pcq_insert(PCQueue pcq, void *data) {
    csesem_wait(pcq->s_count);
    csesem_wait(pcq->q_lock);
    
    pcq->queue[(++pcq->rear)%pcq->slots] = data;

    
    csesem_post(pcq->q_lock);
    csesem_post(pcq->i_count);
}

/* This implementation does nothing, for the same reason as
 * pcq_insert(). */
void *pcq_retrieve(PCQueue pcq) {
    void *item = NULL;
    csesem_wait(pcq->i_count);
    csesem_wait(pcq->q_lock);
        item = pcq->queue[(++pcq->front)%pcq->slots];
    csesem_post(pcq->q_lock);
    csesem_post(pcq->s_count);
    return item;
}

/* The given implementation blindly frees the queue.  A minimal
 * implementation of this will need to work harder, and ensure that any
 * synchronization primitives allocated here are destroyed; a complete
 * and correct implementation will have to synchronize with any threads
 * blocked in pcq_create() or pcq_destroy().
 *
 * You should implement the complete and correct clean teardown LAST.
 * Make sure your other operations work, first, as they will be tightly
 * intertwined with teardown and you don't want to be debugging it all
 * at once!
 */
void pcq_destroy(PCQueue pcq) {
    csesem_destroy(pcq->s_count);
    csesem_destroy(pcq->i_count);
    csesem_destroy(pcq->q_lock);
    
    free(pcq->queue);
    free(pcq);
}

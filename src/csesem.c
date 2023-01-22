#include <pthread.h>
#include <stdlib.h>

#include "csesem.h"

/* This definition of struct CSE_Semaphore is only available _inside_
 * your semaphore implementation.  This prevents calling code from
 * inadvertently invalidating the internal representation of your
 * semaphore.  See csesem.h for more information.
 *
 * You may place any data you require in this structure. */
struct CSE_Semaphore {
    int limit;
    int nuse;
    int isDying;
    pthread_mutex_t sem_lock; // Protect nuse
    pthread_cond_t sem_cond;
};

/* This function must both allocate space for the semaphore and perform
 * any initialization that is required for safe operation on the
 * semaphore.  The user should be able to immediately call csesem_post()
 * or csesem_wait() after this routine returns. */
CSE_Semaphore csesem_create(int count) {
    CSE_Semaphore sem = calloc(1, sizeof(struct CSE_Semaphore));
    sem->isDying = 0;
    sem->limit = count;
    sem->nuse = count;
    pthread_mutex_init(&(sem->sem_lock), NULL);
    return sem;
}

void csesem_post(CSE_Semaphore sem) {
    if(sem->isDying) {return;}
    pthread_mutex_lock(&sem->sem_lock);
    sem->nuse += 1;
    pthread_mutex_unlock(&sem->sem_lock);
    pthread_cond_signal(&sem->sem_cond);
}

void csesem_wait(CSE_Semaphore sem) {
    pthread_mutex_lock(&sem->sem_lock);
    while(sem->nuse == 0) {
        pthread_cond_wait(&sem->sem_cond, &sem->sem_lock);
            if(sem->isDying){
                pthread_mutex_unlock(&sem->sem_lock);
                return;
            }
    }
    sem->nuse -= 1;
    pthread_mutex_unlock(&sem->sem_lock);
    pthread_cond_signal(&sem->sem_cond);
}

/* This function should destroy any resources allocated for this
 * semaphore; this includes mutexes or condition variables. */
void csesem_destroy(CSE_Semaphore sem) {
    sem->isDying = 1;
    pthread_cond_broadcast(&sem->sem_cond);
    pthread_mutex_destroy(&sem->sem_lock);
    pthread_cond_destroy(&sem->sem_cond);
    free(sem);
}

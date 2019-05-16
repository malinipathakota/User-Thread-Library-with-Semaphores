#include <stddef.h>
#include <stdlib.h>
#include "queue.h"
#include "sem.h"
#include "thread.h"

/*struct holds queue of waiting threads along with the size of the queue
or number of threads inside */
struct semaphore {
	queue_t queue;
	size_t size;
};

/*sem_getvalue gets the value of the count of threads inside the queue,
if If semaphore @sems's internal count is equal to 0, assign a negative number
whose absolute value is the count of the number of threads currently blocked*/
int sem_getvalue(sem_t sem, int *sval) {
	if(sem == NULL) {
		return -1;
	} 
	if(sem->size > 0) {
		*sval = sem->size;
	}
	if(sem == 0) {
		*sval = (queue_length(sem->queue)) * -1;
	}
	return 0;
}

/*sem_create allocates space for an instance of semaphore, creates
the queue, and populates the size with @count */
sem_t sem_create(size_t count) {
	sem_t sem = (sem_t)malloc(sizeof(sem_t));
	sem->size = count;
	sem->queue = queue_create();

	if(sem->queue == NULL || sem == NULL) {
		return NULL;
	}
	return sem;
}

/*sem_destroy destroys the queue by freeing its memory only if
the length of the queue is equal to zero*/
int sem_destroy(sem_t sem) {
	if (queue_length(sem->queue) > 0 || sem == NULL) {
		return -1;
	}
	else {
		queue_destroy(sem->queue); 
		free(sem);
	}
	return 0; 
}

/*sem_down takes a resource from semaphore @sem.*/
int sem_down(sem_t sem) {
	if(sem == NULL) {
		return -1;
	}
	enter_critical_section();
	while(sem->size == 0) {
		pthread_t id = pthread_self();
		queue_enqueue(sem->queue, (void*)id);
		thread_block();
	}	
	sem->size--;

	exit_critical_section(); 
	return 0;
}

/*Release a resource to semaphore @sem.*/
int sem_up(sem_t sem) {
	if(sem == NULL) {
		return -1;
	}
	enter_critical_section();
	sem->size++;
	if(queue_length(sem->queue) > 0) { 
		pthread_t tid;
		queue_dequeue(sem->queue, (void**)&tid);
		thread_unblock(tid);
	}	

	exit_critical_section();
	return 0;
}

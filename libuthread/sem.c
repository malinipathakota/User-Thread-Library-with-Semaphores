#include <stddef.h>
#include <stdlib.h>
#include "queue.h"
#include "sem.h"
#include "thread.h"

struct semaphore {
	queue_t queue;
	size_t size;
};

int sem_getvalue(sem_t sem, int *sval) {
	if (sem->size > 0) {
		*sval = sem->size;
	}
	if(sem == NULL) {
		return -1;
	} 
	else {
		*sval = (queue_length(sem->queue)) * -1;
	}
	return 0;
}

sem_t sem_create(size_t count) {
	sem_t mysem = (sem_t)malloc(sizeof(sem_t));
	mysem->size = count;
	mysem->queue = queue_create();

	return mysem;
}

int sem_destroy(sem_t sem) {
	if (queue_length(sem->queue) != 0 || sem == NULL) {
		return -1;
	}
	queue_destroy(sem->queue); 
	free(sem);
	return 0; 
}

int sem_down(sem_t sem) {
	enter_critical_section();
	if(sem == NULL) {
		return -1;
	}
	while(sem->size == 0) {
		pthread_t tid = pthread_self();
		queue_enqueue(sem->queue, (void*)tid);
		thread_block();
	}	
	sem->size--;

	exit_critical_section(); 
	return 0;
}

int sem_up(sem_t sem) {
	enter_critical_section();
	if(sem == NULL) {
		return -1;
	}
	sem->size++;
	if(queue_length(sem->queue) > 0) { 
		pthread_t tid;
		queue_dequeue(sem->queue, (void**)&tid);
		thread_unblock(tid);
	}	

	exit_critical_section();
	return 0;
}

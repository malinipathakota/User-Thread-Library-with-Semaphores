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

sem_t sem_create(size_t count) {
	sem_t sem = (sem_t)malloc(sizeof(sem_t));
	sem->size = count;
	sem->queue = queue_create();

	if(sem->queue == NULL || sem == NULL) {
		return NULL;
	}
	return sem;
}

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

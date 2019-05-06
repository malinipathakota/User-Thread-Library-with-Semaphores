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
	return 0;
}

sem_t sem_create(size_t count) {
	sem_t sem;
	if(sem = (sem_t)malloc(sizeof(struct semaphore))) {
		sem->queue = queue_create();
		if(sem->queue == NULL) {
			return NULL;
		}
		sem->size = count;	
	}
	else {
		return NULL;
	}
	return sem;
}

int sem_destroy(sem_t sem) {
	enter_critical_section();
	if (queue_length(sem->queue) != 0 || sem == NULL) {
		exit_critical_section();
		return -1;
	}
	exit_critical_section();
	queue_destroy(sem->queue); 
	free(sem);
	return 0; 
}

int sem_down(sem_t sem) {
	enter_critical_section();
	if (sem == NULL) {
		exit_critical_section();
		return -1;
	}
	while (sem->size == 0) {
		pthread_t tid = pthread_self();
		queue_enqueue(sem->queue, &tid);	
		thread_block(); 	
	}	
	sem->size--;
	
	exit_critical_section();
	return 0;
}

int sem_up(sem_t sem) {
	return 0; 
}


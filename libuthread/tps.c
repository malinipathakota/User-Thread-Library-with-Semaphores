#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "queue.h"
#include "thread.h"
#include "tps.h"


struct tps_node
{
	pthread_t thread;
	void* memory_page;
};

typedef struct tps_node* tps_node_t;

bool initialized = false;

queue_t tps_queue;

int tps_init(int segv)
{
	tps_queue = queue_create();
	initialized = true;
	return 0;
}

int tps_create(void)
{
	//add error cases
	enter_critical_section();
	tps_init();
	tps_node_t new_node = (tps_node_t)malloc(sizeof(struct tps_node));
	if(new_node == NULL)
	{
		return -1;
		exit_critical_section();
	}
	new_node->thread = pthread_self();
	//last two parameters??
	new_node->memory_page = mmap(NULL, TPS_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0 , 0);
	queue_enqueue(tps_queue, (void*)new_node);
	exit_critical_section();
}

int tps_destroy(void)
{
	//do we free each node?
	enter_critical_section();
	if (queue_length(tps_queue) != 0){
		exit_critical_section();
		return -1;
	}
	exit_critical_section();
	queue_destroy(tps_queue); 
	return 0; 
}

int tps_read(size_t offset, size_t length, char *buffer)
{
	enter_critical_section();

}

int tps_write(size_t offset, size_t length, char *buffer)
{
	/* TODO: Phase 2 */
}

int tps_clone(pthread_t tid)
{
	/* TODO: Phase 2 */
}


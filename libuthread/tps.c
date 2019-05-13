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

static int find_item(void *data, void *arg)
{
    tps_node_t node = (tps_node_t)data;
	pthread_t thread = (pthread_t)arg;
    if (node->thread == thread)
	{
		 return 1;
	} 
    return 0;
}

static void segv_handler(int sig, siginfo_t *si, void *context)
{
	void *p_fault = (void*)((uintptr_t)si->si_addr & ~(TPS_SIZE - 1));
	tps_node_t matched_node;
	//iterate through TPS areas and find if p_fault matches one of them
	queue_iterate(tps_queue, find_item, p_fault, (void**)&matched_node);

	if(matched_node != NULL)
	{
		fprintf(stderr, "TPS protection error!\n");
	}
	signal(SIGSEGV, SIG_DFL);
	signal(SIGBUS, SIG_DFL);
	raise(sig);
}

int tps_init(int segv)
{
	if(initialized == true)
	{
		return -1;
	}

	tps_queue = queue_create();
	initialized = true;

	if(segv)
	{
		struct sigaction sa;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_SIGINFO;
		sa.sa_sigaction = segv_handler;
		sigaction(SIGBUS, &sa, NULL);
		sigaction(SIGSEGV, &sa, NULL);
	}
	return 0;
}

int tps_create(void)
{
	//add error cases
	//if already created return -1
	//
	tps_node_t new_node = (tps_node_t)malloc(sizeof(struct tps_node));
	if(new_node == NULL)
	{
		return -1;
	}
	new_node->thread = pthread_self();
	new_node->memory_page = mmap(NULL, TPS_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1 , 0);
	queue_enqueue(tps_queue, (void*)new_node);
	return 0;
}

int tps_destroy(void)
{
	//add error cases
	tps_node_t node_to_destroy;
	pthread_t current_thread = pthread_self();
	if (queue_length(tps_queue) == 0){
		return -1;
	}
	if (queue_iterate(tps_queue, find_item, (void*)current_thread, (void**)&node_to_destroy) == -1)
	{
		return -1;
	}
	queue_iterate(tps_queue, find_item, (void*)current_thread, (void**)&node_to_destroy);
	queue_delete(tps_queue, (void*)node_to_destroy);
	//are we missing something here?
	free(node_to_destroy);
	return 0; 
}

int tps_read(size_t offset, size_t length, char *buffer)
{
	// tps_node_t tps_read;
	// enter_critical_section();
	// //iterate through the queue, find the thread
	// queue_iterate(tps_queue, find_item, (void*)pthread_self(), (void**)&tps_read);
	return 0;

}

int tps_write(size_t offset, size_t length, char *buffer)
{
	return 0;
	/* TODO: Phase 2 */
}

int tps_clone(pthread_t tid)
{
	// void *data new_mmap;
	// new_mmap = mmap
	return 0;
	/* TODO: Phase 2 */
}


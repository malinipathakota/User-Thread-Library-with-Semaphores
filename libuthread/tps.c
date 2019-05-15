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

typedef enum {false, true} bool;

struct tps_page {
	void *mmap_address;
	int count;
};
typedef struct tps_page* tps_page_t;

struct tps_node {
	void *memory_page;
	pthread_t thread;
	size_t length;
	tps_page_t page;
};
typedef struct tps_node* tps_node_t;
bool initialized = false;
queue_t tps_queue;

static int find_item(void *data, void *arg) {
    tps_node_t node = (tps_node_t)data;
	pthread_t thread = (pthread_t)arg;
    if(node->thread == thread) {
		 return 1;
	} 
    return 0;
}

static int find_queue(void *data, void *arg) {
	tps_node_t node = (tps_node_t)data;
	pthread_t thread = *(pthread_t *)arg;
	if(node->thread == thread) {
		 return 1;
	} 
	return 0;
}

static void segv_handler(int sig, siginfo_t *si, void *context) {
	void *p_fault = (void*)((uintptr_t)si->si_addr & ~(TPS_SIZE - 1));
	tps_node_t matched_node;
	queue_iterate(tps_queue, find_item, p_fault, (void**)&matched_node);

	if(matched_node != NULL) {
		fprintf(stderr, "TPS protection error!\n");
	}
	signal(SIGSEGV, SIG_DFL);
	signal(SIGBUS, SIG_DFL);
	raise(sig);
}

int tps_init(int segv) {
	if(initialized == true) {
		return -1;
	}

	tps_queue = queue_create();
	initialized = true;

	if(segv) {
		struct sigaction sa;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_SIGINFO;
		sa.sa_sigaction = segv_handler;
		sigaction(SIGBUS, &sa, NULL);
		sigaction(SIGSEGV, &sa, NULL);
	}
	return 0;
}

int tps_create(void) {
	tps_node_t node = NULL;
	queue_iterate(tps_queue, find_queue, (void *)pthread_self(), (void **)&node);
	if(node != NULL) {
		return -1;
	}

	node = malloc(sizeof(struct tps_node));
	tps_page_t page = malloc(sizeof(struct tps_page));

	node->thread = pthread_self();
	node->page = page;
	node->page->mmap_address = mmap(NULL, TPS_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	node->page->count = 1;
	queue_enqueue(tps_queue, node);
}

int tps_destroy(void) {
	tps_node_t node = NULL;
	queue_iterate(tps_queue, find_queue, (void *)pthread_self(), (void **)&node);
	if(node == NULL) {
		return -1;
	}
	if(node->page->count > 1) {
		node->page->count = node->page->count - 1;
	}
	else {
		if(node->page->count <= 1 || !munmap(node->page->mmap_address, TPS_SIZE)) {
			free(node->page);
		}
	}
	free(node);
	return 0;
}

int check_fail(tps_node_t node, size_t offset, size_t length, char *buffer) {
	if(node == NULL || buffer == NULL || offset < 0 || length < 0) {
		return -1;
	}
	if(length + offset > TPS_SIZE) {
		return -1;
	}
	if(mprotect(node->page->mmap_address, TPS_SIZE, PROT_READ) == -1) {
		return -1;
	}
	return 0;
}

int tps_read(size_t offset, size_t length, char *buffer) {
	tps_node_t node = NULL;
	queue_iterate(tps_queue, find_queue, (void *)pthread_self(), (void **)&node);

	if(check_fail(node, offset, length, buffer) != -1) {
		memcpy(buffer, node->page->mmap_address + offset, length);
		mprotect(node->page->mmap_address, TPS_SIZE, PROT_NONE);
		return 0;
	}
	return -1;
}

int tps_write(size_t offset, size_t length, char *buffer) {
	tps_node_t node = NULL;
	queue_iterate(tps_queue, find_queue, (void *)pthread_self(), (void **)&node);

	if(check_fail(node, offset, length, buffer) == -1) {
		return -1;
	}
	else {
		if(node->page->count >= 0) {
			tps_page_t thispage = malloc(sizeof(struct tps_page));
			thispage->mmap_address = mmap(NULL, TPS_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			thispage->count = 1;
			mprotect(thispage->mmap_address, TPS_SIZE, PROT_WRITE);
			mprotect(node->page->mmap_address, TPS_SIZE, PROT_READ);
			memcpy(thispage->mmap_address, node->page->mmap_address, TPS_SIZE);
			mprotect(thispage->mmap_address, TPS_SIZE, PROT_NONE);
			mprotect(node->page->mmap_address, TPS_SIZE, PROT_NONE);
			node->page->count = node->page->count - 1;
			node->page = thispage;
		}
	}
	mprotect(node->page->mmap_address, TPS_SIZE, PROT_WRITE);
	memcpy(node->page->mmap_address + offset, buffer, length);
	mprotect(node->page->mmap_address, TPS_SIZE, PROT_NONE);
	return 0;
}

int tps_clone(pthread_t tid) {
	tps_node_t original = NULL;
	queue_iterate(tps_queue, find_queue, (void *)pthread_self(), (void **)&original);
	tps_node_t node = NULL;
	queue_iterate(tps_queue, find_queue, (void *)tid, (void **)&node);

	if (original != NULL || node == NULL) {
		return -1;
	}
	else {
		tps_node_t newnode = NULL;
		queue_iterate(tps_queue, find_queue, (void *)pthread_self(), (void **)&newnode);
		if (newnode != NULL) {
			return -1;
		}
		newnode = malloc(sizeof(struct tps_node));
		newnode->page = node->page;
		newnode->thread = pthread_self();
		queue_enqueue(tps_queue, newnode);

		original = NULL;
		queue_iterate(tps_queue, find_queue, (void *)pthread_self(), (void **)&original);
		original->page->count = original->page->count + 1;
		return 0;
	}
}

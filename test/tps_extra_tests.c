#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tps.h>
#include <sem.h>

static char msg1[TPS_SIZE] = "Hello world!\n";

void test_init()
{
	assert(tps_init(0) == 0);
	assert(tps_init(0) == -1);
    	printf("test_init passed\n");
}

void test_create_destroy()
{
	assert(tps_destroy() == -1);
	assert(tps_create() == 0);
	assert(tps_destroy() == 0);
    	printf("test_create_destroy passed\n");
}

void no_create()
{
	assert(tps_read(0,TPS_SIZE, msg1) == -1);
	assert(tps_write(0,TPS_SIZE, msg1) == -1);
    	printf("no_create passed \n");
}

void no_buffer()
{
	char buffer;
	assert(tps_read(0, 1 , &buffer) == -1);
	assert(tps_write(0, 1, &buffer) == -1);
    	printf("no buffer passed \n");
}

void bounds()
{
	tps_create();
	static char buffer[TPS_SIZE];
	assert(tps_read(0, TPS_SIZE + 1, buffer) == -1);
	assert(tps_write(0, TPS_SIZE + 1, buffer) == -1);
	tps_destroy();
    	printf("bounds passed \n");
}

void null_tps_read_write()
{
	tps_create();
	assert(tps_read(0, 16, NULL)== -1);
	assert(tps_write(0, 16, NULL) == -1);
	tps_destroy();
    	printf("null_tps_read_write passed \n");
}

int main(int argc, char **argv)
{
	test_init();
	no_create();
	test_create_destroy();
	null_tps_read_write();
	bounds();
	no_buffer();
    	printf("All tests passed!\n");
	return 0;
}




objs := queue.o sem.o thread.o tps.o
lib := libuthread.a
CC := gcc
CFLAGS := -Wall -Werror -g
deps := $(patsubst %.o,%.d,$(objs)) 
-include $(deps)
DEPFLAGS = -MMD -MF $(@:.o=.d)

libuthread.a: $(objs)
	ar rcs libuthread.a $(objs)

%.o: %.c         
	$(Q)$(CC) $(CFLAGS) -c -o $@ $< $(DEPFLAGS)

clean:
	$(Q)rm -rf tps.o sem.o $(deps) $(targets)

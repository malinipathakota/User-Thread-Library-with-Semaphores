## Report
Malini Pathakota <br />
Karishma Tangella <br />

## Introduction
In this project we are implementing a sempaphore API, which allows for
efficient thread synchronization using sempahores; a popular synchronization
primative. We are also implementing a thread private storage API (TPS). This
TPS API allows each thread to have its own private storage by having protected
memory regions per each thread. 

## Design
The two APIS we implemented are independent from each other. We designed
the Semaphore API using queues and making use of critical sections that is
provided by an external thread library. For our TPS API, we made use of
memory mapping, along with the external thread library just mentioned.

## Implementation

***Semaphore API***
As mentioned above, we used queues for this phase. We have a struct that
holds the size, and a queue. The size acts as a count and keeps track of 
waiting threads. We have multiple functions for this phase. We used an 
external thread library in order to enter and exit critical sections.

Our functions consist of:

1. Create
The create function initializes a queue, along with the count which is the 
size of the queue.

2. Destroy
The destroy function deallocates the semaphore. This is done by first making
sure that the queue is empty before we destroy it.

3. Sem up
Our sem up function releases a semaphore and all of its resources. This
is achieved by unblocking the thread, along with increasing the size.  
This is all done after entering the critical section. This ensures
mutual exclusion with other threads.

4. Sem down
Our sem down function takes resources from the semaphore. When we take an
unavailabe semaphore, the calling thread will be blocked until the semaphore
is available again. This was achived by enqueing the thread into the queue,
and blocking it. The size is also decreased. Just like in our sem up function,
this all takes place in the critical section to ensure
mutual exclusion with other threads.
ts size

5. Get value
Our get value function simply, gets the value of the size of our queue. If 
the size is equal to zero, we take the absolute value of it. 

***TPS API***
For this phase, we made use of memory maps. We have a struct that holds
a thread, the length, and a struct type memory page,which holds a memory page
address and a count. The memory page will protect data from being overwritten 
by other threads. This API provides a single private and protected memory page
of 4096 bytes for each thread that requires it. It also makes
use of critical sections, similar to our semamphore API.

We have multiple main functions for this phase that consist of:
1. tps init
Our init function creates our queue, and sets up our handler that will catch 
segmentation faults. This handler will get the tps of the specific address 
that causes the segmentation fault. It will also display an error message 
when there is a protection error. This function can only been called once, 
if not there will be an error. 

2. tps create
The create function creates a TPS area and associates it with the current 
thread. It does this by allocating space for both instances of the two structs 
mentioned previously, and populates them. The memory page is populated using 
mmap(), making sure that there is no read or write permission by default. 
Finally, we enqueue our instance of tps_node, which holds the thread, length 
and the memory map into the queue we initialized earlier. 

3. tps destroy 
This function destroys the tps area in association with the current thread. It 
finds the thread from the queue, and frees its memory.

4. tps read
This function reads a specified number of bytes of data from the current 
thread's TPS. We have a buffer and offsset so the function wont exceed the 
bounds. A helper function check_fail() was used to check for possible errors 
regarding the buffer, offset, length and the tps area. If there isnt any 
errors, we proceed. In order to read, we used memcpy() to copy the content 
starting from the memory page address added to offset, to the length given. We 
copied this content into our parameter buffer. We also made use of mprotect(), 
which protects the memory and makes sure it cannot be accessed at all, by 
using PROT_NONE as our parameter.

5. tps write
This function writes what is in our buffer into the current TPS at a specific
byte @offset which is given to us. Like we did in tps read, we call our
check_fail function to check for any errors, if there isnt any
we will proceed. We call mprotect, with the parameter PROT_WRITE which
allows the memory that has been accessed to be modified. We then use memcpy
to write in what we had from buffer into the address, and finally set the
protection to none. In the case that the current threads TPS shares a memory 
page with another threads TPS, this trigger a copy-on-write operation that 
occurs before we proceed with the write operation. We create a new private 
memory page that the thread can write to. This prevents it from overwriting 
another threads memory. We did this by allocating space for a new memory page,
make a copy of this page and set the current threads page to this copy. 

6. tps clone
In our clone function, we clone the threads TPS. We did this in two phases. 
In the first phase, we copied directly from the TPS content. In the second 
phase however we refer to the same memory page, and increase the reference 
counter to show that it is being shared. 

Our helper functions consist of:
1. find item
Our find item function will be used when we call queue_iterate(). 
It checks for matching threads. This is only used in our segv handler.

2. find queue
This is another find function that will be used with queue iterate function
and is used throughout the program to find matching threads.

3. segv_handler
Our segv handler like mentioned previously, will catch segmentation faults.
This handler will get the tps of the specific address that causes the 
segmentation fault. It will also display an error message when there is a 
protection error.

4. check fail
As mentioned previously, our check fail function checks for errors regarding 
the offset, length or if our buffer is null. This function is used when we 
call tps read, or tps write. 


## Testing
To test for these phases, we used the testers that were provided as as well as 
our own testers. These consist of sem_buffer.c, sem_count.c, sem_prime.c and 
tps.c. These testers tested for basic functionality of our program. 
The testers we implemented test for all the edge cases of errors.
We have a multitude of tests to check for different possiblities including
checking for out of bounds with read and write, reading and writing without 
creating a tps, destroying without creating, and reading without a buffer.
Finally, we caused an intentional TPS protection error to see if there would
be a segmentation fault.


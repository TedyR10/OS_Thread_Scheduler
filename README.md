**Theodor-Ioan Rolea**
**323CA**

## Thread Scheduler HW2

### Overview:

* This project aims to create a thread scheduler in C using a Round Robin approach. I have used a thread structure that stores essential information about the thread, a scheduler structure to manage the threads and a priority queue to always have the threads ordered by their priority.
* My implementation is pretty fast, but what could be improved is how the queue is stored. It could have started at size 1 and realloc its memory once it is full. Other than that, the implementation is straight-forward and works pretty well with the given tests.
* The priority queue structure has another queue made specifically for active threads. It was easier to create a sperate queue for the threads that have been started, rather than trying to have the priority queue do all the work.
* Threads have a semaphore to signal if the thread can run or not. I have opted for a semaphore instead of a mutex (or barrier) because the implementation was easier for me and it suited the scheduler more, given that it works on a signaling mechanism and not a locking mechanism.

### Main functions breakdown:

* **In depth comments throughout the code**
* so_init: initializes the scheduler & queue and checks for possible errors;
* so_fork: allocs and initializes a new thread, adds it to the queue and updates the scheduler accordingly;
* so_exec: simulates the execution of an instruction, thus decreasing its time slice and updating the scheduler afterwards;
* so_wait: the thread waits for an event and the scheduler is updated;
* so_signal: signals all the threads waiting for an event;
* so_end: waits for all the threads to finish their execution and destroys everything.

### How to run:

* use "make" to compile the code
* use "make -f Makefile.checker" to check tests

### Resources:

* https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-08
* https://ocw.cs.pub.ro/courses/so/cursuri/curs-08
* https://ocw.cs.pub.ro/courses/so/cursuri/curs-09

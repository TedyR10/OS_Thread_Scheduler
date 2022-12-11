/* SPDX-License-Identifier: <SPDX License Expression> */
// Copyright Theodor-Ioan Rolea 2022
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "so_scheduler.h"
#include "utils.h"
#define MAX_QUEUE_SIZE 1000
#define MAX_THREAD_SIZE 1000
#define BLOCKED 0
#define READY 1
#define RUNNING 2
#define TERMINATED 3
#define NEW 4

/**
 *  Thread struct
 */
typedef struct {
	/*
	 * Thread parameters
	 */
	uint time_left;
	uint priority;
	uint io;
	tid_t tid;
	uint state;
	so_handler *handler;
	/*
	 * Semaphore for the different states
	 */
	sem_t sem;
} thread;

/**
 *	Queue struct
 */
typedef struct Queue {
	thread **priority_queue;
	thread **active_threads;
	uint q_size;
	uint t_size;
} Queue;

/**
 *  Scheduler struct
 */
typedef struct scheduler {
	int is_initialized;
	uint events_nr;
	uint time_quantum;
	thread *running;
} scheduler;
/**
 *	Create a new queue
 */
Queue *createQueue(void);

/**
 *	Add a node to the queue
 */
void enQueue(Queue *q, thread *t);

/**
 *	Delete a node from the queue
 */
void deQueue(Queue *q);

/**
 *	Destroy a queue
 */
void destroyQueue(Queue *q);

/**
 *  Get the first node in queue
 */
thread *peek(Queue *q);

/**
 *  Check if the queue is empty
 */
int is_empty(Queue *q);

/**
 *  Starts the next thread
 */
void start_next_thread(scheduler *s, Queue *q, thread *t);

/**
 *  Check for the next thread & update the scheduler
 */
void update_scheduler(scheduler *s, Queue *q);

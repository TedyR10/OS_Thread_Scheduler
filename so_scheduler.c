// SPDX-License-Identifier: <SPDX License Expression>
// Copyright Theodor-Ioan Rolea 2022

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include "utils.h"
#include "so_scheduler.h"
#include "queue.h"
#define BLOCKED 0
#define READY 1
#define RUNNING 2
#define TERMINATED 3
#define NEW 4

static scheduler s;
static Queue *q;

static void *start_routine(void *args);
thread *init_thread(so_handler *func, uint priority);

int so_init(uint time_quantum, uint io)
{
	/*
	 * Check for possible errors
	 */
	if (s.is_initialized)
		return -1;

	else if (time_quantum <= 0)
		return -2;

	else if (io > SO_MAX_NUM_EVENTS)
		return -2;

	/*
	 * Initialize scheduler
	 */
	s.is_initialized = 1;
	s.events_nr = io;
	s.time_quantum = time_quantum;
	s.running = NULL;

	/*
	 * Create the queue
	 */
	q = createQueue();

	return 0;
}

tid_t so_fork(so_handler *func, uint priority)
{
	thread *t;

	/*
	 * Check for possible errors
	 */
	if (func == NULL)
		return INVALID_TID;

	else if (priority > SO_MAX_PRIO)
		return INVALID_TID;

	else {
		t = init_thread(func, priority);

		/*
		 * Queue the new thread
		 */
		q->active_threads[q->t_size] = t;
		q->t_size++;
		enQueue(q, t);

		/*
		 * Update scheduler
		 */
		if (s.running == NULL)
			update_scheduler(&s, q);
		else
			so_exec();

		return t->tid;
	}
}

void so_exec(void)
{
	/*
	 * Decrease the time left
	 */
	thread *t = s.running;

	t->time_left--;

	/*
	 * Update the scheduler
	 */
	update_scheduler(&s, q);

	/*
	 * Lock the semaphore in case of preemption
	 */
	int ret = sem_wait(&t->sem);

	DIE(ret != 0, "sem_wait");
}

int so_wait(uint io)
{
	/*
	 * Check for possible errors
	 */
	if (io >= s.events_nr)
		return -1;

	else {
		/*
		 * Change the state of the current thread
		 */
		s.running->state = BLOCKED;
		/*
		 * Wait for io
		 */
		s.running->io = io;

		so_exec();
		return 0;
	}
}

int so_signal(uint io)
{
	/*
	 * Check for possbile errors
	 */
	if (io >= s.events_nr)
		return -1;

	else {
		/*
		 * Count the threads that have been waiting for io
		 */
		int threads_woke = 0;

		for (uint i = 0; i < q->t_size; i++)
			if (q->active_threads[i]->io == io) {
				if (q->active_threads[i]->state == BLOCKED) {
					q->active_threads[i]->io = SO_MAX_NUM_EVENTS;
					q->active_threads[i]->state = READY;
					enQueue(q, q->active_threads[i]);
					threads_woke++;
				}
			}

		so_exec();
		return threads_woke;
	}
}

void so_end(void)
{
	/*
	 * Check for possible errors
	 */
	if (!s.is_initialized)
		return;

	else {
		/*
		 * Wait for threads to finish
		 */
		for (uint i = 0; i < q->t_size; i++) {
			int ret = pthread_join(q->active_threads[i]->tid, NULL);

			DIE(ret != 0, "thread join");
		}

		/*
		 * Destroy threads
		 */
		for (uint i = 0; i < q->t_size; i++) {
			thread *t = q->active_threads[i];

			int ret = sem_destroy(&t->sem);

			DIE(ret != 0, "sem destroy");

			free(t);
		}

		/*
		 * Destroy scheduler
		 */
		s.is_initialized = 0;

		/*
		 * Destroy queue
		 */
		destroyQueue(q);
	}
}

/**
 *  Helper function for creating a thread
 */
static void *start_routine(void *args)
{
	thread *t = (thread *)args;

	/*
	 * Lock the semaphore
	 */
	int ret = sem_wait(&t->sem);

	DIE(ret != 0, "sem wait");

	/*
	 * Run handler
	 */
	t->handler(t->priority);

	/*
	 * Update scheduler
	 */
	t->state = NEW;
	update_scheduler(&s, q);

	return NULL;
}

/**
 *  Helper function for initializing a thread
 */
thread *init_thread(so_handler *func, uint priority)
{
	thread *t;

	t = malloc(sizeof(thread));

	if (t != NULL) {
		/*
		 * Initialize the new thread
		 */
		t->time_left = s.time_quantum;
		t->priority = priority;
		t->io = SO_MAX_NUM_EVENTS;
		t->tid = INVALID_TID;
		t->state = NEW;
		t->handler = func;

		/*
		 * Initialize thread semaphore
		 */
		sem_init(&t->sem, 0, 0);

		/*
		 * Create new thread
		 */
		pthread_create(&t->tid, NULL, &start_routine, (void *)t);

		return t;
	}

	DIE(t == NULL, "malloc error");
	return t;
}

// SPDX-License-Identifier: <SPDX License Expression>
// Copyright Theodor-Ioan Rolea 2022

#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "so_scheduler.h"

Queue *createQueue(void)
{
	/*
	 * Initialize queue
	 */
	Queue *q = (Queue *)malloc(sizeof(Queue));

	DIE(q == NULL, "malloc error");

	q->priority_queue = malloc(MAX_QUEUE_SIZE * sizeof(thread));

	DIE(q->priority_queue == NULL, "malloc error");

	q->active_threads = malloc(MAX_THREAD_SIZE * sizeof(thread));

	DIE(q->active_threads == NULL, "malloc error");

	q->q_size = 0;
	q->t_size = 0;
	return q;
}

void enQueue(Queue *q, thread *t)
{
	/*
	 * Search for the correct position
	 */
	uint pos = 0;

	for (uint i = 0; i < q->q_size; i++) {
		if (pos < q->q_size && q->priority_queue[i]->priority < t->priority)
			pos++;
		else
			break;
	}

	/*
	 * Shift threads accordingly
	 */
	for (uint j = q->q_size; j > pos; j--)
		q->priority_queue[j] = q->priority_queue[j - 1];

	q->priority_queue[pos] = t;
	q->priority_queue[pos]->state = READY;
	++q->q_size;
}

int is_empty(Queue *q)
{
	if (q->q_size == 0)
		return 1;
	return 0;
}

thread *peek(Queue *q)
{
	return q->priority_queue[q->q_size - 1];
}

void destroyQueue(Queue *q)
{
	free(q->priority_queue);
	free(q->active_threads);
	free(q);
}

void start_next_thread(scheduler *s, Queue *q, thread *t)
{
	q->priority_queue[q->q_size - 1] = NULL;
	q->q_size--;
	t->state = RUNNING;
	t->time_left = s->time_quantum;
	int ret = sem_post(&t->sem);

	DIE(ret != 0, "sem_post");
}

void update_scheduler(scheduler *s, Queue *q)
{
	thread *current = s->running;

	if (is_empty(q) == 1) {
		/*
		 * Threads finished, release semaphore
		 */
		sem_post(&current->sem);
		return;
	}

	/*
	 * Get first thread in queue
	 */
	thread *next = peek(q);

	/*
	 * If there's no thread is running, start the first one in queue
	 */
	if (s->running == NULL) {
		s->running = next;
		start_next_thread(s, q, next);
		return;
	}

	/*
	 * if the current thread is blocked, start the first one in queue
	 */
	else if (current->state == BLOCKED) {
		s->running = next;
		start_next_thread(s, q, next);
		return;
	}

	/*
	 * if the current thread is terminated, start the first one in queue
	 */
	else if (current->state == TERMINATED) {
		s->running = next;
		start_next_thread(s, q, next);
		return;
	}

	/*
	 * if the current thread is new, start the first one in queue
	 */
	else if (current->state == NEW) {
		s->running = next;
		start_next_thread(s, q, next);
		return;
	}

	/*
	 * Check if there is a higher priority thread in queue
	 */
	else if (current->priority < next->priority && (next->state != BLOCKED
		|| next->state != TERMINATED)) {
		enQueue(q, current);
		s->running = next;
		start_next_thread(s, q, next);
		return;
	}

	/*
	 * Check if the thread's time has run out
	 */
	else if (next->priority == current->priority && current->time_left < 1) {
		enQueue(q, current);
		s->running = next;
		start_next_thread(s, q, next);
		return;
	}

	else
		sem_post(&current->sem);
}

#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>

#include "queue.h"

struct STRUCT_QUEUE_NODE {
	void *data;
	queue_node *next;
};

struct STRUCT_QUEUE {
	// int capacity;
	sem_t sem_count;
	sem_t sem_remaining;
	pthread_mutex_t mtx_queue;
	queue_node *head;
	queue_node *tail;
};


void queue_enqueue(queue *q, void* elem) {
	sem_wait(&q->sem_remaining);
	queue_node *node = malloc(sizeof(queue_node));
	node->next = NULL;
	node->data = elem;
	pthread_mutex_lock(&q->mtx_queue);
	if (!q->tail) {
		q->tail = (q->head =  node);
	} else {
		q->tail->next = node;
		q->tail = node;	
	}
	pthread_mutex_unlock(&q->mtx_queue);
	sem_post(&q->sem_count);
}

void* queue_dequeue(queue *q) {
	sem_wait(&q->sem_count);
	pthread_mutex_lock(&q->mtx_queue);
	queue_node *head = q->head;
	assert(head);
	if (head == q->tail) {
		q->head = (q->tail = NULL);
	} else {
		q->head = head->next;	
	}
	pthread_mutex_unlock(&q->mtx_queue);
	sem_post(&q->sem_remaining);
	void *ret = head->data;
	free(head);
	
	return ret;
}


queue *queue_create(int max_elems) {
	queue *q = malloc(sizeof(queue));
	sem_init(&q->sem_count, 0, 0);
	if (max_elems <= 0) {
		max_elems = (int)(((unsigned int)-1) >> 1); // INT_MAX
		sem_init(&q->sem_remaining, 0, max_elems);
	} else {
		sem_init(&q->sem_remaining, 0, max_elems);
	}

	pthread_mutex_init(&q->mtx_queue, NULL);
	q->head = NULL;
	q->tail = NULL;
	return q;
}

void queue_destroy(queue *q) {
	pthread_mutex_lock(&q->mtx_queue);
	while (q->head) {
		queue_node *current = q->head;
		free(current);
		q->head = current;
	}
	pthread_mutex_unlock(&q->mtx_queue);
	pthread_mutex_destroy(&q->mtx_queue);
	sem_destroy(&q->sem_count);
	sem_destroy(&q->sem_remaining);
	free(q);
}



#pragma once

/**
 * Simple implementation of a thread-safe FIFO queue.
 * Only enqueue and dequeue operations are provided since the size and head of
 * the queue may change at anytime after a thread gets information about size
 * and/or head of the queue.
 */

struct STRUCT_QUEUE;
struct STRUCT_QUEUE_NODE;
typedef struct STRUCT_QUEUE queue;
typedef struct STRUCT_QUEUE_NODE queue_node;


/**
 * Allocate a new queue.
 *
 * Caller is responsible for cleanup the queue by calling queue_destroy().
 * 
 * For positive max_elems, the queue will block the thread calling 
 * queue_enqueue() when the queue contains max_elems elements
 * 
 * Non-positive max_elems will create a queue that never blocks on 
 * queue_enqueue(). However, since pthread semaphores are used for this 
 * implementation, the queue WILL block on queue_enqueue() when the queue
 * contains INT_MAX elements
 */
queue *queue_create(int max_elems);


/**
 * Deallocate all internal data structures used by the queue, and then the 
 * queue itself.
 */
void queue_destroy(queue *q);


/**
 * Inserts an element to the back of the queue.
 * Blocks if the queue is full(contains max_elems elements)
 *
 * Thread safety: MT-safe
 */
void queue_enqueue(queue *q, void *element);


/**
 * Remove the element at the head of the queue.
 * Blocks if the queue is empty.
 *
 * Thread safety: MT-safe
 *
 */
void* queue_dequeue(queue *q);


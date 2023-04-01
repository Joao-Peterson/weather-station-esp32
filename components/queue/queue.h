#ifndef _QUEUE_HEADER_
#define _QUEUE_HEADER_

#include <stdlib.h>

// ------------------------------------------------------------ Types --------------------------------------------------------------

/**
 * @brief struct for a queue for type double
 */
typedef struct{
    double *array;
    size_t size;
    size_t elements;
}queue_double_t;

// ------------------------------------------------------------ Functions ----------------------------------------------------------

// create a new queue
queue_double_t *queue_new(size_t size);

// destroy a queue
void queue_delete(queue_double_t *queue);

// add an element to the queue
void queue_add(queue_double_t *queue, double value);

// get the average value from the elements
double queue_average(queue_double_t *queue);

#endif
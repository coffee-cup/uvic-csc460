#include "process.h"

/**
 * Initializes a task queue for tracking a certain priority task.
 * Returns a pointer to the initialized list if successfull.
 */
task_queue_t* queue_init(task_queue_t* list, PRIORITY_LEVEL type) {
    // Have a non-null pointer, and a valid priority type
    // All conditions inside inner-most parens must be true to continue
    if (!(list && type < NUM_PRIORITY_LEVELS)) {
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
    list->type = type;

    return list;
}

/**
 * Enqueues a task at the end of the queue iff the task type matches the queue type.
 */
void enqueue(task_queue_t* list, PD* task) {
    // Have non-null list and task, and the queue type matches the task priority.
    // All conditions inside inner-most parens must be true to continue
    if (!(list && task && list->type == task->priority)) {
        return;
    }

    if (list->length == 0) {
        list->head = task;
        list->tail = task;
        list->length = 1;
    } else {
        list->tail->next = task;
        list->tail = task;
        list->length += 1;
    }
}

/**
 * Dequeues the first task from the queue
 * Returns the dequeued task or NULL if the queue is empty
 */
PD* deque(task_queue_t* list) {
    // Have a non-null list, and its length is greater than 0
    // All conditions inside inner-most parens must be true to continue
    if (!(list && list->length > 0)) {
        return NULL;
    }

    PD* element = list->head;
    list->length -= 1;
    if (list->length == 0) {
        // Nothing left in the queue, reset it
        list = queue_init(list, list->type);
    } else {
        // Still have some items left, clean up
        list->head = element->next;
        element->next = NULL;
    }

    return element;
}

/**
 * Peeks at the first task in the queue
 * Returns the task without dequeueing it
 */
PD* peek(task_queue_t* list) {
    // Have a non-null list
    // All conditions inside inner-most parens must be true to continue
    if (!(list)) {
        return NULL;
    }

    return list->head;
}

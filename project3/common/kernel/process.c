#include "process.h"
#include "os.h"

/**
 * Initializes a task queue for tracking a certain priority task.
 * Returns a pointer to the initialized list if successful.
 */
task_queue_t* queue_init(task_queue_t* list, PRIORITY_LEVEL type) {
    // Have a non-null pointer, and a valid priority type
    // All conditions inside inner-most parens must be true to continue
    if (!(list && type < NUM_PRIORITY_LEVELS)) {
        OS_Abort(QUEUEING_ERROR);
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
    list->type = type;

    return list;
}


/**
 * Peeks at the first task in the queue
 * Returns the task without dequeueing it
 */
PD* peek(task_queue_t* list) {
    // Have a non-null list
    // All conditions inside inner-most parens must be true to continue
    if (!(list)) {
        OS_Abort(QUEUEING_ERROR);
        return NULL;
    }

    return list->head;
}


/**
 * Dequeues the first task from the queue
 * Returns the dequeued task or NULL if the queue is empty
 */
PD* deque(task_queue_t* list) {
    // Have a non-null list, and its length is greater than 0
    // All conditions inside inner-most parens must be true to continue
    if (!(list && list->length > 0)) {
        OS_Abort(QUEUEING_ERROR);
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
 * Enqueues a task at the end of the queue iff the task type matches the queue type.
 */
void enqueue(task_queue_t* list, PD* task) {
    // Have non-null list and task, and the queue type matches the task priority.
    // All conditions inside inner-most parens must be true to continue
    if (!(list && task && list->type == task->priority)) {
        OS_Abort(QUEUEING_ERROR);
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
 * Returns whether task t1 is scheduled later than task t2.
 * Takes into account the clock overflow state.
 */
BOOL later(PD* t1, PD* t2) {
    return ((t1->clockState != t2->clockState && t1->ttns < t2->ttns) ||
            (t1->clockState == t2->clockState && t1->ttns > t2->ttns));
}

/**
 * Inserts a task into the queue respecting time to next start,
 * This operation is only supported for PERIODIC queues
 */
void insert(task_queue_t* list, PD* task) {
    // Have a non-null list, and the queue type matches the task priority.
    // All conditions inside inner-most parens must be true to continue
    if (!(list && task && list->type == task->priority && list->type == PERIODIC)) {
        OS_Abort(QUEUEING_ERROR);
        return;
    }

    // No searching required, queue is empty
    if (list->length == 0) {
        enqueue(list, task);
    }

    // Queue has items, find the right place to insert
    else {
        PD* element = list->head;
        PD* element_prev = NULL;

        // Starting from the front, find the first element that
        // this task should run before
        while (element != NULL && later(task, element)) {
            element_prev = element;
            element = element->next;
        }

        // Task should run next, inserting at front
        if (element_prev == NULL) {
            list->head = task;
            task->next = element;
        }

        // Task should run last, inserting at end
        else if (element == NULL) {
            list->tail = task;
            element_prev->next = task;
            task->next = NULL;
        }

        // Task runs before some element and after other,
        // inserting between existing elements
        else {
            element->next = task;
            task->next = element;
        }

        list->length += 1;
    }
}

# Scheduling
The core algorithm of a real-time operating system is it's scheduler. Our scheduler has proven capable, despite its rather simple implementation. The scheduler tracks each task priority type in separate 'in-place' queues. By in-place, we mean that there is no extra memory allocated for the queue elements, and instead the queue operates on the `ProcessDescriptor`'s `next` pointer.

The dispatch function, located in the kernel, is responsible for selecting the next task to run. In our code, this means re-assigning `Cp` and `CurrentSp`. The dispatch function can be thought about in two halves. The top half, is just the `switch` statement shown in the source below. This switch moves the currently running task to the end of it's queue, or in the case of periodic tasks, to the correct offset. Since the kernel assumes and upholds the invariant that the running task is at the front of it's respective queue, this move to end operation is trivial.

```c
/**
 * This internal kernel function is a part of the "scheduler". It chooses the
 * next task to run, i.e., Cp.
 */
static void Dispatch() {
    /* Move the current task to the end of it's queue */
    /* We use the invariant that the running task is at the front of it's queue */
    switch (Cp->priority) {
        case SYSTEM:
            if (system_tasks.length > 1) {
                enqueue(&system_tasks, deque(&system_tasks));
            }
            break;

        case PERIODIC:
            if (periodic_tasks.length > 1) {
                insert(&periodic_tasks, deque(&periodic_tasks));
            }
            break;

        case RR:
            if (rr_tasks.length > 1) {
                enqueue(&rr_tasks, deque(&rr_tasks));
            }
            break;

        default:
            /* Could have Cp == IdleProcess, in which case the priority isn't a normal value
               Only abort if the CP isn't the idle process */
            if (Cp != &IdleProcess) {
                OS_Abort(INVALID_PRIORITY);
                return;
            }
        break;
    }

    /* Bottom half of dispatch begins here */
    /* Only change the current task if it's not running */
    if (Cp->state != RUNNING ) {
        PD* new_p = NULL;

        /* Check the system tasks */
        if (system_tasks.length > 0) {
            new_p = Queue_Rotate_Ready(&system_tasks);
        }

        /* Haven't found a new task */
        if (new_p == NULL) {

            /* Check for periodic tasks which are ready to run, assuming sorted order
               based on increasing time to next start, only need to check first task */
            if (periodic_tasks.length > 0 && sys_clock >= peek(&periodic_tasks)->ttns) {
                new_p = Queue_Rotate_Ready(&periodic_tasks);

                /* A periodic task must be run on its period */
                if (new_p != NULL && sys_clock > new_p->ttns) {
                    OS_Abort(TIMING_VIOLATION);
                    return;
                }
            }

            /* No periodic tasks should be started, check round robin tasks now */
            else if (rr_tasks.length > 0) {
                /* Check all the round robin tasks */
                new_p = Queue_Rotate_Ready(&rr_tasks);
            }
        }

        /* Nothing is ready to run! Use our lower-than-low priority task */
        if (new_p == NULL) {
            new_p = &IdleProcess;
        }

        /* Finally set the new task */
        Cp = new_p;
    }

    CurrentSp = Cp->sp;
    Cp->state = RUNNING;
}
```
The bottom half of dispatch, shown above, is for selecting a new task to run. The dispatcher looks though the system task queue first for any system task with `READY` state. This is search is handled by the `Queue_Rotate_Ready` function, which moves the first item to the end of the queue until the queue has been completely rotated, or until the item at head of the queue is in the `READY` state.

If no system tasks were ready, we check for periodic tasks which are currently at their next scheduled start time. We rotate the queue here as well, since we allow *sub-tick* context switching in our kernel, that is it is possible for two periodic tasks to execute on the same tick, so long as neither process consumes the whole tick. If a periodic task is found, a timing violation check is performed before it is executed.

Next, if no periodic tasks are ready to be executed, we finally check for round robin tasks. A round robin task is extracted in the same way that a system task is extracted. The source for `Queue_Rotate_Ready` is below.

Finally, if no other processes are ready, we elect to context switch to a special, low priority idle task. The idle task has a hidden priority level which is one level lower than round robin tasks. The idle task can be used to calculate the CPU utilization among other things.

The following source is for the `Queue_Rotate_Ready` algorithm mentioned above.

```c
/**
 * Finds and returns the first READY task in `queue`.
 * If no task is found, returns NULL and queue order is unchanged
 * If a task is found, `queue` will be rotated so that the READY task is
 * at the front.
 */
PD* Queue_Rotate_Ready(task_queue_t* queue) {
    // Get candidate task and first task in queue
    PD* iter_task;
    PD* first;
    first = iter_task = peek(queue);


    // do-while so that the first loop doesn't break since first == new_p
    do {
        // Specify that the task has to be ready
        if (iter_task->state != READY) {
            // Move non-ready tasks to the end of the queue
            // If this is a periodic queue, use insert
            if (queue->type == PERIODIC) {
                insert(queue, deque(queue));
            } else {
                enqueue(queue, deque(queue));
            }

            // Get the new head task
            iter_task = peek(queue);
        } else {
            // task state is ready!
            break;
        }
    } // Loop until the first task comes up again
    while (iter_task != first);

    if (iter_task->state != READY) {
        // Didn't find anything that was ready
        iter_task = NULL;
    }

    return iter_task;
}
```

## Task Queueing
As mentioned we maintain the property that the currently executing task is at the beginning of its respective queue. This helps clarify the state in the kernel, along with making task termination easier. It should be noted that the task queues verify that any task added respects the designated task queue type.

## Task Priority and Preemption

As discussed in [task design](#task-design), differing tasks can have differing priorities. Some of which are preemptive to others. In the [testing](#testing) section we verify that this scheduling logic respects the priorities and causes preemption when expected.

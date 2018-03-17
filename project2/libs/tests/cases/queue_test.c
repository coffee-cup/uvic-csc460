#include "../../../common.h"
#include "../../../process.h"
#include "test_utils.h"
#include <avr/io.h>
#include <util/delay.h>

task_queue_t rr_test_queue;
task_queue_t sy_test_queue;
task_queue_t pr_test_queue;
task_queue_t test_queue;

PD rr_test_task1;
PD rr_test_task2;
PD rr_test_task3;
PD sy_test_task;
PD pr_test_task;

/////////////////////////////////////////////////////
// Initialization succeeds
/////////////////////////////////////////////////////
void Task_Queue_Init_Test()
{
    Assert(queue_init(&test_queue, NUM_PRIORITY_LEVELS) == NULL); // Doesn't initialize with bad priority
    AssertAborted();
    Assert(queue_init(&sy_test_queue, SYSTEM) != NULL); // Initialized result is returned
    Assert(sy_test_queue.length == 0);                  // Queue is empty upon initialization
    Assert(sy_test_queue.head == NULL);                 // Queue head is null upon initialization
    Assert(sy_test_queue.tail == NULL);                 // Queue tail is null initialization
    Assert(sy_test_queue.type == SYSTEM);               // Queue type is set correctly

    Assert(queue_init(&pr_test_queue, PERIODIC) != NULL); // Initialized result is returned
    Assert(pr_test_queue.head == NULL);                   // Queue head is null upon initialization
    Assert(pr_test_queue.tail == NULL);                   // Queue tail is null initialization
    Assert(pr_test_queue.length == 0);                    // Queue is empty upon initialization
    Assert(pr_test_queue.type == PERIODIC);               // Queue type is set correctly

    Assert(queue_init(&rr_test_queue, RR) != NULL); // Initialized result is returned
    Assert(rr_test_queue.head == NULL);             // Queue head is null upon initialization
    Assert(rr_test_queue.tail == NULL);             // Queue tail is null initialization
    Assert(rr_test_queue.length == 0);              // Queue is empty upon initialization
    Assert(rr_test_queue.type == RR);               // Queue type is set correctly
}


/////////////////////////////////////////////////////
// Adding tasks succeeds
/////////////////////////////////////////////////////
void Task_Queue_add_task_test()
{
    enqueue(&sy_test_queue, &sy_test_task);
    Assert(sy_test_queue.head == &sy_test_task);      // Task added is the one we added
    Assert(sy_test_queue.head == sy_test_queue.tail); // One task is accessed by head and tail
    Assert(sy_test_queue.length == 1);                // Have added one task, length reflects that

    enqueue(&sy_test_queue, &pr_test_task);
    AssertAborted();                                  // Adding the wrong type of task causes abort
    Assert(sy_test_queue.head == sy_test_queue.tail); // Queue remains unchanged
    Assert(sy_test_queue.length == 1);                // Queue remains unchanged
}


/////////////////////////////////////////////////////
// Adding multiple tasks succeeds
/////////////////////////////////////////////////////
void Task_Queue_add_multi_test()
{
    enqueue(&rr_test_queue, &rr_test_task1);
    Assert(rr_test_queue.head == rr_test_queue.tail); // One element, head and tail are same
    Assert(rr_test_queue.length == 1);                // One element, length is one

    enqueue(&rr_test_queue, &rr_test_task2);
    Assert(rr_test_queue.head != rr_test_queue.tail);       // Two elements, head and tail differ
    Assert(rr_test_queue.head->next == rr_test_queue.tail); // Two elements, head's next is tail
    Assert(rr_test_queue.length == 2);                      // Two elements, length is two

    enqueue(&rr_test_queue, &rr_test_task3);
    Assert(rr_test_queue.head != rr_test_queue.tail);             // Three elements, head and tail differ
    Assert(rr_test_queue.head->next->next == rr_test_queue.tail); // Three elements, head.next's next is tail
    Assert(rr_test_queue.length == 3);                            // Three elements, length is three

    Assert(rr_test_queue.head == &rr_test_task1); // Tasks are in order
    Assert(rr_test_queue.head->next == &rr_test_task2);
    Assert(rr_test_queue.tail == &rr_test_task3);
}


/////////////////////////////////////////////////////
// Removing and peeking at tasks succeeds
/////////////////////////////////////////////////////
void Task_Queue_remove_peek_test()
{
    PD *first = peek(&rr_test_queue);
    Assert(first != NULL);                   // Got something
    Assert(first == &rr_test_task1);         // Expected item is returned
    Assert(rr_test_queue.length == 3)        // Length is unchanged
    Assert(rr_test_queue.head == first);     // Item is still in queue

    PD *deq1 = deque(&rr_test_queue);
    Assert(deq1 != NULL);                     // Got something
    Assert(deq1->next == NULL);               // Item doesn't refer to queue anymore
    Assert(first == deq1);                    // Expected item was returned
    Assert(first == &rr_test_task1);          // Expected item is returned
    Assert(rr_test_queue.length == 2);        // Length was updated
    Assert(rr_test_queue.head != deq1);       // Dequeued item is no longer in the queue
    Assert(rr_test_queue.head->next != deq1); // Dequeued item is no longer in the queue

    PD *deq2 = deque(&rr_test_queue);
    Assert(deq2 != NULL);                             // Got something from subsequent dequeue
    Assert(deq2->next == NULL);                       // Item doesn't refer to queue anymore
    Assert(deq1 != deq2);                             // It's different from what we saw previously
    Assert(deq2 == &rr_test_task2);                   // Expected item was returned
    Assert(rr_test_queue.length == 1);                // Length was updated
    Assert(rr_test_queue.head == rr_test_queue.tail); // One item left, head and tail should point to it

    PD *last = peek(&rr_test_queue);
    PD *deq3 = deque(&rr_test_queue);
    Assert(deq3 != NULL);       // Got something from subsequent dequeue
    Assert(deq3->next == NULL); // Item doesn't refer to queue anymore

    Assert(first != last);              // Peek is different from last peek
    Assert(last == deq3);               // Peek is same as subsequent dequeue
    Assert(deq1 != deq3);               // It's different from what we saw previously
    Assert(deq2 != deq3);               // It's different from what we saw previously
    Assert(deq3 == &rr_test_task3);     // Expected item was returned
    Assert(rr_test_queue.length == 0);  // Length was updated
    Assert(rr_test_queue.head == NULL); // Nothing left, head is null
    Assert(rr_test_queue.tail == NULL); // Nothing left, tail is null

    Assert(peek(&rr_test_queue) == NULL);  // Didn't get anything from peeking empty queue
    Assert(deque(&rr_test_queue) == NULL); // Didn't get anything from dequeueing empty queue
    AssertAborted();
}

void Task_Queue_Test()
{
    ZeroMemory(rr_test_task1, sizeof(PD));
    ZeroMemory(rr_test_task2, sizeof(PD));
    ZeroMemory(rr_test_task3, sizeof(PD));
    ZeroMemory(sy_test_task, sizeof(PD));
    ZeroMemory(pr_test_task, sizeof(PD));

    rr_test_task1.priority = RR;
    rr_test_task2.priority = RR;
    rr_test_task3.priority = RR;

    sy_test_task.priority = SYSTEM;
    pr_test_task.priority = PERIODIC;

    Task_Queue_Init_Test();
    Task_Queue_add_task_test();
    Task_Queue_add_multi_test();
    Task_Queue_remove_peek_test();

}

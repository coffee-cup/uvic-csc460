#include "message.h"
#include "os.h"

/**
 * Initializes a msg queue for tracking messages
 * Returns a pointer to the initialized list if successful.
 */

msg_queue_t*  msg_queue_init(msg_queue_t* list) {
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;

    return list;
}

MSG* msg_deque(msg_queue_t* list) {
    if (!(list && list->length > 0)) {
        OS_Abort(QUEUEING_ERROR);
        return NULL;
    }


    MSG *curr = list->head;
    list->length -= 1;

    if (list->length == 0) {
        list = msg_queue_init(list);
    } else {
        list->head = curr->next;
        curr->next = NULL;
    }

    return curr;
}

void msg_enqueue (msg_queue_t* list, MSG* msg) {
    if (!(list && msg)) {
        OS_Abort(QUEUEING_ERROR);
        return;
    }

    if (list->length == 0) {
        list->head = msg;
        list->tail = msg;
    } else {
        list->tail->next = msg;
        list->tail = msg;
    }

    list->length += 1;
}

/**
 * Finds and removes the first message for `receiver` matching the mask
 * Returns null if no message for receiver
 */
MSG* msg_find_receiver (msg_queue_t* list, PID receiver, MASK mask) {
    if (!(list && VALID_ID(receiver))) {
        OS_Abort(QUEUEING_ERROR);
        return NULL;
    }

    MSG* curr = list->head;
    MSG* prev = NULL;

    while (curr != NULL && !(curr->receiver == receiver && MASK_TEST_ANY(mask, curr->mask))) {
        prev = curr;
        curr = curr->next;
    }

    // No message was found
    if (curr == NULL) {
        return NULL;
    }

    if (curr == list->head && curr == list->tail) {
        // Only 1 element in the list
        list->head = NULL;
        list->tail = NULL;
    } else if (curr == list->head) {
        // Msg is 1st element in the list
        list->head = curr->next;
    } else if (curr == list->tail) {
        // Msg is last element in the list
        list->tail = prev;
        prev->next = NULL;
    } else {
        // Msg is somewhere in the middle
        prev->next = curr->next;
    }

    curr->next = NULL;
    list->length -= 1;

    return curr;
}

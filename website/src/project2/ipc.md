# Interprocess Communication

Interprocess communication (IPC) is an important part of any operating system. It allows isolated tasks to communicate and send data to and from each other. In modern operating system, IPC is normally accomplished in two ways, shared memory and message passing.

Shared memory is when multiple processes can access the same block of memory. This creates a shared buffer for the processes to communicate with each other. With shared memory there needs to be some way to _lock_ a resource, so when one processes is in a critical section, only that process can access the shared data and other processes will wait. This is typically implemented using mutexes and semaphores.

Another method to accomplish interprocess communication is with message passing, which is the method we used for our RTOS. Message passing allows one process to synchronously (or asynchronously) send a message to another process using that processes id (pid). The data that is being sent is copied from the sender memory space to the receivers memory space. This removes any concern of race conditions that comes from multiple processes accessing the same resource.

## Message Passing API

Our message passing API is very similar to the [QNX-style message passing](http://www.qnx.com/developers/docs/6.5.0/index.jsp?topic=%2Fcom.qnx.doc.neutrino_sys_arch%2Fipc.html). Each process has a unique process ID. This ID can be used to send and receive messages. Our RTOS has 3 synchronous functions and 1 asynchronous function.

## Implementation Technique

We implemented message passing by using an _outgoing mailbox_ and message queue. Since a process can only be sending a single message at a time, we did not need a dynamic buffer to store the messages. A static buffer the size of `MAXTHREAD` would be sufficient. We also used a message queue to store the order of messages being sent. This was needed in order to be _fair_ and ensure that the first message sent to a process is the first message delivered.

```c
/**
 * This array represents an outgoing mailbox.
 * If Process[i] is in the SEND_BLOCK state then Messages[i] will be the message
 * it is trying to send.
 */
static MSG Messages[MAXTHREAD];

/**
 * This message queue stores the order of messages that a process is trying to send.
 * If a process is trying to send a message, the message will be in this queue.
 * This queue is needed in addition to the Messages array so that the order of outgoing messages
 * is first come first serve. When a process receives a message, it will receive the first message sent to it.
 */
static msg_queue_t msg_queue;
```

### Synchronous Communication

The following figure shows how a processes state transitions as it synchronously communicates.

![Message Passing State Diagram](https://i.imgur.com/GI6fvd6.png)

A process is blocked and cannot be scheduled when it is in the three blocked states,

- Send Block,
- Reply Block, and
- Receive Block

One restriction is that no periodic process can use the synchronous communication functions. This is because of their strict timing requirement. Blocking a periodic task can easily violate its scheduling. For this reason, and error is thrown when a periodic task attempts to call these functions.

A process can transition between these states by using the following functions defined in `os.h`,

```c
void Msg_Send(PID  id, MTYPE t, uint16_t* v);
PID  Msg_Recv(MASK m,           uint16_t* v);
void Msg_Rply(PID  id,          uint16_t r);
```

#### Send

`Msg_Send(p, t, v)` sends the data `v` to process `p` with the message type `t`. If process `p` is not already waiting for a message of type `t` (in Receive block state), then the sending process becomes blocked in the Send block state. On the other hand, if process `p` is waiting for a message of type `t` then the message is delivered immediately and the receiving process is transitioned to the Ready state.

The message is delivered to the receiving process by copying the data at the location of the `v` pointer, which is on the senders stack, to the receivers `request_info` data structure, which is located on the receivers stack. It is important that the data is copied to the receivers stack because when the original data sent goes out of scope on the senders stack, the receiver may still need access.

The core logic of the `Msg_Send` function is shown below

```c
PD *p_recv = &Process[request_info->msg_to];
MTYPE recv_mask = p_recv->req_params->msg_mask;

// Check if the recipent is waiting for a message that matches the type of message sent
if (p_recv->state == RECV_BLOCK && MASK_TEST_ANY(recv_mask, request_info->msg_mask)) {
    // If yes, change state of waiting process to ready and sender to reply block
    p_recv->state = READY;
    Cp->state = REPLY_BLOCK;

    // Add the message data and pid of sender to the receiving processes request info
    p_recv->req_params->msg_ptr_data = request_info->msg_ptr_data;
    p_recv->req_params->out_pid = Cp->process_id;
} else {
    // If not, sender process goes to send block state
    MSG *msg = &Messages[Cp->process_id];

    // Save message
    msg->data = request_info->msg_ptr_data;
    msg->mask = request_info->msg_mask;
    msg->receiver = request_info->msg_to;

    Cp->state = SEND_BLOCK;
}

Dispatch();
```

`Dispatch` is called at the end of sending because no matter what the sender will be in a block state and a new ready process must be selected to run.

#### Receive

`p = Msg_Recv(m, v)` receives the data `v` from process `p` that matches the message mask `m`. If no message was sent to the process calling this function then it is transitioned into the Receive block state. If there is a message waiting that matches the mask `m`, the message is immediately copied to the receivers memory stack and the state of the receiver is set to Ready. The state of the sender who sent the message is transitioned to Reply block, as that process is now waiting for a reply. As mentioned above, a message queue is used to remember the order of message being sent. When checking if any message has been sent to this process, we use the first in first out (FIFO) approach. If multiple messages have been sent, the message sent the earliest will be received first.

The core logic of the kernel message receive is shown below,

```c
// Get the first message that was sent to this process
MSG *msg = msg_find_receiver(&msg_queue, Cp->process_id, request_info->msg_mask);

// Check if there is a message waiting for this process
if (msg != NULL) {
    // If yes, change state to ready and set msg data on Cp's request info
    request_info->msg_ptr_data = msg->data;
    request_info->out_pid = msg->sender;

    Cp->state = READY;

    // Sender process now waiting for reply
    PD *sender = &Process[msg->sender];
    sender->state = REPLY_BLOCK;

    // Remove data from Messages
    msg->data = NULL;
    msg->receiver = -1;
    msg->sender = -1;
} else {
    // If not, set process to receive block state
    Cp->state = RECV_BLOCK;
}
```

#### Reply

`Msg_Rply(p, r);` replies to process `p` with the data `r`. If process `p` is not in the Reply block state, then the operation is a no-op. If process `p` is waiting for a reply, the data `r` is copied to `p`'s memory stack and its state is set to Ready. The core logic of the kernel message reply is shown below,

```c
PD *p_recv = &Process[request_info->msg_to];

// Check if process replying to is in reply block state
if (p_recv->state == REPLY_BLOCK) {
    // If yes, set state to ready and set its msg data
    p_recv->state = READY;

    p_recv->req_params->msg_data = request_info->msg_data;
} else {
    // If not, noop
}

Cp->state = READY;

Dispatch();
```

A `Dispatch` is needed after replying to a process because the process replied to may be a higher priority.

#### Sequence Diagrams

The sequence diagram for the case where the sender is waiting for a receiver is shown in the following figure,

![Send-Receive-Reply Communication 1](https://i.imgur.com/rLNtokW.png)

The next sequence diagram shows the case where the receiver is waiting for the sender to,

![Send-Receive-Reply Communication 2](https://i.imgur.com/Hvy8UbF.png)

### Asynchronous Communication

Since periodic tasks cannot communicate synchronously, we need another way for them to communicate. The solution to this is asynchronous communication. The function used to do this has the following signature.

```c
void Msg_ASend(PID id, MTYPE t, uint16_t v);
```

`Msg_ASend(p, t, v)` sends a message `v` to process `p` with type `t`. This is almost identical to how the synchronous send works, except the message data is passed by value instead of by reference. However, one major difference is that the sending process can never become blocked. If process `p` is waiting for a message of type `t` then the message is delivered immediately. If no message is waiting, then the operation is a no-op. This means that the message may never be sent to anyone.

The core logic of the kernel asynchronous send is shown below,

```c
// If sending to non-existent process, noop
if (p_recv->state == DEAD) {
    return;
}

// Check if recipent is waiting for a message of the same type
if (p_recv->state == RECV_BLOCK && MASK_TEST_ANY(recv_mask, request_info->msg_mask)) {
    // If yes, change state of waiting process to ready and sender to reply block
    p_recv->state = READY;

    // Add the message data and pid of sender to the receiving processes request info
    p_recv->req_params->msg_data = request_info->msg_data;

    // Since the sender passes data by value, we need to set the data ptr to NULL
    // so the receiver knows to look at the msg_data instead
    p_recv->req_params->msg_ptr_data = NULL;
    p_recv->req_params->out_pid = Cp->process_id;

    // Dispatch because awaiting process might be higher priority
    Dispatch();
} else {
    // If not, noop
}
```

We dispatch if the message was successfully sent to a process because the receiver may be a higher priority.

## Client-Server

A common use case of the message passing model is to have a server continuously waiting for messages and a bunch of clients sending it messages with commands. As a small test of our implementation we implemented this scenario using periodic tasks and LEDs.

In our program a round robin server task is waiting in an infinite loop for messages. When a message is received, that IO port on the board is set to HIGH and all other ports are set to LOW. We then start several periodic client tasks which simply send the server an asynchronous message telling it to enable a specific port. The code for this program is as follows,

```c
static PID server_pid;

void Server(void) {
    uint16_t x;
    for (;;) {
        Msg_Recv(ANY, &x);
        PORTB = 0x00;
        BIT_SET(PORTB, x);
    }
}

void Client(void) {
    uint16_t pin = Task_GetArg();
    for (;;) {
        Msg_ASend(server_pid, 1 << pin, pin);
        Task_Next();
    }
}

void setup_1(void) {
    // All outputs
    DDRB = 0xFF;
    PORTB = 0x00;

    server_pid = Task_Create_RR(Server, 0);

    int16_t speed = 5;
    int16_t n = 8;
    int16_t p = speed * n;

    int i;
    for (i = 0; i < n; i += 1) {
        Task_Create_Period(Client, i, p, 1, speed * i);
    }

    return;
}
```

The speed of the lights can be set through a single `speed` variable. The periodic client just sends an asynchronous message to the server and then yields execution until its next period. The result can be seen in the following gif.

![Client-Server Lights Loop](https://i.imgur.com/CrRRztz.gif)

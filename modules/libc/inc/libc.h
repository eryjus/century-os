

#include "types.h"
#include "ipc.h"


//
// -- SYSCALL 1: Receive a message
//    ----------------------------
extern "C" int ReceiveMessage(Message_t *msg);


//
// -- SYSCALL 2: Send a message
//    ----------------------------
extern "C" int SendMessage(PID_t pid, Message_t *msg);





#include "types.h"

//
// -- These are the messages that the PMM manager will respond to
//    -----------------------------------------------------------
typedef enum {
    PMM_NOOP,
    PMM_FREE_FRAME,
    PMM_ALLOC_FRAME,
    PMM_FREE_RANGE,
    PMM_ALLOC_RANGE,
    PMM_NEW_FRAME,
} PmmMessages_t;




frame_t PmmAllocFrame(void);
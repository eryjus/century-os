#define DEBUG_TOKEN_PASTE(x) DEBUG_##x
#define DEBUG_ENABLED(func) DEBUG_TOKEN_PASTE(func)>0
#define DEBUG_MmuMapToFrame 0
#define DEBUG_LApicBroadcastIpi 1
#define DEBUG_IpiHandleTlbFlush 1
#define DEBUG_PicBroadcastIpi 1
#define DEBUG_PicMaskIrq 1
#define DEBUG_PicUnmaskIrq 1
#define DEBUG_IsrHandler 1

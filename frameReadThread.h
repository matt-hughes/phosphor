#ifndef _frameReadThread_h_
#define _frameReadThread_h_
#ifdef __cplusplus
extern "C" {
#endif

#include "usbSerialDecoder.h"

int frameReadThread_init(void);
int frameReadThread_getNewFrame(usbSerialDecoder_frameData* frameData);

#ifdef __cplusplus
} // extern "C"
#endif
#endif

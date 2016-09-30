#ifndef _usbSerialDecoder_h_
#define _usbSerialDecoder_h_

#ifdef __cplusplus
extern "C" {
#endif

#define USBSERIALDECODER_NROWS	25
#define USBSERIALDECODER_NCOLS	80

typedef struct
{
	unsigned char frame_buffer[USBSERIALDECODER_NROWS][USBSERIALDECODER_NCOLS+1];
	unsigned char flags_buffer[USBSERIALDECODER_NCOLS];
} usbSerialDecoder_frameData;

int usbSerialDecoder_open(void);
void usbSerialDecoder_close(void);
int usbSerialDecoder_readFrame(usbSerialDecoder_frameData* frame);

#ifdef __cplusplus
} // extern "C"
#endif
#endif

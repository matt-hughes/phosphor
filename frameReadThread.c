#include "frameReadThread.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

struct
{
    int run;
    int open;
    pthread_t thread;
    pthread_mutex_t mutex;
    usbSerialDecoder_frameData frameData;
    int haveFrame;
} gFrameReadThread;

static void* frameReadThreadFn(void* args)
{
    while(gFrameReadThread.run)
    {
        if(gFrameReadThread.open)
        {
            usbSerialDecoder_frameData tmpFrame;
            int res = usbSerialDecoder_readFrame(&tmpFrame);
            if(res < 0)
            {
                fprintf(stderr, "Read failed. Closing usb device.\n");
                usbSerialDecoder_close();
                gFrameReadThread.open = 0;
            }
            else if(res > 0)
            {
                pthread_mutex_lock(&gFrameReadThread.mutex);
                memcpy(&gFrameReadThread.frameData, &tmpFrame, sizeof(usbSerialDecoder_frameData));
                gFrameReadThread.haveFrame = 1;
                pthread_mutex_unlock(&gFrameReadThread.mutex);
            }
        }
        else
        {
            if(usbSerialDecoder_open() < 0)
            {
                fprintf(stderr, "Waiting for usb device...\n");
                sleep(1);
            }
            else
            {
                gFrameReadThread.open = 1;
            }
        }
    }
}

int frameReadThread_init(void)
{
    gFrameReadThread.run = 1;
    gFrameReadThread.open = 0;
    gFrameReadThread.haveFrame = 0;

    pthread_mutex_init(&gFrameReadThread.mutex, NULL);

    if(pthread_create(&gFrameReadThread.thread, NULL, frameReadThreadFn, NULL) != 0)
    {
        fprintf(stderr, "Failed to create thread!\n");
        return -1;
    }

    return 0;
}

int frameReadThread_getNewFrame(usbSerialDecoder_frameData* frameData)
{
    int haveFrame;
    pthread_mutex_lock(&gFrameReadThread.mutex);
    haveFrame = gFrameReadThread.haveFrame;
    gFrameReadThread.haveFrame = 0;
    if(haveFrame)
    {
        memcpy(frameData, &gFrameReadThread.frameData, sizeof(usbSerialDecoder_frameData));
    }
    pthread_mutex_unlock(&gFrameReadThread.mutex);
    return haveFrame;
}

// [END]

#include "bcm_host.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdio.h>

#define ERROR_AT() printf("Error at %s:%d\n", __FILE__, __LINE__)

EGLDisplay display;
EGLContext context;
EGLSurface surface;

int rpi_egl_init(int screenW, int screenH)
{
    static EGL_DISPMANX_WINDOW_T nativewindow;

    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    int display_width;
    int display_height;

    bcm_host_init();

    // create an EGL window surface, passing context width/height
    int success = graphics_get_display_size(0, &display_width, &display_height);
    if ( success < 0 )
    {
        ERROR_AT();
        return -1;
    }

    printf("Display size: %d x %d\n", display_width, display_height);

    // You can hardcode the resolution here:
    display_width = screenW;
    display_height = screenH;

    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = display_width;
    dst_rect.height = display_height;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = display_width << 16;
    src_rect.height = display_height << 16;

    dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
    dispman_update = vc_dispmanx_update_start( 0 );

    dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display, 0/*layer*/, &dst_rect, 0/*src*/, &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);

    nativewindow.element = dispman_element;
    nativewindow.width = display_width;
    nativewindow.height = display_height;
    vc_dispmanx_update_submit_sync( dispman_update );

    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLConfig config;
    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };


    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if ( display == EGL_NO_DISPLAY )
    {
        ERROR_AT();
        return -1;
    }

   // Initialize EGL
    if ( !eglInitialize(display, &majorVersion, &minorVersion) )
    {
        ERROR_AT();
        return -1;
    }

    // Get configs
    if ( !eglGetConfigs(display, NULL, 0, &numConfigs) )
    {
        ERROR_AT();
        return -1;
    }

    EGLint attribList[] =
    {
        EGL_RED_SIZE,       5,
        EGL_GREEN_SIZE,     6,
        EGL_BLUE_SIZE,      5,
        EGL_ALPHA_SIZE,     EGL_DONT_CARE,
        EGL_DEPTH_SIZE,     EGL_DONT_CARE,
        EGL_STENCIL_SIZE,   EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_NONE
    };

    // Choose config
    if ( !eglChooseConfig(display, attribList, &config, 1, &numConfigs) )
    {
        ERROR_AT();
        return -1;
    }

    // Create a surface
    surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)&nativewindow, NULL);
    if ( surface == EGL_NO_SURFACE )
    {
        ERROR_AT();
        return -1;
    }

    // Create a GL context
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs );
    if ( context == EGL_NO_CONTEXT )
    {
        ERROR_AT();
        return -1;
    }   

    // Make the context current
    if ( !eglMakeCurrent(display, surface, surface, context) )
    {
        ERROR_AT();
        return -1;
    }

    return 0;
}

int rpi_egl_swap(void)
{
    eglSwapBuffers(display, surface);
}



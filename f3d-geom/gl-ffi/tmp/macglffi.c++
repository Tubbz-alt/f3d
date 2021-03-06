// This file is compiled and linked by c/links/Makefile

// It must be conditionalized according to architecture.

// The AGL API is deprecated and unsupported for 64-bit applications.
// Although we keep the AGL flag here, this is the Cocoa API, not AGL.

#if defined(AGL)

// What remains are callouts to C functions that are defined in the
// nsglffi.m file...

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

#include "namespace.h"

#define PIXELFORMATDESCRIPTOR GLint

BEGIN_NAMESPACE_FREEDIUS

// *************   chooseAndSetPixelformat  *************

struct glRenderingSpec
{
  int RgbaFlag;		/* configuration flags (ala GLX parameters) */
  int ColorBits;
  int DoubleFlag;
  int DepthSize;
  int AccumBits;
  int AlphaSize;
  int StencilSize;
  int OverlayFlag;
  int StereoFlag;
  int AuxNumber;
};

extern "C" {


struct glRenderingSpec *
FREEDIUS_GLOBAL(aglMakeRenderingSpec)()
{ return (struct glRenderingSpec *) malloc(sizeof(glRenderingSpec));
}


// if error dump agl errors to debugger string, return error
static int aglReportError(void)
{
  // Need to determine if there was an error, and if so, report it...
  return (int) 0;
}

} // end extern "C"



void set_pixelformatdescriptor (PIXELFORMATDESCRIPTOR *pfd, 
				struct glRenderingSpec *spec)
{
  int n;

  n = 0;
  // Not really good.  We need to rewrite some of the structs.
  if (spec->RgbaFlag) pfd[n++] = AGL_RGBA;

  pfd[n++] = NSOpenGLPFAColorSize;
  pfd[n++] = 8;
  pfd[n++] = NSOpenGLPFADepthSize;
  pfd[n++] = 8;

  if (spec->DoubleFlag) pfd[n++] = NSOpenGLPFADoubleBuffer;
  
  if (spec->StereoFlag) pfd[n++] = NSOpenGLPFAStereo;

  /* Alpha bitplanes are not supported in the current generic OpenGL
   * implementation, but may be supported by specific hardware devices.
   */

  if (spec->StencilSize > 0) {
    pfd[n++] = NSOpenGLPFAStencilSize;
    pfd[n++] = spec->StencilSize;
  }

  pfd[n++] = NSOpenGLPFAAccumSize;
  pfd[n++] = 32;

  pfd[n++] = NSOpenGLPFAAccelerated;

  if (spec->AlphaSize > 0) {
    pfd[n++] = AGL_ALPHA_SIZE;
    pfd[n++] = spec->AlphaSize;
  }

  pfd[n++] = AGL_DEPTH_SIZE;
  pfd[n++] = spec->DepthSize;


  /* Auxiliary buffers are not supported in the current generic OpenGL
   * implementation, but may be supported by specific hardware devices.
   */

  if (spec->AuxNumber > 0) {
    pfd[n++] = AGL_AUX_BUFFERS;
    pfd[n++] = spec->AuxNumber;
  }

  //  pfd[n++] = AGL_BACKING_STORE;
  pfd[n++] = AGL_ALL_RENDERERS;
  pfd[n] = AGL_NONE;
}





extern "C" {

  // return 0 on failure, the chosen pixel format on success.
  // The contents of spec are set to the actual values chosen.
  // 
AGLPixelFormat
FREEDIUS_GLOBAL(aglChooseAndSetPixelformat) (struct glRenderingSpec *spec)
{
  PIXELFORMATDESCRIPTOR pfd[64];
  AGLPixelFormat pixelformat, save;
  AGLRendererInfo rinfo;
  AGLDevice *devs;
  GLint i, ndevs;
  int k;

  set_pixelformatdescriptor(pfd, spec);
  
  save = pixelformat = aglChoosePixelFormat(NULL, 0, pfd);
  devs = aglDevicesOfPixelFormat(pixelformat, &ndevs);

  printf("%d devices support this format\n", ndevs);
  printf("Dev 0: %d\n", devs[0]);

  // we can do this later...
  aglReportError();
  k = i = 0;
  while (!i && k < 10) {
    aglDescribePixelFormat(pixelformat, AGL_ACCELERATED, &i);
    if (i) printf("Pixel format %d is accelerated\n", k);
    else {
      printf("Pixel format %d is NOT accelerated.\n", k);
      pixelformat = aglNextPixelFormat(pixelformat);
    }
    k++;
  }

  if (!pixelformat) {
    printf("No accelerated format was found.  Falling back to first valid format.\n");
    pixelformat = save;
  }

#if 0
  /* Get the actual pixel format */
  DescribePixelFormat(GLHdc, pixelformat, sizeof(pfd), &pfd);

  // Modify the values in spec according to what was actually obtained
  read_pixelformatdescriptor(&pfd, spec);
#endif
  
  return(pixelformat);
}

} // end extern "C"




extern "C" {


  static void SetMacBufRect(MacDrawable *macWin, AGLContext ctx, int width, int height) {
    WindowRef window;
    CGrafPtr grafPtr;
    Rect rectPort;
    GLint wrect[4];

    grafPtr = macWin->toplevel->grafPtr;
    window = GetWindowFromPort(grafPtr);

    GetWindowPortBounds (window, &rectPort);

    wrect[2] = width;
    wrect[3] = height;
    wrect[0] = macWin->xOff;
    wrect[1] = rectPort.bottom - wrect[3] - macWin->yOff;

    aglSetInteger(ctx, AGL_BUFFER_RECT, wrect);
    aglEnable(ctx, AGL_BUFFER_RECT);
    aglUpdateContext(ctx);
  }


  static void InvalidateWindow(MacDrawable *macWin, int width, int height) {
    CGrafPtr grafPtr;
    WindowRef window;
    GrafPtr portSave = NULL;
    Rect rectPort;
    GLint wrect[4];

    grafPtr = macWin->toplevel->grafPtr;
    window = GetWindowFromPort(grafPtr);
    SetPort(grafPtr);

    rectPort.left = macWin->xOff;
    rectPort.right = rectPort.left + width - 1;
    rectPort.top = macWin->yOff;
    rectPort.bottom = rectPort.top + height - 1;

    InvalWindowRect(window, &rectPort);
  }


  AGLContext FREEDIUS_GLOBAL(aglCreateContext) (MacDrawable *macWin, AGLPixelFormat pixelformat, AGLContext share, int width, int height) {
    AGLContext ctx;
    CGrafPtr window;
    GLint swap = 0; // Was 1.

    window = macWin->toplevel->grafPtr;

    ctx = aglCreateContext(pixelformat, share);
    aglReportError();

    aglDestroyPixelFormat(pixelformat);

    printf("\nAbout to call SetDrawable...\n");  fflush(stdout);
    if (!aglSetDrawable(ctx, window)) {
      printf("aglSetDrawable failed.\n");
      aglReportError();
      return 0;
    }

    if (!aglSetCurrentContext(ctx)) {
      printf("aglSetCurrentContext failed.\n");
      aglReportError ();
      return 0;
    }

    InvalidateWindow(macWin, width, height);
    SetMacBufRect(macWin, ctx, width, height);

    if (!aglSetInteger (ctx, AGL_SWAP_INTERVAL, &swap)) {
      aglReportError ();
      return 0;
    }

    return ctx;
  }


  // Much of this is from the "Render" togl function.  Basically, set
  // up the window's rectangle and invalidate it, thus forcing Apple's
  // graphics system to flush the pixels onto the screen:

  void  FREEDIUS_GLOBAL(aglSwapBuffers)(MacDrawable *macWin, AGLContext ctx, int width, int height) {
    aglSwapBuffers(ctx);
  }


  void  FREEDIUS_GLOBAL(aglPreDisplay)(MacDrawable *macWin, AGLContext ctx, int width, int height) {
    GrafPtr curPort;

    //    GetPort(&curPort);
    SetPort(macWin->toplevel->grafPtr);
    aglSetCurrentContext(ctx);
    InvalidateWindow(macWin, width, height);
    SetMacBufRect(macWin, ctx, width, height);
    //    aglSwapBuffers(ctx);
    //    SetPort(curPort);
  }


  // Not used:
  void  FREEDIUS_GLOBAL(aglPostDisplay)(MacDrawable *macWin, AGLContext ctx, int width, int height) {
    CGrafPtr grafPtr;
    WindowRef window;
    GrafPtr portSave = NULL;
    Rect rectPort;

    grafPtr = macWin->toplevel->grafPtr;
    window = GetWindowFromPort(grafPtr);
    SetPort(grafPtr);

    rectPort.left = macWin->xOff;
    rectPort.right = rectPort.left + width - 1;
    rectPort.top = macWin->yOff;
    rectPort.bottom = rectPort.top + height - 1;

    InvalWindowRect(window, &rectPort);
  }


  // This section assumes that the display never changes - I can
  // imagine that this assumption will be wrong if we move a frame
  // from one screen to another - not sure though.

  // static CGDirectDisplayID DispID = 0;
  // static Boolean old_state;

  // This call always returns the main display ID, which is the
  // display containing the menu bar.  For now, a reasonable default:
  //  
  // CGDirectDisplayID DispID =  CGMainDisplayID();
  //
  // Also, note that show and hide cursor calls ignore the display ID,
  // per Apple's documentation.  Note also that the show and hide
  // operations generally only work if FREEDIUS is the foreground
  // operation.  It may be necessary to insert calls to aglShowCursor
  // in the repl as a catch-all to get the mouse to reappear.

#if 1 // Was 0

  // Bad, but...
  static Boolean old_state;

  int FREEDIUS_GLOBAL(aglHideCursor)() {
    //    CGDirectDisplayID DispID = CGMainDisplayID();
    //    if (DispID == 0) DispID = CGMainDisplayID();
    SetMouseCoalescingEnabled(0, &old_state);
    if (IsMouseCoalescingEnabled()) printf("Mouse coalescing is enabled.\n");
    else printf("Mouse coalescing is disabled.\n");
    CGDisplayHideCursor(kCGDirectMainDisplay);
  }

  int FREEDIUS_GLOBAL(aglShowCursor)() {
    //    CGDirectDisplayID DispID = CGMainDisplayID();
    //    if (DispID == 0) DispID = CGMainDisplayID();
    SetMouseCoalescingEnabled(old_state, &old_state);
    CGDisplayShowCursor(kCGDirectMainDisplay);
  }
#else
  int FREEDIUS_GLOBAL(aglHideCursor)() {
    CGDisplayHideCursor(kCGDirectMainDisplay);
  }

  int FREEDIUS_GLOBAL(aglShowCursor)() {
    CGDisplayShowCursor(kCGDirectMainDisplay);
  }

/*  MOUSE COALESCING

If mouse coalescing is enabled, intermediate mouse movement events are
merged into the most recent event, so that only one mouse moved or
mouse dragged event is in the event queue at any time. For example,
when the user moves the mouse across the screen, more mouse moved
events are generated than most applications care about. Rather than
place all these events in the queue (which would probably slow down
the application), the Carbon Event Manager first checks to see if a
mouse moved event already exists. If a mouse moved event already
exists, that event is updated with the position and delta information
from the more recently-generated event.
*/

  int FREEDIUS_GLOBAL(aglSetMouseCoalescing) (Boolean state) {
    Boolean old_state;
    SetMouseCoalescingEnabled(state, &old_state);
    return old_state;
  }
    
#endif

/* CGWarpMouseCursorPosition
 * Warp the mouse cursor to the desired position in global
 * coordinates without generating events
 */
/* CGDisplayMoveCursorToPoint
 * Move the cursor to the specified point relative to the display origin
 * (the upper left corner of the display).  Returns CGDisplayNoErr on success.
 * No events are generated as a result of this move.
 * Points that would lie outside the desktop are clipped to the desktop.
 */
/* CGSetLocalEventsSuppressionInterval
 * Set the period of time in seconds that local hardware events (keyboard and mouse)
 * are suppressed after posting an event.  Defaults to 0.25 second.
 */

  int FREEDIUS_GLOBAL(aglWarpCursor)(int screenx, int screeny) {
    CGPoint pt;
    CGDirectDisplayID DispID = CGMainDisplayID();
    pt.x = (float) screenx;
    pt.y = (float) screeny;
#if 0
    CGSetLocalEventsSuppressionInterval(0);
#endif
    return CGDisplayMoveCursorToPoint(DispID, pt);
  }

  int FREEDIUS_GLOBAL(aglAssociateMouseAndMouseCursorPosition) (int connected) {
    return CGAssociateMouseAndMouseCursorPosition(connected);
  }

  void FREEDIUS_GLOBAL (aglGetLastMouseDelta) (CGMouseDelta delta[]) {
    CGGetLastMouseDelta(&delta[0], &delta[1]);
  }


}


END_NAMESPACE_FREEDIUS

#endif // defined(AGL)



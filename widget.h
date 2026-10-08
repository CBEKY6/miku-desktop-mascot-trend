/**********************************************************************************************
*
*   widget - desktop widget behaviour for the mascot window
*
*   The window is created by raylib, which picks a GLFW backend on its own: a native Wayland
*   client when the library has Wayland support and WAYLAND_DISPLAY is set, X11 (or XWayland)
*   otherwise. Everything in widget.c is therefore gated twice:
*
*     - at compile time, so an X11-only, a Wayland-only and a DRM build each compile;
*     - at run time, because under native Wayland GetWindowHandle() returns a wl_surface*
*       and handing that pointer to Xlib is undefined behaviour. That is exactly what the
*       old ForceAlwaysOnTopLinux() did, which is why the widget behaviour silently stopped
*       working on native Wayland sessions.
*
**********************************************************************************************/

#ifndef WIDGET_H
#define WIDGET_H

#include <raylib.h>

#if defined(__cplusplus)
extern "C" {
#endif

// Window flags to pass to SetConfigFlags() before InitWindow().
unsigned int WidgetConfigFlags(void);

// Turns the window that raylib just created into a desktop widget: drawn above other windows,
// absent from the taskbar and the window switcher, and never stealing keyboard focus.
// Mouse clicks are passed through to whatever is underneath, including fully transparent
// pixels. Safe to call on X11, XWayland, native Wayland and DRM; features that the current
// backend cannot express are skipped and reported on stdout.
void WidgetMakeOverlay(void);

// Call once per frame. Keeps the mascot below fullscreen windows (X11/XWayland only).
void WidgetUpdate(void);

// "wayland", "x11", "drm" or "unknown", for logging.
const char *WidgetDisplayServer(void);

#if defined(__cplusplus)
}
#endif

#endif // WIDGET_H
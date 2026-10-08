/**********************************************************************************************
*
*   widget - desktop widget behaviour for the mascot window
*
*   See widget.h for the rationale.
*
**********************************************************************************************/

#include "widget.h"

#include <stdlib.h>

#if defined(PLATFORM_DRM)
    #define WIDGET_DRM 1
#endif

// WIDGET_USE_X11 is defined by the build system when the X11 headers are available.
#if defined(WIDGET_USE_X11) && !defined(WIDGET_DRM)
    #define Font XFont
    #include <X11/Xlib.h>
    #include <X11/Xatom.h>
    #include <X11/Xutil.h>
    #undef Font
    #define WIDGET_X11 1
#endif

// raylib added FLAG_WINDOW_MOUSE_PASSTHROUGH in 4.2 ("Support MOUSE_PASSTHROUGH", #2516) and
// moved the implementation onto GLFW 3.4 in 5.5. It is an enum member rather than a macro, so
// #ifdef cannot see it and the version has to be checked instead.
#if RAYLIB_VERSION_MAJOR > 4 || (RAYLIB_VERSION_MAJOR == 4 && RAYLIB_VERSION_MINOR >= 2)
    #define WIDGET_MOUSE_PASSTHROUGH 1
#endif

// Asking the compositor about fullscreen windows on every frame is wasteful, so poll.
#define WIDGET_FULLSCREEN_POLL_INTERVAL 6

typedef enum {
    SERVER_UNKNOWN,
    SERVER_X11,
    SERVER_XWAYLAND,
    SERVER_WAYLAND,
    SERVER_DRM
} DisplayServer;

static DisplayServer server = SERVER_UNKNOWN;

#if defined(WIDGET_X11)
// Set once WidgetMakeOverlay() has run, so WidgetUpdate() knows the hints actually landed.
static bool widgetActive = false;
static Display *xDisplay = NULL;
static Window xWindow = 0;
static Atom netActiveWindow = None;
static bool hiddenBelowFullscreen = false;
static int framesSinceFullscreenPoll = WIDGET_FULLSCREEN_POLL_INTERVAL;
#endif

// Clicks are handled by raylib itself, on the same backend as the window: XShape under X11,
// a wl_region under Wayland.
unsigned int WidgetConfigFlags(void) {
    unsigned int flags = FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_TOPMOST;

#if defined(WIDGET_MOUSE_PASSTHROUGH)
    flags |= FLAG_WINDOW_MOUSE_PASSTHROUGH;
#endif

    return flags;
}

const char *WidgetDisplayServer(void) {
    switch (server) {
        case SERVER_X11: return "x11";
        case SERVER_XWAYLAND: return "wayland (xwayland)";
        case SERVER_WAYLAND: return "wayland";
        case SERVER_DRM: return "drm";
        default: return "unknown";
    }
}

#if defined(WIDGET_X11)

// Xlib's default handler prints the error and terminates the process, so a probe against a
// window id that belongs to another display server would kill the mascot. Swallow errors for
// the duration of the probe only, then hand the previous handler back to GLFW.
static int IgnoreXError(Display *display, XErrorEvent *event) {
    (void)display;
    (void)event;
    return 0;
}

static bool IsX11Window(Display *display, Window window) {
    XWindowAttributes attributes;
    XErrorHandler previous = XSetErrorHandler(IgnoreXError);
    int status = XGetWindowAttributes(display, window, &attributes);
    XSync(display, False);
    XSetErrorHandler(previous);

    return status != 0;
}

static void RequestX11States(Display *display, Window window, const char *first, const char *second) {
    Atom netWmState = XInternAtom(display, "_NET_WM_STATE", False);

    Atom states[2] = { None, None };
    int count = 0;

    if (first != NULL) states[count++] = XInternAtom(display, first, False);
    if (second != NULL) states[count++] = XInternAtom(display, second, False);
    if (count == 0) return;

    XEvent event = { 0 };
    event.xclient.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = netWmState;
    event.xclient.format = 32;
    event.xclient.data.l[0] = 1;                                   // 1 = add, 0 = remove
    event.xclient.data.l[1] = (long)states[0];
    event.xclient.data.l[2] = (long)states[1];

    XSendEvent(display, DefaultRootWindow(display), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &event);
    XFlush(display);
}

static void SetX11WindowType(Display *display, Window window, const char *type) {
    Atom netWmWindowType = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom value = XInternAtom(display, type, False);

    XChangeProperty(display, window, netWmWindowType, XA_ATOM, 32, PropModeReplace,
                    (const unsigned char *)&value, 1);
}

// A widget must never take keyboard focus away from the window the user is typing in.
static void SetX11NoFocus(Display *display, Window window) {
    XWMHints hints = { 0 };
    hints.flags = InputHint | StateHint;
    hints.input = False;
    hints.initial_state = NormalState;

    XSetWMHints(display, window, &hints);
}

static bool QueryActiveWindowIsFullscreen(Display *display) {
    Atom actualType = None;
    int actualFormat = 0;
    unsigned long items = 0, bytesAfter = 0;
    unsigned char *data = NULL;

    if (!netActiveWindow) netActiveWindow = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);

    Window active = None;
    if (XGetWindowProperty(display, DefaultRootWindow(display), netActiveWindow, 0, 1, False,
                           XA_WINDOW, &actualType, &actualFormat, &items, &bytesAfter,
                           &data) != Success || data == NULL) return false;

    if (items > 0) active = (Window)(*(unsigned long *)data);
    XFree(data);

    // Ignore our own window: it must not duck below itself.
    if (active == None || active == xWindow) return false;

    Atom netWmState = XInternAtom(display, "_NET_WM_STATE", False);
    Atom netFullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
    bool fullscreen = false;

    if (XGetWindowProperty(display, active, netWmState, 0, 32, False, XA_ATOM,
                           &actualType, &actualFormat, &items, &bytesAfter, &data) == Success &&
        data != NULL) {
        Atom *states = (Atom *)data;
        for (unsigned long i = 0; i < items; i++) {
            if (states[i] == netFullscreen) {
                fullscreen = true;
                break;
            }
        }
        XFree(data);
    }

    return fullscreen;
}

// The active window can be destroyed between the two property reads above, which would reach
// Xlib's default handler and terminate the process. Same treatment as IsX11Window().
static bool ActiveWindowIsFullscreen(Display *display) {
    XErrorHandler previous = XSetErrorHandler(IgnoreXError);
    bool fullscreen = QueryActiveWindowIsFullscreen(display);
    XSync(display, False);
    XSetErrorHandler(previous);

    return fullscreen;
}

#endif  // WIDGET_X11

void WidgetMakeOverlay(void) {
#if defined(WIDGET_DRM)
    server = SERVER_DRM;
    TraceLog(LOG_INFO, "WIDGET: DRM session, no window manager to talk to");
    return;
#else

#if defined(WIDGET_MOUSE_PASSTHROUGH)
    TraceLog(LOG_INFO, "WIDGET: click-through enabled (FLAG_WINDOW_MOUSE_PASSTHROUGH)");
#else
    TraceLog(LOG_WARNING, "WIDGET: click-through needs raylib 4.2 or newer");
#endif

#if defined(WIDGET_X11)
    // An unset WAYLAND_DISPLAY means a plain X11 session. Otherwise the window is a native
    // Wayland surface unless XWayland is in use, which is only knowable by looking at it.
    bool waylandSession = (getenv("WAYLAND_DISPLAY") != NULL);

    Display *display = XOpenDisplay(NULL);
    if (display == NULL) {
        server = waylandSession ? SERVER_WAYLAND : SERVER_UNKNOWN;
        TraceLog(LOG_WARNING, "WIDGET: no X11 display reachable, window manager hints skipped");
        TraceLog(LOG_INFO, "WIDGET: only click-through applies here; see issue #2 for the rest");
        return;
    }

    Window window = *(Window *)GetWindowHandle();
    if (window == None || !IsX11Window(display, window)) {
        // Native Wayland: GetWindowHandle() is a wl_surface*, not a window id. Touching X11
        // here would be undefined behaviour, so leave the window alone.
        server = waylandSession ? SERVER_WAYLAND : SERVER_UNKNOWN;
        XCloseDisplay(display);
        TraceLog(LOG_WARNING, "WIDGET: native Wayland surface, X11 window manager hints skipped");
        TraceLog(LOG_INFO, "WIDGET: only click-through applies here; see issue #2 for the rest");
        return;
    }

    // DOCK windows are panels: compositors reserve space for them and snap them to an edge,
    // which fights with SetWindowPosition(). An ordinary window with explicit EWMH states
    // stays where it was told to and still skips the taskbar.
    SetX11WindowType(display, window, "_NET_WM_WINDOW_TYPE_NORMAL");
    RequestX11States(display, window, "_NET_WM_STATE_ABOVE", NULL);
    RequestX11States(display, window, "_NET_WM_STATE_STAYS_ON_TOP", NULL);
    RequestX11States(display, window, "_NET_WM_STATE_SKIP_TASKBAR", "_NET_WM_STATE_SKIP_PAGER");
    SetX11NoFocus(display, window);

    xDisplay = display;
    xWindow = window;
    server = waylandSession ? SERVER_XWAYLAND : SERVER_X11;
    widgetActive = true;
    framesSinceFullscreenPoll = WIDGET_FULLSCREEN_POLL_INTERVAL;

    TraceLog(LOG_INFO, "WIDGET: %s session, always on top, hidden from taskbar, no focus stealing",
             WidgetDisplayServer());
#else
    server = (getenv("WAYLAND_DISPLAY") != NULL) ? SERVER_WAYLAND : SERVER_UNKNOWN;
    TraceLog(LOG_WARNING, "WIDGET: built without X11, window manager hints unavailable");
    TraceLog(LOG_INFO, "WIDGET: only click-through applies here; see issue #2 for the rest");
#endif

#endif  // WIDGET_DRM
}

void WidgetUpdate(void) {
#if defined(WIDGET_X11)
    if (!widgetActive) return;

    if (++framesSinceFullscreenPoll < WIDGET_FULLSCREEN_POLL_INTERVAL) return;
    framesSinceFullscreenPoll = 0;

    bool fullscreen = ActiveWindowIsFullscreen(xDisplay);
    if (fullscreen == hiddenBelowFullscreen) return;

    hiddenBelowFullscreen = fullscreen;
    RequestX11States(xDisplay, xWindow,
                     fullscreen ? "_NET_WM_STATE_BELOW" : "_NET_WM_STATE_ABOVE", NULL);
#endif
}
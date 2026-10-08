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

// A rectangle in screen coordinates.
typedef struct {
    int x, y, width, height;
} WidgetArea;

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

// _NET_WORKAREA holds four CARD32 values per monitor: x, y, width, height, with the panels
// already subtracted. The entries follow the window manager's own monitor order, which is not
// necessarily raylib's, so `index` is matched against the monitor geometry rather than trusted
// positionally.
static bool QueryX11WorkArea(Display *display, int index, WidgetArea *area) {
    Atom actualType = None;
    int actualFormat = 0;
    unsigned long items = 0, bytesAfter = 0;
    unsigned char *data = NULL;
    Atom netWorkArea = XInternAtom(display, "_NET_WORKAREA", False);
    long offset = (long)index * 4;

    if (XGetWindowProperty(display, DefaultRootWindow(display), netWorkArea, offset, 4, False,
                           XA_CARDINAL, &actualType, &actualFormat, &items, &bytesAfter,
                           &data) != Success || data == NULL) return false;

    if (items < 4) {
        XFree(data);
        return false;
    }

    long *values = (long *)data;
    area->x = (int)values[0];
    area->y = (int)values[1];
    area->width = (int)values[2];
    area->height = (int)values[3];
    XFree(data);

    return true;
}

// Reads the work area of monitor `index` (raylib's numbering), or the primary monitor's when the
// window manager has fewer entries than raylib has monitors.
static bool X11WorkArea(Display *display, int index, WidgetArea *area) {
    XErrorHandler previous = XSetErrorHandler(IgnoreXError);
    bool found = QueryX11WorkArea(display, index, area);
    XSync(display, False);
    XSetErrorHandler(previous);

    if (found) return true;

    // Some window managers only publish the primary monitor. Its work area still covers our
    // monitor whenever our monitor is at the same origin, which is the single-monitor case.
    if (index > 0 && QueryX11WorkArea(display, 0, area)) {
        return true;
    }

    return false;
}

#endif  // WIDGET_X11

// Index of the monitor the mascot should appear on. raylib's GetCurrentMonitor() reports the
// monitor the *window* is on, which is not what is wanted here, so the pointer is matched
// against the monitor rectangles directly.
static int ResolveMonitor(WidgetMonitor monitor) {
    int count = GetMonitorCount();
    if (count <= 0) return 0;

    if (monitor == WIDGET_MONITOR_CURSOR) {
        Vector2 pointer = GetMousePosition();

        for (int i = 0; i < count; i++) {
            Vector2 origin = GetMonitorPosition(i);
            int width = GetMonitorWidth(i);
            int height = GetMonitorHeight(i);

            if (pointer.x >= origin.x && pointer.x < origin.x + width &&
                pointer.y >= origin.y && pointer.y < origin.y + height) {
                return i;
            }
        }

        TraceLog(LOG_WARNING, "WIDGET: no monitor under the pointer, using the primary one");
        return 0;
    }

    if (monitor >= 0 && monitor < count) return (int)monitor;

    TraceLog(LOG_WARNING, "WIDGET: monitor %d does not exist (%d found), using the primary one",
             (int)monitor, count);
    return 0;
}

// Area of the given monitor that is free of panels. raylib knows the monitor geometry but not the
// panel, so X11 gets asked via _NET_WORKAREA; on native Wayland nothing reports panels, which
// means a bottom panel will overlap the mascot.
static WidgetArea MonitorWorkArea(int index) {
    WidgetArea area;
    Vector2 origin = GetMonitorPosition(index);

    area.x = (int)origin.x;
    area.y = (int)origin.y;
    area.width = GetMonitorWidth(index);
    area.height = GetMonitorHeight(index);

#if defined(WIDGET_X11)
    if (widgetActive) {
        WidgetArea workArea;
        // A work area smaller than the monitor means it describes different geometry, or a
        // panel is eating more than half the screen. Either way it is not worth trusting.
        if (X11WorkArea(xDisplay, index, &workArea) && workArea.width >= area.width &&
            workArea.height >= area.height) {
            area = workArea;
        }
    }
#endif

    return area;
}

void WidgetAnchor(int windowWidth, int windowHeight, WidgetCorner corner, WidgetMonitor monitor,
                  int margin) {
    if (margin < 0) margin = 0;

    int index = ResolveMonitor(monitor);
    WidgetArea area = MonitorWorkArea(index);

    if (area.width <= 0 || area.height <= 0) {
        TraceLog(LOG_WARNING, "WIDGET: unknown monitor size, window left where it was");
        return;
    }

    int x, y;
    const char *name = "bottom-right";

    switch (corner) {
        case WIDGET_CORNER_BOTTOM_LEFT:
            x = area.x + margin;
            y = area.y + area.height - windowHeight - margin;
            name = "bottom-left";
            break;
        case WIDGET_CORNER_TOP_RIGHT:
            x = area.x + area.width - windowWidth - margin;
            y = area.y + margin;
            name = "top-right";
            break;
        case WIDGET_CORNER_TOP_LEFT:
            x = area.x + margin;
            y = area.y + margin;
            name = "top-left";
            break;
        case WIDGET_CORNER_BOTTOM_RIGHT:
            x = area.x + area.width - windowWidth - margin;
            y = area.y + area.height - windowHeight - margin;
            name = "bottom-right";
            break;
        default:
            TraceLog(LOG_WARNING, "WIDGET: unknown corner %d, using bottom-right", (int)corner);
            x = area.x + area.width - windowWidth - margin;
            y = area.y + area.height - windowHeight - margin;
            name = "bottom-right";
            break;
    }

    SetWindowPosition(x, y);
    TraceLog(LOG_INFO, "WIDGET: anchored %s at %d,%d on monitor %d (%dx%d area, margin %d)",
             name, x, y, index, area.width, area.height, margin);
}

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
#include <bits/time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <raylib.h>
#include <stdint.h>

typedef unsigned long XID;
typedef XID Atom;
typedef XID Window;
typedef struct _XDisplay Display;

extern Display *XOpenDisplay(const char *);
extern int XCloseDisplay(Display *);
extern Atom XInternAtom(Display *, const char *, int);
extern int XChangeProperty(Display *, Window, Atom, Atom, int, int, const unsigned char *, int);
extern Window XRootWindow(Display *, int);
extern int XSendEvent(Display *, Window, int, long, void *);
extern int XFlush(Display *);

typedef struct {
    int type;
    unsigned long serial;
    int send_event;
    Display *display;
    Window window;
    Atom message_type;
    int format;
    long data[5];
} XClientMessageEventCustom;

void ForceAlwaysOnTopLinux(void) {
    Display *display = XOpenDisplay(NULL);
    if (!display) return;

    // В Raylib GetWindowHandle() на X11 возвращает структуру,
    // где первым полем идет Native X11 Window Handle
    void *ptr = GetWindowHandle();
    if (!ptr) {
        XCloseDisplay(display);
        return;
    }

    Window window = *(Window *)ptr;
    if (!window) {
        XCloseDisplay(display);
        return;
    }

    Atom wmState = XInternAtom(display, "_NET_WM_STATE", 0);
    Atom wmStateAbove = XInternAtom(display, "_NET_WM_STATE_ABOVE", 0);
    Atom wmStateStaysOnTop = XInternAtom(display, "_NET_WM_STATE_STAYS_ON_TOP", 0);
    Atom wmWindowType = XInternAtom(display, "_NET_WM_WINDOW_TYPE", 0);
    Atom wmTypeDock = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", 0);

    XChangeProperty(display, window, wmWindowType, 4, 32, 0, (const unsigned char *)&wmTypeDock, 1);

    XClientMessageEventCustom xev = {0};
    xev.type = 33;
    xev.window = window;
    xev.message_type = wmState;
    xev.format = 32;
    xev.data[0] = 1;
    xev.data[1] = wmStateAbove;
    xev.data[2] = wmStateStaysOnTop;
    xev.data[3] = 1;

    XSendEvent(display, XRootWindow(display, 0), 0, 0x00020000L | 0x00080000L, &xev);

    XFlush(display);
    XCloseDisplay(display);
}
#define FRAME_COUNT 222
int main(void) {
 SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_TOPMOST);
  InitWindow(800, 800, "Hatsune Miku");
  ForceAlwaysOnTopLinux();
  void* display = GetWindowHandle();
  SetExitKey(KEY_NULL);
  SetWindowPosition(1120, 433);
  SetTargetFPS(100);
  Texture2D frames[FRAME_COUNT];
  char fileName[32];
  for (int i = 0; i < FRAME_COUNT; i++) {
    snprintf(fileName, sizeof(fileName), "assets/frame_%03d.png", i + 1);
    
    Image img = LoadImage(fileName);
    Color *pixels = LoadImageColors(img);
    int totalPixels = img.width * img.height;

    for (int j = 0; j < totalPixels; j++) {
        if (pixels[j].r < 35 && pixels[j].g < 35 && pixels[j].b < 35) {
            pixels[j].a = 0;
        }
    }
    Image customImg = {
        .data = pixels,
        .width = img.width,
        .height = img.height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    frames[i] = LoadTextureFromImage(customImg);

    UnloadImageColors(pixels);
    UnloadImage(img);
}
  int currentFrame = 0;
  int frameCounter = 0;
while(!WindowShouldClose()){
  BeginDrawing();
  frameCounter++;
  if (frameCounter >= 3) {
currentFrame = (currentFrame +1) % FRAME_COUNT;
frameCounter = 0;
  }
  ClearBackground(BLANK);
  DrawTexture(frames[currentFrame], 100, -340,WHITE);
  EndDrawing();

 }
  CloseWindow();
  return 0;
}
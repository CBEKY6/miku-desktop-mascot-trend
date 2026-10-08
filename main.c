#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <raylib.h>

#include "widget.h"

#define FRAME_COUNT 222

// The frames are 720x1280. The original code drew them at (100, -340) in an 800x800 window,
// which showed texture rows 340..1139 and columns 0..699, i.e. a 700x800 cut out. Sizing the
// window to that cut out means the window is exactly the mascot, so it can be anchored to a
// screen corner without empty space hanging off the edge.
#define WINDOW_WIDTH   700
#define WINDOW_HEIGHT  800
#define SPRITE_X       0
#define SPRITE_Y       340

// Pixels kept between the mascot and the corner of the screen.
#define SCREEN_MARGIN  24

static void PrintUsage(const char *program) {
    printf("Usage: %s [options]\n\n", program);
    printf("  --corner=CORNER    where to put the mascot: bottom-right (default),\n");
    printf("                     bottom-left, top-right, top-left\n");
    printf("  --monitor=N        monitor to appear on, 0 is the primary one (default)\n");
    printf("                     or -1 for the monitor under the mouse pointer\n");
    printf("  --margin=PIXELS    distance from the corner of the screen (default %d)\n",
           SCREEN_MARGIN);
    printf("  --help             show this text\n");
}

static bool ParseInt(const char *text, int *out) {
    char *end = NULL;
    long value = strtol(text, &end, 10);

    if (end == text || *end != '\0') return false;

    *out = (int)value;
    return true;
}

// Accepts both "--name=value" and "--name value". Returns false if `arg` is not this option at
// all; a recognised option with a missing value is reported and then skipped.
static bool ParseOption(int argc, char **argv, int *i, const char *name, bool numeric,
                        const char **value) {
    const char *arg = argv[*i];
    size_t length = strlen(name);

    if (strncmp(arg, name, length) != 0) return false;
    if (arg[length] != '\0' && arg[length] != '=') return false;

    if (arg[length] == '=') {
        *value = arg + length + 1;
    } else {
        int dummy = 0;
        // Guard against "--corner --margin=4" swallowing the next option as a value, but let a
        // negative number through for the numeric options.
        if (*i + 1 >= argc || argv[*i + 1][0] == '\0' ||
            (argv[*i + 1][0] == '-' && !(numeric && ParseInt(argv[*i + 1], &dummy)))) {
            TraceLog(LOG_WARNING, "MIKU: %s needs a value", name);
            return false;
        }
        *value = argv[++(*i)];
    }

    // "--name=" with nothing after it is a malformed value, not a missing one.
    if (**value == '\0') {
        TraceLog(LOG_WARNING, "MIKU: %s needs a value", name);
        return false;
    }

    return true;
}

int main(int argc, char **argv) {
  WidgetCorner corner = WIDGET_CORNER_BOTTOM_RIGHT;
  WidgetMonitor monitor = WIDGET_MONITOR_PRIMARY;
  int margin = SCREEN_MARGIN;
  const char *value = NULL;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      PrintUsage(argv[0]);
      return 0;
    } else if (ParseOption(argc, argv, &i, "--corner", false, &value)) {
      if (strcmp(value, "bottom-right") == 0) corner = WIDGET_CORNER_BOTTOM_RIGHT;
      else if (strcmp(value, "bottom-left") == 0) corner = WIDGET_CORNER_BOTTOM_LEFT;
      else if (strcmp(value, "top-right") == 0) corner = WIDGET_CORNER_TOP_RIGHT;
      else if (strcmp(value, "top-left") == 0) corner = WIDGET_CORNER_TOP_LEFT;
      else {
        TraceLog(LOG_WARNING, "MIKU: unknown corner '%s', using bottom-right", value);
        corner = WIDGET_CORNER_BOTTOM_RIGHT;
      }
    } else if (ParseOption(argc, argv, &i, "--monitor", true, &value)) {
      int parsed = 0;
      if (!ParseInt(value, &parsed) || parsed < -1) {
        TraceLog(LOG_WARNING, "MIKU: bad monitor '%s', using the primary one", value);
      } else {
        monitor = (WidgetMonitor)parsed;
      }
    } else if (ParseOption(argc, argv, &i, "--margin", false, &value)) {
      int parsed = 0;
      if (!ParseInt(value, &parsed) || parsed < 0) {
        TraceLog(LOG_WARNING, "MIKU: bad margin '%s', using %d", value, SCREEN_MARGIN);
      } else {
        margin = parsed;
      }
    } else {
      TraceLog(LOG_WARNING, "MIKU: unknown option '%s'", argv[i]);
      PrintUsage(argv[0]);
      return 1;
    }
  }

 SetConfigFlags(WidgetConfigFlags());
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hatsune Miku");
  WidgetMakeOverlay();
  SetExitKey(KEY_NULL);
  WidgetAnchor(WINDOW_WIDTH, WINDOW_HEIGHT, corner, monitor, margin);
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
  WidgetUpdate();
  BeginDrawing();
  frameCounter++;
  if (frameCounter >= 3) {
currentFrame = (currentFrame +1) % FRAME_COUNT;
frameCounter = 0;
  }
  ClearBackground(BLANK);
  Rectangle source = { SPRITE_X, SPRITE_Y, WINDOW_WIDTH, WINDOW_HEIGHT };
  Rectangle dest = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
  DrawTexturePro(frames[currentFrame], source, dest, (Vector2){ 0, 0 }, 0, WHITE);
  EndDrawing();

 }
  CloseWindow();
  return 0;
}
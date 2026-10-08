#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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

// Pixels kept between the mascot and the bottom-right corner of the screen.
#define SCREEN_MARGIN  24

int main(void) {
 SetConfigFlags(WidgetConfigFlags());
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hatsune Miku");
  WidgetMakeOverlay();
  SetExitKey(KEY_NULL);
  WidgetAnchorBottomRight(WINDOW_WIDTH, WINDOW_HEIGHT, SCREEN_MARGIN);
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
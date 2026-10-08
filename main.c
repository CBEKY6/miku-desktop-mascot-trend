#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <raylib.h>

#include "widget.h"

#define FRAME_COUNT 222
int main(void) {
 SetConfigFlags(WidgetConfigFlags());
  InitWindow(800, 800, "Hatsune Miku");
  WidgetMakeOverlay();
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
  WidgetUpdate();
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
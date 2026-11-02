#include <raylib.h>
#include <stdio.h>
#include <limits.h>

#define FRAME_COUNT 222
#define ANIM_FPS 24.0f

#define POS_X 1220
#define POS_Y 433

static Image LoadKeyedFrame(int i) {
    char fileName[64];
    snprintf(fileName, sizeof(fileName), "assets/frame_%03d.png", i + 1);

    Image img = LoadImage(fileName);
    if (img.data == NULL) return img;

    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    Color *pixels = (Color *)img.data;
    int totalPixels = img.width * img.height;

    for (int j = 0; j < totalPixels; j++) {
        if (pixels[j].r < 35 && pixels[j].g < 35 && pixels[j].b < 35) {
            pixels[j] = BLANK;
        }
    }
    return img;
}

int main(void) {
    int minX = INT_MAX, minY = INT_MAX, maxX = -1, maxY = -1;

    for (int i = 0; i < FRAME_COUNT; i++) {
        Image img = LoadKeyedFrame(i);
        if (img.data == NULL) continue;

        Color *pixels = (Color *)img.data;
        for (int y = 0; y < img.height; y++) {
            for (int x = 0; x < img.width; x++) {
                if (pixels[y * img.width + x].a > 0) {
                    if (x < minX) minX = x;
                    if (x > maxX) maxX = x;
                    if (y < minY) minY = y;
                    if (y > maxY) maxY = y;
                }
            }
        }
        UnloadImage(img);
    }

    if (maxX < 0) { minX = 0; minY = 0; maxX = 799; maxY = 799; }

    int cropW = maxX - minX + 1;
    int cropH = maxY - minY + 1;
    TraceLog(LOG_INFO, "Crop: x=%d y=%d size=%dx%d", minX, minY, cropW, cropH);

    SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TRANSPARENT |
                   FLAG_WINDOW_TOPMOST | FLAG_WINDOW_MOUSE_PASSTHROUGH);
    InitWindow(cropW, cropH, "Hatsune Miku");
    SetExitKey(KEY_NULL);
    SetWindowPosition(POS_X, POS_Y);
    SetTargetFPS(30);

    Texture2D frames[FRAME_COUNT] = {0};

    for (int i = 0; i < FRAME_COUNT; i++) {
        Image img = LoadKeyedFrame(i);
        if (img.data == NULL) continue;

        ImageCrop(&img, (Rectangle){ (float)minX, (float)minY, (float)cropW, (float)cropH });
        frames[i] = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    while (!WindowShouldClose()) {
        int currentFrame = (int)(GetTime() * ANIM_FPS) % FRAME_COUNT;

        BeginDrawing();
            ClearBackground(BLANK);
            if (frames[currentFrame].id != 0) {
                DrawTexture(frames[currentFrame], 0, 0, WHITE);
            }
        EndDrawing();
    }

    for (int i = 0; i < FRAME_COUNT; i++) {
        if (frames[i].id != 0) UnloadTexture(frames[i]);
    }

    CloseWindow();
    return 0;
}
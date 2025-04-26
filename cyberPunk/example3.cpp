#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <utility>
#include <cmath>
#include <cstdlib>
#include <ctime>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define PI 3.14159265358979323846
#define SMOOTH 1

typedef struct {
    SDL_Renderer* gRenderer;
    SDL_Renderer* renderer;
    SDL_Texture* screenTexture;
    Uint32* pix;
    int pitch;
    int screenWidth;
    int screenHeight;
    int pitchPixels;
} LSD;


// Simple clamp replacement for pre-C++17 compilers
template <typename T>
T clamp(T value, T min, T max) {
    if (value < min) return min;
    else if (value > max) return max;
    else return value;
}


extern void pixel(LSD* self, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
extern Uint32 getPixelColor(LSD* self, int x, int y);

SDL_Color getColor(int i) {
    SDL_Color c = { 0, 0, 0, 255 };

    if (i <= 31)
        c.r = std::min(i * 4, 63);
    else if (i <= 63)
        c.r = std::min((63 - i) * 4, 63);
    else if (i <= 95)
        c.g = std::min((i - 64) * 4, 63);
    else if (i <= 127)
        c.g = std::min((127 - i) * 4, 63);
    else if (i <= 159)
        c.b = std::min((i - 128) * 4, 63);
    else if (i <= 191)
        c.b = std::min((191 - i) * 4, 63);
    else if (i <= 223)
        c.r = c.g = c.b = std::min((i - 192) * 4, 63);
    else
        c.r = c.g = c.b = std::min((255 - i) * 4, 63);

    // Scale to 255 range
    c.r = (c.r * 255) / 63;
    c.g = (c.g * 255) / 63;
    c.b = (c.b * 255) / 63;
    return c;
}

void compensate(LSD* self, unsigned xa, unsigned ya, unsigned x, unsigned y, unsigned xb, unsigned yb) {
    if (getPixelColor(self, x, y) != 0) return;

    int dist = std::abs((int)(xa - xb)) + std::abs((int)(ya - yb));
    Uint32 colorA = getPixelColor(self, xa, ya);
    Uint32 colorB = getPixelColor(self, xb, yb);

    Uint8 rA = (colorA >> 16) & 0xFF;
    Uint8 gA = (colorA >> 8) & 0xFF;
    Uint8 bA = colorA & 0xFF;
    Uint8 rB = (colorB >> 16) & 0xFF;
    Uint8 gB = (colorB >> 8) & 0xFF;
    Uint8 bB = colorB & 0xFF;

    int r = (rA + rB) / 2 + (rand() % 2) * dist / SMOOTH;
    int g = (gA + gB) / 2 + (rand() % 2) * dist / SMOOTH;
    int b = (bA + bB) / 2 + (rand() % 2) * dist / SMOOTH;

    r = clamp(r, 0, 255);
    g = clamp(g, 0, 255);
    b = clamp(b, 0, 255);


    pixel(self, x, y, r, g, b, 255);
}

void plasma_init(LSD* self, unsigned x1, unsigned y1, unsigned x2, unsigned y2) {
    if ((x2 - x1 < 2) && (y2 - y1 < 2)) return;

    unsigned x = (x1 + x2) / 2;
    unsigned y = (y1 + y2) / 2;

    compensate(self, x1, y1, x1, y, x1, y2);
    compensate(self, x1, y1, x, y1, x2, y1);
    compensate(self, x1, y2, x, y2, x2, y2);
    compensate(self, x2, y1, x2, y, x2, y2);

    if (getPixelColor(self, x, y) == 0) {
        Uint32 colorTL = getPixelColor(self, x1, y1);
        Uint32 colorTR = getPixelColor(self, x2, y1);
        Uint32 colorBR = getPixelColor(self, x2, y2);
        Uint32 colorBL = getPixelColor(self, x1, y2);

        Uint8 r = ((colorTL >> 16) & 0xFF + (colorTR >> 16) & 0xFF + (colorBR >> 16) & 0xFF + (colorBL >> 16) & 0xFF) / 4;
        Uint8 g = ((colorTL >> 8) & 0xFF + (colorTR >> 8) & 0xFF + (colorBR >> 8) & 0xFF + (colorBL >> 8) & 0xFF) / 4;
        Uint8 b = ((colorTL) & 0xFF + (colorTR) & 0xFF + (colorBR) & 0xFF + (colorBL) & 0xFF) / 4;

        pixel(self, x, y, r, g, b, 255);
    }

    plasma_init(self, x1, y1, x, y);
    plasma_init(self, x, y1, x2, y);
    plasma_init(self, x, y, x2, y2);
    plasma_init(self, x1, y, x, y2);
}

static Uint8 plasma_indices[SCREEN_HEIGHT][SCREEN_WIDTH];

void plasma2ini(LSD* iii) {
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            float value = (
                128.0f + (128.0f * sinf(x / 40.0f)) +
                128.0f + (128.0f * sinf(y / 30.0f)) +
                128.0f + (128.0f * sinf((x + y) / 20.0f)) +
                128.0f + (128.0f * sinf(sqrtf(x * x + y * y) / 25.0f))
                ) / 4.0f;

            plasma_indices[y][x] = (Uint8)((int)value % 256);
        }
    }
}

void plasma2(LSD* iii, int offset) {
    void* pixels = nullptr;
    int pitch = 0;

    if (SDL_LockTexture(iii->screenTexture, NULL, &pixels, &pitch) != 0) {
        std::cerr << "SDL_LockTexture Error: " << SDL_GetError() << std::endl;
        return;
    }

    iii->pix = (Uint32*)pixels;
    iii->pitch = pitch;
    iii->pitchPixels = pitch / sizeof(Uint32);

    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            int index = (plasma_indices[y][x] + offset) % 256;
            SDL_Color c = getColor(index);
            pixel(iii, x, y, c.r, c.g, c.b, 255);
        }
    }

    SDL_UnlockTexture(iii->screenTexture);
}

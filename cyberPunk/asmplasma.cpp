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


 
const int WIDTH = 1920;// 320;
const int HEIGHT = 1080; // 200;

 
uint8_t cosine[256];
uint8_t palette[128][3];
uint8_t buffer[HEIGHT][WIDTH];

uint8_t Co1 = 0, Co2 = 0, Co3 = 0, Co4 = 0;

void initCosine2() {
    for (int i = 0; i < 256; ++i) {
        cosine[i] = static_cast<uint8_t>(std::lround(64 + 63 * std::cos(i * M_PI / 128.0)));
    }
}

void initCosine() {
    for (int i = 0; i < 256; ++i) {
        // Generate values using a sine wave with an offset for better visual range
        cosine[i] = static_cast<uint8_t>(std::lround(30 + 32 * std::sin(i * M_PI / 129.0)));
    }
}

void initPalette2() {
    int i = 0;
    for (int j = 0; j < 32; ++j) palette[i][0] = 33, palette[i][1] = j, palette[i++][2] = j;
    for (int j = 31; j >= 0; --j) palette[i][0] = j, palette[i][1] = j, palette[i++][2] = 33;
    for (int j = 0; j < 32; ++j) palette[i][0] = j, palette[i][1] = j, palette[i++][2] = 33;
    for (int j = 31; j >= 0; --j) palette[i][0] = 33, palette[i][1] = j, palette[i++][2] = j;
}

void initPalette() {
    int i = 0;
    // Create a gradient that smoothly transitions between colors
    for (int j = 0; j < 64; ++j) {
        // Transition from blue to green
        palette[i][0] = 0;
        palette[i][1] = j * 4;  // Green increases
        palette[i][2] = 255 - (j * 4);  // Blue decreases
        ++i;
    }
    for (int j = 0; j < 64; ++j) {
        // Transition from green to red
        palette[i][0] = j * 4;  // Red increases
        palette[i][1] = 255 - (j * 4);  // Green decreases
        palette[i][2] = 0;
        ++i;
    }
}

void doPlasma() {
    uint8_t cl = Co1;
    uint8_t ch = Co2;

    for (int y = 0; y < HEIGHT; ++y) {
        uint8_t dl = Co3;
        uint8_t dh = Co4;

        for (int x = 0; x < WIDTH; ++x) {
            uint8_t v = 50;
            v += cosine[dl];
            v += cosine[dh];
            v += cosine[cl];
            v += cosine[ch];
            buffer[y][x] = v & 0x7F;

            dl += 1;
            dh += 2;
        }

        cl += 3;
        ch += 4;
    }

    Co1 -= 4;
    Co2 += 3;
    Co3 -= 2;
    Co4 += 1;
}

uint8_t noise(uint8_t baseValue) {
    int offset = rand() % 8 - 4;  // Random value between -4 and +4
    return clamp(baseValue + offset, 0, 255);
}

void cyclePalette() {
    for (int i = 0; i < 128; ++i) {
        palette[i][0] = palette[i][0] + 1;
        palette[i][1] = palette[i][1] + 1;
        palette[i][2] = palette[i][2] + 1;
        // Relying on uint8_t overflow for modulo 256 behavior
    }
}

 

int asmini() {

    initCosine();
    initPalette();

    //texture = SDL_CreateTexture(//renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);



    return 0;
}
 
void asmgo(LSD* iii) {

    cyclePalette();
    doPlasma();

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            uint8_t idx = buffer[y][x];
            Uint8 r = palette[idx][0] * 2;
            Uint8 g = palette[idx][1] * 2;
            Uint8 b = palette[idx][2] * 2;
            Uint8 a = 255; // Set alpha if needed, or remove this line if not necessary

      
            
            pixel(iii, x, y, r, g, b, a);
        }
    }
 

 
}

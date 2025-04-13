#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <SDL2/SDL.h>

 

 
//int X = 1920, Y = 1080;
//#define PLASMA_WIDTH 960
//#define PLASMA_HEIGHT 540
#define PLASMA_WIDTH 1920
#define PLASMA_HEIGHT 1080
#define PI 3.14159265358979323846264338327950288419716939937510582097494459


typedef struct {
    SDL_Renderer* gRenderer;
    SDL_Renderer* renderer; // Renderer for drawing
    SDL_Texture* screenTexture;
    Uint32* pix;
    int pitch;
    int screenWidth;
    int screenHeight;
    int pitchPixels;  // New field to store the pitch divided by 4
} LSD;

extern void pixel(LSD* self, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

 

typedef struct {
    unsigned char* body;      // Plasma body buffer (960x540)
    Uint8 palette[256+1][3];    // Static palette for DX7-like colors
    int cosinus[256+1];         // Cosine table
    int width;
    int height;
} Plasma;

 Plasma plasma;

void initPlasma(Plasma* plasma, int x, int y);
void updatePlasma(Plasma* plasma, float dt);
void drawPlasma(Plasma* plasma, LSD* gfx);
void destroyPlasma(Plasma* plasma);


void initPlasma(Plasma* plasma, int x, int y) {
    plasma->width = PLASMA_WIDTH;
    plasma->height = PLASMA_HEIGHT;
    plasma->body = (unsigned char*)malloc(PLASMA_WIDTH * PLASMA_HEIGHT);
    if (!plasma->body) {
        printf("Failed to allocate plasma body\n");
        return;
    }
    memset(plasma->body, 0, PLASMA_WIDTH * PLASMA_HEIGHT);

    // Initialize cosine table  
    for (int i = 0; i < 256; i++) {
        plasma->cosinus[i] = (int)(30 * cos(i * (PI / 128)));
    }

    // Initialize static palette  
    for (int i = 0; i < 256; i++) {
        plasma->palette[i][0] = 0;
        plasma->palette[i][1] = 0;
        plasma->palette[i][2] = 0;
    }
    for (int x = 1; x <= 32; x++) {
        // Blue ramp
        plasma->palette[x][0] = 0;
        plasma->palette[x][1] = 0;
        plasma->palette[x][2] = (x * 2 - 1) * 4;
        // Red to blue
        plasma->palette[x + 32][0] = (x * 2 - 1) * 4;
        plasma->palette[x + 32][1] = 0;
        plasma->palette[x + 32][2] = 255;
        // Red + blue to white
        plasma->palette[x + 64][0] = 255;
        plasma->palette[x + 64][1] = (x * 2 - 1) * 4;
        plasma->palette[x + 64][2] = 255;
        // White
        plasma->palette[x + 96][0] = 255;
        plasma->palette[x + 96][1] = 255;
        plasma->palette[x + 96][2] = 255;
        // White
        plasma->palette[x + 128][0] = 255;
        plasma->palette[x + 128][1] = 255;
        plasma->palette[x + 128][2] = 255;
        // White to yellow
        plasma->palette[x + 160][0] = 255;
        plasma->palette[x + 160][1] = 255;
        plasma->palette[x + 160][2] = (63 - (x * 2 - 1)) * 4;
        // Yellow to red
        plasma->palette[x + 192][0] = 255;
        plasma->palette[x + 192][1] = (63 - (x * 2 - 1)) * 4;
        plasma->palette[x + 192][2] = 0;
        // Red to black
        plasma->palette[x + 224][0] = (63 - (x * 2 - 1)) * 4;
        plasma->palette[x + 224][1] = 0;
        plasma->palette[x + 224][2] = 0;
    }

    srand((unsigned int)time(NULL));
   // printf("Initialized plasma at (%d, %d)\n", x, y);
}

void destroyPlasma(Plasma* plasma) {
    if (plasma->body) {
        free(plasma->body);
        plasma->body = NULL;
    }
}

void updatePlasma(Plasma* plasma, float dt) {
    static unsigned char p1 = 0, p2 = 0, p3 = 0, p4 = 0;
    unsigned char t1, t2, t3, t4;
    int col;

    // Compute plasma body (DX7 Do_Plasma)
    t1 = p1;
    t2 = p2;
    for (int y = 0; y < PLASMA_HEIGHT; y++) {
        t3 = p3;
        t4 = p4;
        for (int x = 0; x < PLASMA_WIDTH; x++) {
            col = plasma->cosinus[t1] + plasma->cosinus[t2] + plasma->cosinus[t3] + plasma->cosinus[t4];
            plasma->body[y * PLASMA_WIDTH + x] = (unsigned char)col; // Natural wrapping
            t3 = (t3 + 1) % 256;
            t4 = (t4 + 3) % 256;
        }
        t1 = (t1 + 2) % 256;
        t2 = (t2 + 1) % 256;
    }

    // Update animation offsets
    p1 = (p1 + 1) % 256;
    p2 = (p2 - 2) % 256;
    p3 = (p3 - 1) % 256;
    p4 = (p4 + 3) % 256;
}

// Wrap macro for clean coordinate wrapping
#define WRAP(i, max) (((i) + (max)) % (max))

void drawPlasma(Plasma* plasma, LSD* gfx) {
    int w = gfx->screenWidth;
    int h = gfx->screenHeight;
    int pw = PLASMA_WIDTH;
    int ph = PLASMA_HEIGHT;

    for (int y = 0; y < h; y++) {
        float v = (float)y / h * ph;
        int iy = (int)v;
        float fy = v - iy;

        for (int x = 0; x < w; x++) {
            float u = (float)x / w * pw;
            int ix = (int)u;
            float fx = u - ix;

            // Wrap coordinates instead of clamping
            int i00 = WRAP(iy, ph) * pw + WRAP(ix, pw);
            int i10 = WRAP(iy, ph) * pw + WRAP(ix + 1, pw);
            int i01 = WRAP(iy + 1, ph) * pw + WRAP(ix, pw);
            int i11 = WRAP(iy + 1, ph) * pw + WRAP(ix + 1, pw);

            int c00 = plasma->body[i00];
            int c10 = plasma->body[i10];
            int c01 = plasma->body[i01];
            int c11 = plasma->body[i11];

            float w00 = (1.0f - fx) * (1.0f - fy);
            float w10 = fx * (1.0f - fy);
            float w01 = (1.0f - fx) * fy;
            float w11 = fx * fy;

            float r = plasma->palette[c00][0] * w00 +
                      plasma->palette[c10][0] * w10 +
                      plasma->palette[c01][0] * w01 +
                      plasma->palette[c11][0] * w11;

            float g = plasma->palette[c00][1] * w00 +
                      plasma->palette[c10][1] * w10 +
                      plasma->palette[c01][1] * w01 +
                      plasma->palette[c11][1] * w11;

            float b = plasma->palette[c00][2] * w00 +
                      plasma->palette[c10][2] * w10 +
                      plasma->palette[c01][2] * w01 +
                      plasma->palette[c11][2] * w11;

            pixel(gfx, x, y, (Uint8)(r + 0.5f), (Uint8)(g + 0.5f), (Uint8)(b + 0.5f), 255);
        }
    }
}





void ini2(int x, int y)
{ //ini2(int x, int y)

 
    initPlasma(&plasma, 1920, 1080);

}

void example2(LSD* gfx, float dt) {
    updatePlasma(&plasma, dt);
    drawPlasma(&plasma, gfx);
}
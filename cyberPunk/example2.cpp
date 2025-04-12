#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
 

#include <SDL.h>

#include <math.h>

#define PLASMA_WIDTH 960
#define PLASMA_HEIGHT 540
#define TABLE_SIZE (PLASMA_WIDTH * PLASMA_HEIGHT)

#define PI 3.1415926f

 

typedef struct {
    unsigned char* body;      // Plasma body buffer (960x540)
    unsigned char* tab1;      // Distance table
    unsigned char* tab2;      // Sine-modulated table
    Uint8 palette[256][3];    // Color palette for DOS-like banding
    float r_phase, g_phase, b_phase; // Color cycling phases
    int width;
    int height;
} Plasma;


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



void initPlasma(Plasma* plasma, int x, int y);
void updatePlasma(Plasma* plasma, float dt);
void drawPlasma(Plasma* plasma, LSD* gfx);
void destroyPlasma(Plasma* plasma);



void initPlasma(Plasma* plasma, int x, int y) {
    plasma->width = PLASMA_WIDTH;
    plasma->height = PLASMA_HEIGHT;
    plasma->body = (unsigned char*)malloc(PLASMA_WIDTH * PLASMA_HEIGHT);
    plasma->tab1 = (unsigned char*)malloc(TABLE_SIZE);
    plasma->tab2 = (unsigned char*)malloc(TABLE_SIZE);
    if (!plasma->body || !plasma->tab1 || !plasma->tab2) {
        printf("Failed to allocate plasma buffers\n");
        return;
    }
    memset(plasma->body, 0, PLASMA_WIDTH * PLASMA_HEIGHT);
    memset(plasma->tab1, 0, TABLE_SIZE);
    memset(plasma->tab2, 0, TABLE_SIZE);

    // Calculate tab1: DOS formula
    for (int i = 0; i < PLASMA_HEIGHT; i++) {
        for (int j = 0; j < PLASMA_WIDTH; j++) {
            int idx = i * PLASMA_WIDTH + j;
            float dist = sqrtf(16.0f + (PLASMA_HEIGHT / 2 - i) * (PLASMA_HEIGHT / 2 - i) +
                (PLASMA_WIDTH / 2 - j) * (PLASMA_WIDTH / 2 - j)) - 4;
            plasma->tab1[idx] = (unsigned char)(dist * 5);
        }
    }

    // Calculate tab2: sine-modulated distance
    for (int i = 0; i < PLASMA_HEIGHT; i++) {
        for (int j = 0; j < PLASMA_WIDTH; j++) {
            int idx = i * PLASMA_WIDTH + j;
            float dist = sqrtf(16.0f + (PLASMA_HEIGHT / 2 - i) * (PLASMA_HEIGHT / 2 - i) +
                (PLASMA_WIDTH / 2 - j) * (PLASMA_WIDTH / 2 - j)) - 4;
            plasma->tab2[idx] = (unsigned char)((sinf(dist / 9.5f) + 1) * 90);
        }
    }

    // Initialize palette
    for (int i = 0; i < 256; i++) {
        float u = 2 * PI / 256 * i;
        plasma->palette[i][0] = (cosf(u + 1.0f / 6.0f * PI) + 1) * 31;
        plasma->palette[i][1] = (cosf(u + 3.0f / 6.0f * PI) + 1) * 31;
        plasma->palette[i][2] = (cosf(u + 5.0f / 6.0f * PI) + 1) * 31;
    }

    plasma->r_phase = 1.0f / 6.0f * PI;
    plasma->g_phase = 3.0f / 6.0f * PI;
    plasma->b_phase = 5.0f / 6.0f * PI;

    srand((unsigned int)time(NULL));
    printf("Initialized plasma at (%d, %d)\n", x, y);
}

void destroyPlasma(Plasma* plasma) {
    if (plasma->body) {
        free(plasma->body);
        plasma->body = NULL;
    }
    if (plasma->tab1) {
        free(plasma->tab1);
        plasma->tab1 = NULL;
    }
    if (plasma->tab2) {
        free(plasma->tab2);
        plasma->tab2 = NULL;
    }
}

void updatePlasma(Plasma* plasma, float dt) {
    static float circle1 = 0, circle2 = 0, circle3 = 0, circle4 = 0;
    static float circle5 = 0, circle6 = 0, circle7 = 0, circle8 = 0;
    static float roll = 0;

    // Update circle offsets (DOS speeds)
    circle1 += 0.0142f * dt * 60.0f; // 0.085/6
    circle2 -= 0.0167f * dt * 60.0f; // 0.1/6
    circle3 += 0.05f * dt * 60.0f;   // 0.3/6
    circle4 -= 0.0333f * dt * 60.0f; // 0.2/6
    circle5 += 0.0667f * dt * 60.0f; // 0.4/6
    circle6 -= 0.025f * dt * 60.0f;  // 0.15/6
    circle7 += 0.0583f * dt * 60.0f; // 0.35/6
    circle8 -= 0.0083f * dt * 60.0f; // 0.05/6
    roll += 5.0f * dt * 60.0f;       // 5/frame

    // Update palette (DOS-style multi-phase)
    for (int i = 0; i < 256; i++) {
        float u = 2 * PI / 256 * i;
        plasma->palette[i][0] = (cosf(u + plasma->r_phase) + 1) * 31;
        plasma->palette[i][1] = (cosf(u + plasma->g_phase) + 1) * 31;
        plasma->palette[i][2] = (cosf(u + plasma->b_phase) + 1) * 31;
    }

    // Calculate offsets
    int x1 = (PLASMA_WIDTH / 2) + (PLASMA_WIDTH / 2) * cosf(circle3);
    int y1 = (PLASMA_HEIGHT / 2) + (PLASMA_HEIGHT / 2) * sinf(circle4);
    int x2 = (PLASMA_WIDTH / 2) + (PLASMA_WIDTH / 2) * sinf(circle1);
    int y2 = (PLASMA_HEIGHT / 2) + (PLASMA_HEIGHT / 2) * cosf(circle2);
    int x3 = (PLASMA_WIDTH / 2) + (PLASMA_WIDTH / 2) * cosf(circle5);
    int y3 = (PLASMA_HEIGHT / 2) + (PLASMA_HEIGHT / 2) * sinf(circle6);
    int x4 = (PLASMA_WIDTH / 2) + (PLASMA_WIDTH / 2) * cosf(circle7);
    int y4 = (PLASMA_HEIGHT / 2) + (PLASMA_HEIGHT / 2) * sinf(circle8);

    // Calculate plasma body
    for (int i = 0; i < PLASMA_HEIGHT; i++) {
        for (int j = 0; j < PLASMA_WIDTH; j++) {
            int k = i * PLASMA_WIDTH + j;
            int idx1 = ((i + y1) % PLASMA_HEIGHT) * PLASMA_WIDTH + ((j + x1) % PLASMA_WIDTH);
            int idx2 = ((i + y2) % PLASMA_HEIGHT) * PLASMA_WIDTH + ((j + x2) % PLASMA_WIDTH);
            int idx3 = ((i + y3) % PLASMA_HEIGHT) * PLASMA_WIDTH + ((j + x3) % PLASMA_WIDTH);
            int idx4 = ((i + y4) % PLASMA_HEIGHT) * PLASMA_WIDTH + ((j + x4) % PLASMA_WIDTH);
            float sum = plasma->tab1[idx1] + roll +
                plasma->tab2[idx2] +
                plasma->tab2[idx3] +
                plasma->tab2[idx4];
            plasma->body[k] = (unsigned char)(sum * 0.25f);
        }
    }

    // Update color phases (DOS speeds)
    plasma->r_phase += 0.05f * dt * 60.0f;  // ~3.0 radians/s
    plasma->g_phase -= 0.05f * dt * 60.0f;
    plasma->b_phase += 0.1f * dt * 60.0f;
}

void drawPlasma(Plasma* plasma, LSD* gfx) {
    // Stretch 960x540 body to 1920x1080
    for (int y = 0; y < PLASMA_HEIGHT; y++) {
        for (int x = 0; x < PLASMA_WIDTH; x++) {
            int k = y * PLASMA_WIDTH + x;
            unsigned char value = plasma->body[k];
            Uint8 r = plasma->palette[value][0];
            Uint8 g = plasma->palette[value][1];
            Uint8 b = plasma->palette[value][2];
            pixel(gfx, x * 2, y * 2, r, g, b, 255);
            pixel(gfx, x * 2 + 1, y * 2, r, g, b, 255);
            pixel(gfx, x * 2, y * 2 + 1, r, g, b, 255);
            pixel(gfx, x * 2 + 1, y * 2 + 1, r, g, b, 255);
        }
    }
}



Plasma plasma;;

void ini2(int x, int y)
{ //ini2(int x, int y)

    initPlasma(&plasma, 960, 540);

}

void example2(LSD* gfx, float dt) {
    updatePlasma(&plasma, dt);
    drawPlasma(&plasma, gfx);
}
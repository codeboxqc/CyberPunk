

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
 
 
#include <SDL2/SDL.h>

 

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

 


#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

typedef struct {
    unsigned char* buffer;     // Main fire intensity buffer
    unsigned char* fire_buffer; // Temporary buffer for smoothing
    int width;
    int height;
} Flame;

extern Flame flame;

void initFlame(Flame* flame, float x, float y);
void updateFlame(Flame* flame, float dt);
void drawFlame(Flame* flame, LSD* gfx);
void destroyFlame(Flame* flame);

void initFlame(Flame* flame, float x, float y) {
    flame->width = SCREEN_WIDTH;
    flame->height = SCREEN_HEIGHT;
    flame->buffer = (unsigned char*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT);
    flame->fire_buffer = (unsigned char*)malloc(SCREEN_WIDTH * SCREEN_HEIGHT);
    if (!flame->buffer || !flame->fire_buffer) {
        printf("Failed to allocate fire buffers\n");
        return;
    }
    memset(flame->buffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
    memset(flame->fire_buffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
    srand((unsigned int)time(NULL));
    printf("Initialized flame at (%f, %f)\n", x, y);
}

void destroyFlame(Flame* flame) {
    if (flame->buffer) {
        free(flame->buffer);
        flame->buffer = NULL;
    }
    if (flame->fire_buffer) {
        free(flame->fire_buffer);
        flame->fire_buffer = NULL;
    }
}

void updateFlame(Flame* flame, float dt) {
    // Place denser coals over a thicker band
    for (int x = 1; x < SCREEN_WIDTH - 5; x += rand() % 10) { // Tighter spacing
        if (rand() % 3 == 0) { // Higher chance
            for (int dx = 0; dx < 5; dx++) {
                for (int dy = 1; dy <= 30; dy++) { // y = 1050–1079
                    int y = SCREEN_HEIGHT - dy;
                    flame->buffer[y * SCREEN_WIDTH + x + dx] = (dy <= 10) ? 255 : (dy <= 20) ? 254 : 253;
                }
            }
        }
    }

    // Smooth fire: average neighbors, shift up faster, fade slower
    for (int x = 1; x < SCREEN_WIDTH - 1; x++) {
        for (int y = 10; y < SCREEN_HEIGHT - 1; y++) { // Extend to y = 10
            int idx = y * SCREEN_WIDTH + x;
            int avg = (
                flame->buffer[idx - 1] +              // x-1, y
                flame->buffer[idx - SCREEN_WIDTH] +   // x, y-1
                flame->buffer[idx + SCREEN_WIDTH] +   // x, y+1
                flame->buffer[idx + 1]                // x+1, y
                ) >> 2;
            int new_y = y - 2; // Faster shift (2 pixels up)
            if (new_y >= 0) {
                int new_idx = new_y * SCREEN_WIDTH + x;
                flame->fire_buffer[new_idx] = avg > 10 ? avg - 1 : avg; // Slower fade
            }
        }
    }

    // Copy fire_buffer to buffer
    memcpy(flame->buffer, flame->fire_buffer, SCREEN_WIDTH * SCREEN_HEIGHT);
    memset(flame->fire_buffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
}

void drawFlame(Flame* flame, LSD* gfx) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int idx = y * SCREEN_WIDTH + x;
            unsigned char value = flame->buffer[idx];
            Uint8 r, g, b;
            if (value == 0) {
                r = g = b = 0; // Black
            }
            else if (value <= 63) {
                r = value * 4; g = 0; b = 0; // Red
            }
            else if (value <= 127) {
                r = 255; g = (value - 64) * 2; b = 0; // Orange
            }
            else if (value <= 191) {
                r = 255; g = 128 + (value - 128); b = value - 128; // Yellow
            }
            else {
                r = g = b = 255; // White
            }
            pixel(gfx, x, y, r, g, b, 255);
        }
    }
}


Flame flame;
void ini1(int x,int y) {

 
    initFlame(&flame, x, y); // example1
}

 
 

void example1(int x, int y, LSD* gfx, float dt) {
     


     
 
        // Update flame
        updateFlame(&flame, dt);

       

        // Draw flame
        drawFlame(&flame, gfx);
     

        // Draw torch base with pixels
        for (int py = y; py < y + 20; py++) {
            for (int px = x - 10; px < x + 10; px++) {
                pixel(gfx, px, py, 100, 100, 100, 255);
            }
        }
    

 
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


#define PI 3.141592653589793238462643383279502884197169399375105820974944592
 
#include <SDL2/SDL_ttf.h>
#include <math.h>

#define FLUID_WIDTH 960
#define FLUID_HEIGHT 540




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
    float* body; // RGBA (r,g,b,a) per pixel
    float* u;
    float* v;
    Uint8 palette[256][3]; // For text only
    TTF_Font* font;
    SDL_Surface* text_surface;
    int width;
    int height;
    float mouse_x, mouse_y;
    float mouse_dx, mouse_dy;
    int mouse_moved;
} Fluid;

 Fluid fluid;

void initFluid(Fluid* fluid, int x, int y);
void updateFluid(Fluid* fluid, float dt);
void drawFluid(Fluid* fluid, LSD* gfx);
void destroyFluid(Fluid* fluid);


 
 
void initFluid(Fluid* fluid, int x, int y) {
    fluid->width = FLUID_WIDTH;
    fluid->height = FLUID_HEIGHT;
    fluid->body = (float*)malloc(FLUID_WIDTH * FLUID_HEIGHT * 4 * sizeof(float)); // RGBA
    fluid->u = (float*)malloc(FLUID_WIDTH * FLUID_HEIGHT * sizeof(float));
    fluid->v = (float*)malloc(FLUID_WIDTH * FLUID_HEIGHT * sizeof(float));
    if (!fluid->body || !fluid->u || !fluid->v) {
        printf("Failed to allocate fluid buffers\n");
        fluid->body = NULL;
        fluid->u = NULL;
        fluid->v = NULL;
        return;
    }
    memset(fluid->body, 0, FLUID_WIDTH * FLUID_HEIGHT * 4 * sizeof(float));
    memset(fluid->u, 0, FLUID_WIDTH * FLUID_HEIGHT * sizeof(float));
    memset(fluid->v, 0, FLUID_WIDTH * FLUID_HEIGHT * sizeof(float));

    // Initialize palette for text
    for (int i = 0; i < 256; i++) {
        fluid->palette[i][0] = 0;
        fluid->palette[i][1] = 0;
        fluid->palette[i][2] = 0;
    }
    fluid->palette[255][0] = 255;
    fluid->palette[255][1] = 255;
    fluid->palette[255][2] = 255;

    // Initialize SDL_ttf
    if (TTF_Init() < 0) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        fluid->font = NULL;
        fluid->text_surface = NULL;
        return;
    }

    // Try multiple font paths
    const char* font_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:\\Windows\\Fonts\\verdana.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        NULL
    };
    fluid->font = NULL;
    for (int i = 0; font_paths[i]; i++) {
        fluid->font = TTF_OpenFont(font_paths[i], 80);
        if (fluid->font) break;
        printf("Tried font %s: %s\n", font_paths[i], TTF_GetError());
    }
    if (!fluid->font) {
        printf("Failed to load any font\n");
        TTF_Quit();
        fluid->text_surface = NULL;
        return;
    }

    // Render initial text
    SDL_Color white = { 255, 255, 255, 255 };
    fluid->text_surface = TTF_RenderText_Blended(fluid->font, "fluid", white);
    if (!fluid->text_surface) {
        printf("Failed to render text: %s\n", TTF_GetError());
        TTF_CloseFont(fluid->font);
        TTF_Quit();
        fluid->font = NULL;
        return;
    }

    // Initialize body with text (white, opaque)
    for (int y = 0; y < FLUID_HEIGHT; y++) {
        for (int x = 0; x < FLUID_WIDTH; x++) {
            int tx = x - (FLUID_WIDTH - fluid->text_surface->w) / 2;
            int ty = y - (FLUID_HEIGHT - fluid->text_surface->h) / 2;
            int i = (y * FLUID_WIDTH + x) * 4;
            if (tx >= 0 && tx < fluid->text_surface->w && ty >= 0 && ty < fluid->text_surface->h) {
                Uint8* pixel = (Uint8*)fluid->text_surface->pixels + ty * fluid->text_surface->pitch + tx * 4;
                if (pixel[3] > 0) {
                    fluid->body[i] = 255.0f;
                    fluid->body[i + 1] = 255.0f;
                    fluid->body[i + 2] = 255.0f;
                    fluid->body[i + 3] = 255.0f;
                }
            }
        }
    }

    fluid->mouse_x = fluid->mouse_y = 0;
    fluid->mouse_dx = fluid->mouse_dy = 0;
    fluid->mouse_moved = 0;

    srand((unsigned int)time(NULL));
    printf("Initialized fluid at (%d, %d)\n", x, y);
}

void destroyFluid(Fluid* fluid) {
    if (fluid->body) free(fluid->body);
    if (fluid->u) free(fluid->u);
    if (fluid->v) free(fluid->v);
    if (fluid->text_surface) SDL_FreeSurface(fluid->text_surface);
    if (fluid->font) TTF_CloseFont(fluid->font);
    TTF_Quit();
    fluid->body = NULL;
    fluid->u = NULL;
    fluid->v = NULL;
    fluid->text_surface = NULL;
    fluid->font = NULL;
}

void updateFluid(Fluid* fluid, float dt) {
    if (!fluid->body || !fluid->u || !fluid->v || !fluid->text_surface) return;

    // Splat velocity/color at mouse
    if (fluid->mouse_moved) {
        int mx = (int)(fluid->mouse_x * FLUID_WIDTH / 1920.0f);
        int my = (int)(fluid->mouse_y * FLUID_HEIGHT / 1080.0f);
        float radius = 30.0f;
        float speed = sqrtf(fluid->mouse_dx * fluid->mouse_dx + fluid->mouse_dy * fluid->mouse_dy);
        float alpha = fminf(100.0f + speed * 2.0f, 200.0f); // 100–200
        for (int y = my - (int)radius; y <= my + (int)radius; y++) {
            for (int x = mx - (int)radius; x <= mx + (int)radius; x++) {
                if (x >= 0 && x < FLUID_WIDTH && y >= 0 && y < FLUID_HEIGHT) {
                    float dx = x - mx;
                    float dy = y - my;
                    float dist = sqrtf(dx * dx + dy * dy);
                    if (dist < radius) {
                        float strength = 1.0f - dist / radius;
                        fluid->u[y * FLUID_WIDTH + x] += fluid->mouse_dx * strength * 0.5f;
                        fluid->v[y * FLUID_WIDTH + x] += fluid->mouse_dy * strength * 0.5f;
                        int i = (y * FLUID_WIDTH + x) * 4;
                        if (fluid->body[i + 3] < 255.0f) { // Skip text
                            fluid->body[i] = 255.0f;
                            fluid->body[i + 1] = 0.0f;
                            fluid->body[i + 2] = 0.0f;
                            fluid->body[i + 3] = alpha * strength;
                        }
                    }
                }
            }
        }
        fluid->mouse_moved = 0;
    }

    // Advect velocities
    float* new_u = (float*)malloc(FLUID_WIDTH * FLUID_HEIGHT * sizeof(float));
    float* new_v = (float*)malloc(FLUID_WIDTH * FLUID_HEIGHT * sizeof(float));
    if (!new_u || !new_v) {
        printf("Failed to allocate advection buffers\n");
        if (new_u) free(new_u);
        if (new_v) free(new_v);
        return;
    }
    memset(new_u, 0, FLUID_WIDTH * FLUID_HEIGHT * sizeof(float));
    memset(new_v, 0, FLUID_WIDTH * FLUID_HEIGHT * sizeof(float));

    for (int y = 0; y < FLUID_HEIGHT; y++) {
        for (int x = 0; x < FLUID_WIDTH; x++) {
            int i = y * FLUID_WIDTH + x;
            float vx = fluid->u[i];
            float vy = fluid->v[i];
            float px = x - vx * dt * 60.0f;
            float py = y - vy * dt * 60.0f;
            if (px < 0) px = 0;
            if (px > FLUID_WIDTH - 1) px = FLUID_WIDTH - 1;
            if (py < 0) py = 0;
            if (py > FLUID_HEIGHT - 1) py = FLUID_HEIGHT - 1;
            int x0 = (int)px;
            int y0 = (int)py;
            float s1 = px - x0;
            float t1 = py - y0;
            float s0 = 1.0f - s1;
            float t0 = 1.0f - t1;
            if (x0 + 1 < FLUID_WIDTH && y0 + 1 < FLUID_HEIGHT) {
                new_u[i] = s0 * (t0 * fluid->u[y0 * FLUID_WIDTH + x0] + t1 * fluid->u[(y0 + 1) * FLUID_WIDTH + x0]) +
                    s1 * (t0 * fluid->u[y0 * FLUID_WIDTH + (x0 + 1)] + t1 * fluid->u[(y0 + 1) * FLUID_WIDTH + (x0 + 1)]);
                new_v[i] = s0 * (t0 * fluid->v[y0 * FLUID_WIDTH + x0] + t1 * fluid->v[(y0 + 1) * FLUID_WIDTH + x0]) +
                    s1 * (t0 * fluid->v[y0 * FLUID_WIDTH + (x0 + 1)] + t1 * fluid->v[(y0 + 1) * FLUID_WIDTH + (x0 + 1)]);
            }
        }
    }
    memcpy(fluid->u, new_u, FLUID_WIDTH * FLUID_HEIGHT * sizeof(float));
    memcpy(fluid->v, new_v, FLUID_WIDTH * FLUID_HEIGHT * sizeof(float));
    free(new_u);
    free(new_v);

    // Advect colors
    float* new_body = (float*)malloc(FLUID_WIDTH * FLUID_HEIGHT * 4 * sizeof(float));
    if (!new_body) {
        printf("Failed to allocate body buffer\n");
        return;
    }
    memset(new_body, 0, FLUID_WIDTH * FLUID_HEIGHT * 4 * sizeof(float));
    for (int y = 0; y < FLUID_HEIGHT; y++) {
        for (int x = 0; x < FLUID_WIDTH; x++) {
            int i = (y * FLUID_WIDTH + x) * 4;
            float vx = fluid->u[y * FLUID_WIDTH + x];
            float vy = fluid->v[y * FLUID_WIDTH + x];
            float px = x - vx * dt * 60.0f;
            float py = y - vy * dt * 60.0f;
            if (px < 0) px = 0;
            if (px > FLUID_WIDTH - 1) px = FLUID_WIDTH - 1;
            if (py < 0) py = 0;
            if (py > FLUID_HEIGHT - 1) py = FLUID_HEIGHT - 1;
            int x0 = (int)px;
            int y0 = (int)py;
            float s1 = px - x0;
            float t1 = py - y0;
            float s0 = 1.0f - s1;
            float t0 = 1.0f - t1;
            if (x0 + 1 < FLUID_WIDTH && y0 + 1 < FLUID_HEIGHT) {
                int i00 = (y0 * FLUID_WIDTH + x0) * 4;
                int i10 = (y0 * FLUID_WIDTH + (x0 + 1)) * 4;
                int i01 = ((y0 + 1) * FLUID_WIDTH + x0) * 4;
                int i11 = ((y0 + 1) * FLUID_WIDTH + (x0 + 1)) * 4;
                for (int c = 0; c < 4; c++) {
                    float v00 = fluid->body[i00 + c];
                    float v10 = fluid->body[i10 + c];
                    float v01 = fluid->body[i01 + c];
                    float v11 = fluid->body[i11 + c];
                    new_body[i + c] = s0 * (t0 * v00 + t1 * v01) + s1 * (t0 * v10 + t1 * v11);
                }
            }
        }
    }
    memcpy(fluid->body, new_body, FLUID_WIDTH * FLUID_HEIGHT * 4 * sizeof(float));
    free(new_body);

    // Fade colors
    for (int i = 0; i < FLUID_WIDTH * FLUID_HEIGHT; i++) {
        int idx = i * 4;
        if (fluid->body[idx + 3] < 255.0f) { // Skip text
            fluid->body[idx + 3] *= 0.95f; // Fade alpha
            if (fluid->body[idx + 3] < 1.0f) {
                fluid->body[idx] = 0;
                fluid->body[idx + 1] = 0;
                fluid->body[idx + 2] = 0;
                fluid->body[idx + 3] = 0;
            }
        }
        fluid->u[i] *= 0.99f;
        fluid->v[i] *= 0.99f;
    }
}

void drawFluid(Fluid* fluid, LSD* gfx) {
    if (!fluid->body || !gfx->pix) return;
    for (int y = 0; y < FLUID_HEIGHT; y++) {
        for (int x = 0; x < FLUID_WIDTH; x++) {
            int i = (y * FLUID_WIDTH + x) * 4;
            Uint8 r = (Uint8)fminf(fluid->body[i], 255.0f);
            Uint8 g = (Uint8)fminf(fluid->body[i + 1], 255.0f);
            Uint8 b = (Uint8)fminf(fluid->body[i + 2], 255.0f);
            Uint8 a = (Uint8)fminf(fluid->body[i + 3], 255.0f);
            pixel(gfx, x * 2, y * 2, r, g, b, a);
            pixel(gfx, x * 2 + 1, y * 2, r, g, b, a);
            pixel(gfx, x * 2, y * 2 + 1, r, g, b, a);
            pixel(gfx, x * 2 + 1, y * 2 + 1, r, g, b, a);
        }
    }
}




void init3(int x, int y)
{

    initFluid(&fluid, x, y);
}

void example3(LSD* gfx, float dt) {


    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    static int last_mouse_x = mouse_x, last_mouse_y = mouse_y;
    fluid.mouse_dx = (mouse_x - last_mouse_x) * 2.0f; // Amplified
    fluid.mouse_dy = (mouse_y - last_mouse_y) * 2.0f;
    fluid.mouse_x = mouse_x;
    fluid.mouse_y = mouse_y;
    fluid.mouse_moved = (fabsf(fluid.mouse_dx) > 0.1f || fabsf(fluid.mouse_dy) > 0.1f);
    last_mouse_x = mouse_x;
    last_mouse_y = mouse_y;

    for (int i = 0; i < gfx->screenWidth * gfx->screenHeight; i++) {
        gfx->pix[i] = 0;
    }

    updateFluid(&fluid, dt);
    drawFluid(&fluid, gfx);
}

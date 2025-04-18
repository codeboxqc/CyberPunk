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

    // Initialize palette for text (not used for fluid colors anymore)
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

    // Initialize body with text and glow outline
    float* temp_body = (float*)malloc(FLUID_WIDTH * FLUID_HEIGHT * 4 * sizeof(float));
    if (!temp_body) {
        printf("Failed to allocate temp body\n");
        return;
    }
    memset(temp_body, 0, FLUID_WIDTH * FLUID_HEIGHT * 4 * sizeof(float));

    // First pass: Render text
    for (int y = 0; y < FLUID_HEIGHT; y++) {
        for (int x = 0; x < FLUID_WIDTH; x++) {
            int tx = x - (FLUID_WIDTH - fluid->text_surface->w) / 2;
            int ty = y - (FLUID_HEIGHT - fluid->text_surface->h) / 2;
            int i = (y * FLUID_WIDTH + x) * 4;
            if (tx >= 0 && tx < fluid->text_surface->w && ty >= 0 && ty < fluid->text_surface->h) {
                Uint8* pixel = (Uint8*)fluid->text_surface->pixels + ty * fluid->text_surface->pitch + tx * 4;
                if (pixel[3] > 0) {
                    temp_body[i] = 255.0f;
                    temp_body[i + 1] = 255.0f;
                    temp_body[i + 2] = 255.0f;
                    temp_body[i + 3] = 255.0f;
                }
            }
        }
    }

    // Second pass: Add pink glow outline
    for (int y = 0; y < FLUID_HEIGHT; y++) {
        for (int x = 0; x < FLUID_WIDTH; x++) {
            int i = (y * FLUID_WIDTH + x) * 4;
            if (temp_body[i + 3] == 255.0f) {
                fluid->body[i] = 255.0f;
                fluid->body[i + 1] = 255.0f;
                fluid->body[i + 2] = 255.0f;
                fluid->body[i + 3] = 255.0f;
            }
            else {
                // Check neighbors for glow
                float glow = 0.0f;
                float glow_radius = 5.0f;
                for (int dy = -5; dy <= 5; dy++) {
                    for (int dx = -5; dx <= 5; dx++) {
                        int nx = x + dx;
                        int ny = y + dy;
                        if (nx >= 0 && nx < FLUID_WIDTH && ny >= 0 && ny < FLUID_HEIGHT) {
                            int ni = (ny * FLUID_WIDTH + nx) * 4;
                            if (temp_body[ni + 3] == 255.0f) {
                                float dist = sqrtf(dx * dx + dy * dy);
                                if (dist <= glow_radius) {
                                    float g = (1.0f - dist / glow_radius);
                                    glow = fmaxf(glow, g);
                                }
                            }
                        }
                    }
                }
                if (glow > 0.0f) {
                    fluid->body[i] = 255.0f * glow;     // Pink glow
                    fluid->body[i + 1] = 105.0f * glow;
                    fluid->body[i + 2] = 180.0f * glow;
                    fluid->body[i + 3] = 255.0f * glow;
                }
            }
        }
    }
    free(temp_body);

    fluid->mouse_x = fluid->mouse_y = 0;
    fluid->mouse_dx = fluid->mouse_dy = 0;
    fluid->mouse_moved = 0;

    srand((unsigned int)time(NULL));
    //printf("Initialized fluid at (%d, %d)\n", x, y);
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
        float alpha = fminf(100.0f + speed * 2.0f, 200.0f); // 100�200
        // Dynamic color based on position and speed
        float hue = (mx + my) * 0.01f + speed * 0.05f; // Vary hue
        float r = 255.0f * fabsf(sinf(hue));
        float g = 255.0f * fabsf(sinf(hue + 2.0f));
        float b = 255.0f * fabsf(sinf(hue + 4.0f));
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
                            fluid->body[i] = r;
                            fluid->body[i + 1] = g;
                            fluid->body[i + 2] = b;
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
            fluid->body[idx + 3] *= 0.98f; // Slower fade
            if (fluid->body[idx + 3] < 1.0f) {
                fluid->body[idx] = 0;
                fluid->body[idx + 1] = 0;
                fluid->body[idx + 2] = 0;
                fluid->body[idx + 3] = 0;
            }
        }
        fluid->u[i] *= 0.995f;
        fluid->v[i] *= 0.995f;
    }
}

void drawFluid(Fluid* fluid, LSD* gfx) {
    if (!fluid->body || !gfx->pix) return;

    // First pass: Render fluid with bilinear interpolation
    float* temp_buffer = (float*)malloc(gfx->screenWidth * gfx->screenHeight * 4 * sizeof(float));
    if (!temp_buffer) {
        printf("Failed to allocate temp buffer\n");
        return;
    }
    memset(temp_buffer, 0, gfx->screenWidth * gfx->screenHeight * 4 * sizeof(float));

    for (int y = 0; y < gfx->screenHeight; y++) {
        for (int x = 0; x < gfx->screenWidth; x++) {
            float u = (float)x * FLUID_WIDTH / gfx->screenWidth;
            float v = (float)y * FLUID_HEIGHT / gfx->screenHeight;
            int u0 = (int)u;
            int v0 = (int)v;
            int u1 = u0 + 1;
            int v1 = v0 + 1;
            float fu = u - u0;
            float fv = v - v0;

            if (u1 >= FLUID_WIDTH) u1 = FLUID_WIDTH - 1;
            if (v1 >= FLUID_HEIGHT) v1 = FLUID_HEIGHT - 1;

            int i00 = (v0 * FLUID_WIDTH + u0) * 4;
            int i10 = (v0 * FLUID_WIDTH + u1) * 4;
            int i01 = (v1 * FLUID_WIDTH + u0) * 4;
            int i11 = (v1 * FLUID_WIDTH + u1) * 4;

            float w00 = (1.0f - fu) * (1.0f - fv);
            float w10 = fu * (1.0f - fv);
            float w01 = (1.0f - fu) * fv;
            float w11 = fu * fv;

            float r = 0, g = 0, b = 0, a = 0;
            for (int c = 0; c < 4; c++) {
                float v00 = fluid->body[i00 + c];
                float v10 = fluid->body[i10 + c];
                float v01 = fluid->body[i01 + c];
                float v11 = fluid->body[i11 + c];
                float value = w00 * v00 + w10 * v10 + w01 * v01 + w11 * v11;
                if (c == 0) r = value;
                else if (c == 1) g = value;
                else if (c == 2) b = value;
                else a = value;
            }

            int idx = (y * gfx->screenWidth + x) * 4;
            temp_buffer[idx] = r;
            temp_buffer[idx + 1] = g;
            temp_buffer[idx + 2] = b;
            temp_buffer[idx + 3] = a;
        }
    }

    // Second pass: Apply glow and render to screen
    for (int y = 0; y < gfx->screenHeight; y++) {
        for (int x = 0; x < gfx->screenWidth; x++) {
            int idx = (y * gfx->screenWidth + x) * 4;
            float r = temp_buffer[idx];
            float g = temp_buffer[idx + 1];
            float b = temp_buffer[idx + 2];
            float a = temp_buffer[idx + 3];

            // Apply glow by sampling neighbors
            float glow_r = 0, glow_g = 0, glow_b = 0, glow_a = 0;
            float glow_radius = 3.0f;
            float glow_weight = 0;
            for (int dy = -3; dy <= 3; dy++) {
                for (int dx = -3; dx <= 3; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < gfx->screenWidth && ny >= 0 && ny < gfx->screenHeight) {
                        float dist = sqrtf(dx * dx + dy * dy);
                        if (dist <= glow_radius) {
                            float weight = (1.0f - dist / glow_radius) * 0.2f;
                            int ni = (ny * gfx->screenWidth + nx) * 4;
                            glow_r += temp_buffer[ni] * weight;
                            glow_g += temp_buffer[ni + 1] * weight;
                            glow_b += temp_buffer[ni + 2] * weight;
                            glow_a += temp_buffer[ni + 3] * weight;
                            glow_weight += weight;
                        }
                    }
                }
            }
            if (glow_weight > 0) {
                glow_r /= glow_weight;
                glow_g /= glow_weight;
                glow_b /= glow_weight;
                glow_a /= glow_weight;
            }

            // Blend glow with original color
            r = fminf(r + glow_r * 0.5f, 255.0f);
            g = fminf(g + glow_g * 0.5f, 255.0f);
            b = fminf(b + glow_b * 0.5f, 255.0f);
            a = fminf(a + glow_a * 0.3f, 255.0f);

            pixel(gfx, x, y, (Uint8)r, (Uint8)g, (Uint8)b, (Uint8)a);
        }
    }

    free(temp_buffer);
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

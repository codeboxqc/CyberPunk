 #include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265358979323846f
#define X 1920
#define Y 1080
#define NB 999
#define TAU 1
#define GR 0.5f
#define CONT 0.5f
#define ANGADD 0.05f
#define VOR 15

 

// Starfield globals (from your code)
static int lx[NB * TAU], ly[NB * TAU], lz[NB * TAU];
static int xd, yd;
static int ra[NB], rb[NB], vo[NB];
static int n = 0, nn = 0;
static float xo = 0, yo = 0, zo = 0, xp = 0, yp = 0, zp = 0;
static float ax = 0, ay = 0;
static float xt = 0, yt = 0, zt = 0, lxn = 0, lyn = 0, lzn = 0;
static float ang = 0;
static float tps = 0;
static int fps = 0;
static int end = 0; // Default to 0

 
extern void logMessage(const char* message);

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


void fillCircle(LSD* self, int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (radius < 1) return;

    // Radial gradient star with glow
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            float dist = sqrtf(x * x + y * y);
            if (dist <= radius) {
                // Gradient: bright center to faint edge
                float t = dist / radius; // 0 (center) to 1 (edge)
                Uint8 star_r = (Uint8)((1.0f - t) * r); // Fade from r to 0
                Uint8 star_g = (Uint8)((1.0f - t) * g);
                Uint8 star_b = (Uint8)((1.0f - t) * b);
                Uint8 star_a = (Uint8)((1.0f - t * 0.7f) * a); // Softer fade for glow
                pixel(self, cx + x, cy + y, star_r, star_g, star_b, star_a);
            }
        }
    }

    // Add cross-shaped flare for larger stars (mimics sparkle)
    if (radius > 4) {
        int flare_len = radius / 2; // Shorter cross for balance
        for (int i = -flare_len; i <= flare_len; i++) {
            // Horizontal line
            pixel(self, cx + i, cy, r / 2, g / 2, b / 2, a / 3); // Faint flare
            // Vertical line
            pixel(self, cx, cy + i, r / 2, g / 2, b / 2, a / 3);
        }
    }
}

void initStarfield() {
    for (int i = 0; i < NB; i++) {
        ra[i] = rand() % 628;
        rb[i] = rand() % 628;
        vo[i] = rand() % 100 + VOR;
    }
    ra[1] = 178;
    rb[1] = rand() % 50;
    ang = -TAU * CONT;
    fps = 0;

    if (end == -1) {
        xo = 0; yo = 0; zo = 0;
        xp = 0; yp = 0; zp = 0;
        ax = 0; ay = 0;
        xt = 0; yt = 0; zt = 0;
        lxn = 0; lyn = 0; lzn = 0;
        ang = 0;
        tps = 0;
        fps = 0;
    }
}

void updateStarfield() {
    fps++;
    ang += ANGADD;

    if (ang > 16) {
        xo = 0;
        yo = ly[1];
        zo = 100;
        xp = X * sinf(ang * 0.2f) - 100;
        yp = Y * cosf(ang * 0.1f) - 50;
        zp = -700 * cosf(ang * 0.13f) + 50;
    }
    else {
        n = 0;
        for (int t = 0; t < TAU; t++) {
            tps = t * CONT + ang;
            if (tps < 0) tps = 0;
            for (int i = 0; i < NB; i++) {
                lx[n] = vo[i] * cosf(ra[i] * 0.01f) * sinf(rb[i] * 0.01f) * tps;
                ly[n] = (-0.5f * GR) * tps * tps + vo[i] * sinf(ra[i] * 0.01f) * sinf(rb[i] * 0.01f) * tps + 50;
                lz[n] = vo[i] * cosf(rb[i] * 0.01f) * tps + 100;
                n++;
            }
        }
        xo = 0;
        yo = ly[1];
        zo = 100;
        xp = X * sinf(ang * 0.2f) + 100;
        yp = Y * cosf(ang * 0.1f) - 50;
        zp = -700 * cosf(ang * 0.13f) + 50;
    }

    ay = 1.57f - acosf((yo - yp) / sqrtf((xo - xp) * (xo - xp) + (yo - yp) * (yo - yp) + (zo - zp) * (zo - zp)));
    if (zo - zp > 0) ax = atanf((xo - xp) / (zo - zp));
    if (zo - zp < 0) ax = PI + atanf((xo - xp) / (zo - zp));
}

void renderStarfield(LSD* gfx) {
    // Clear buffer (black)
    memset(gfx->pix, 0, gfx->screenHeight * gfx->pitch);

    // Draw stars as circles
    for (nn = 0; nn < n; nn++) {
        xt = (lx[nn] - xp) * cosf(ax) - (lz[nn] - zp) * sinf(ax);
        yt = (ly[nn] - yp);
        zt = (lx[nn] - xp) * sinf(ax) + (lz[nn] - zp) * cosf(ax);
        lxn = xt;
        lyn = yt * cosf(ay) - zt * sinf(ay);
        lzn = yt * sinf(ay) + zt * cosf(ay);

        if (lzn > 0) {
            xd = (X / 2) + (int)((X * lxn) / (2 * lzn));
            yd = (Y / 2) - (int)((Y * lyn) / (2 * lzn));
            if (xd > 0 && xd < X && yd > 0 && yd < Y) {
                int radius = (int)(128.0f / (lzn / 64.0f)); // Even larger stars
                if (radius > 0) {
                    radius = radius > 96 ? 96 : radius; // Cap for performance
                    fillCircle(gfx, xd, yd, radius, 255, 200, 200, 255); // Red-tinted white
                }
            }
        }
    }
}


// Initialize LSD struct
  
void example5ini(LSD* ssgfx) {
 
 
    // Initialize starfield
    initStarfield();

 
  
}

void star3d(LSD* ssgfx)
{
	// Update and render
	updateStarfield();
	renderStarfield(ssgfx);
    SDL_UpdateTexture(ssgfx->screenTexture, NULL, ssgfx->pix, ssgfx->pitch);
}

















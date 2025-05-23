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
static float cos_ra[NB], sin_ra[NB]; // Pre-calculated trig values
static float cos_rb[NB], sin_rb[NB]; // Pre-calculated trig values
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
    // Optimization: Compare squared distances to avoid sqrt in condition
    float radius_f = static_cast<float>(radius);
    float radius_sq = radius_f * radius_f;

    for (int y_offset = -radius; y_offset <= radius; y_offset++) {
        for (int x_offset = -radius; x_offset <= radius; x_offset++) {
            float dist_sq = static_cast<float>(x_offset * x_offset + y_offset * y_offset);
            if (dist_sq <= radius_sq) {
                float dist = sqrtf(dist_sq); // sqrtf is now only for pixels inside the circle
                // Gradient: bright center to faint edge
                // Ensure t is not NaN if radius_f is zero, though radius < 1 check should prevent radius_f == 0
                float t = (radius_f == 0.0f) ? 0.0f : (dist / radius_f); // 0 (center) to 1 (edge)
                
                Uint8 star_r = (Uint8)((1.0f - t) * r); // Fade from r to 0
                Uint8 star_g = (Uint8)((1.0f - t) * g);
                Uint8 star_b = (Uint8)((1.0f - t) * b);
                Uint8 star_a = (Uint8)((1.0f - t * 0.7f) * a); // Softer fade for glow
                pixel(self, cx + x_offset, cy + y_offset, star_r, star_g, star_b, star_a);
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
    // First, initialize all ra, rb, vo
    for (int i = 0; i < NB; i++) {
        ra[i] = rand() % 628;
        rb[i] = rand() % 628; 
        vo[i] = rand() % 100 + VOR;
    }
    // Then, apply special values that might overwrite previous random values
    ra[1] = 178;
    rb[1] = rand() % 50; 

    // Now, pre-calculate all trig values using the final ra and rb values
    for (int i = 0; i < NB; i++) {
        float ra_rad = ra[i] * 0.01f;
        float rb_rad = rb[i] * 0.01f;
        cos_ra[i] = cosf(ra_rad);
        sin_ra[i] = sinf(ra_rad);
        cos_rb[i] = cosf(rb_rad);
        sin_rb[i] = sinf(rb_rad);
    }
    
    // The rest of initStarfield initialization
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
                // Use pre-calculated trig values
                lx[n] = vo[i] * cos_ra[i] * sin_rb[i] * tps;
                ly[n] = (-0.5f * GR) * tps * tps + vo[i] * sin_ra[i] * sin_rb[i] * tps + 50;
                lz[n] = vo[i] * cos_rb[i] * tps + 100;
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

    // Optimized ax, ay calculation
    float dx = xo - xp;
    float dy = yo - yp;
    float dz = zo - zp;
    float d_sq = dx * dx + dy * dy + dz * dz;
    float d_sq_inv;

    if (d_sq == 0.0f) { // Check if distance is zero
        // Handle case where viewpoint and target are the same. 
        // Avoid division by zero. Orientation is undefined or could be kept from previous frame.
        // For simplicity, set ay based on some default or previous value if this case is critical.
        // Here, we might just skip updating ay or set it to a neutral value if appropriate.
        // Or, if yo == yp, ay could be 0. If yo > yp, PI/2. If yo < yp, -PI/2.
        // However, if d_sq is 0, then dy must also be 0. So asinf(0) = 0.
        ay = 0.0f; // if d_sq is 0, dy is 0. asinf(0/0) is nan, but asinf(0) is 0.
                   // The only problematic case is 0/0 for d_sq_inv if not handled.
                   // if d_sq is 0, then dy must be 0. So ay should be 0.
        d_sq_inv = 0.0f; // Avoid using inf/NaN
    } else {
        d_sq_inv = 1.0f / sqrtf(d_sq);
    }
    
    // Original ay: 1.57f - acosf((yo - yp) / sqrtf((xo - xp)*(xo - xp) + (yo - yp)*(yo - yp) + (zo - zp)*(zo - zp)))
    // ay = 1.57f - acosf(dy / sqrtf(d_sq));
    // ay = asinf(dy / sqrtf(d_sq));
    // ay = asinf(dy * d_sq_inv);
    // If d_sq was 0, dy is also 0. dy * d_sq_inv would be 0 * 0 = 0. asinf(0) = 0. Correct.
    ay = asinf(dy * d_sq_inv); // PI/2 - acos(x) = asin(x). 1.57f is approx PI/2.

    if (dz == 0.0f) { 
        if (dx > 0.0f) ax = PI / 2.0f;
        else if (dx < 0.0f) ax = -PI / 2.0f; 
        else ax = 0.0f; 
    } else {
        ax = atanf(dx / dz);
        if (dz < 0.0f) {
            // If dz is negative, atanf(dx/dz) is in (-PI/2, PI/2).
            // If dx > 0, dx/dz < 0, atanf gives angle in (-PI/2, 0) (Q4). Add PI for Q2. (Correct)
            // If dx < 0, dx/dz > 0, atanf gives angle in (0, PI/2) (Q1). Add PI for Q3. (Correct)
            // If dx = 0, atanf gives 0. Add PI for negative Z axis. (Correct)
            ax += PI; 
        }
        // ax is now in (-PI/2, 3PI/2). Normalize if needed, but original code didn't.
    }
}

void renderStarfield(LSD* gfx) {
    // Clear buffer (black)
    memset(gfx->pix, 0, gfx->screenHeight * gfx->pitch);

    // Pre-calculate cosines and sines for ax, ay as they are constant for all stars in a frame
    float cos_ax = cosf(ax);
    float sin_ax = sinf(ax);
    float cos_ay = cosf(ay);
    float sin_ay = sinf(ay);

    // Draw stars as circles
    for (nn = 0; nn < n; nn++) {
        // Store intermediate calculations for differences from viewpoint
        float l_xp = lx[nn] - xp;
        float l_yp = ly[nn] - yp; // yt is effectively l_yp before rotation by ay
        float l_zp = lz[nn] - zp;

        xt = l_xp * cos_ax - l_zp * sin_ax;
        // yt = l_yp; // This was the original yt before transformation by ay
        zt = l_xp * sin_ax + l_zp * cos_ax;
        
        // lxn is xt, no change
        lxn = xt;
        // lyn and lzn are results of rotating (yt, zt) by ay
        // Original yt was (ly[nn] - yp)
        lyn = l_yp * cos_ay - zt * sin_ay;
        lzn = l_yp * sin_ay + zt * cos_ay;

        if (lzn > 0) {
            // Simplified perspective projection
            float inv_lzn_half = 0.5f / lzn; // Calculate 1/(2*lzn) once
            xd = (X / 2) + (int)(X * lxn * inv_lzn_half);
            yd = (Y / 2) - (int)(Y * lyn * inv_lzn_half);

            if (xd > 0 && xd < X && yd > 0 && yd < Y) {
                // Simplified radius calculation: (128.0f * 64.0f) / lzn = 8192.0f / lzn
                int radius = (int)(8192.0f / lzn); 
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

















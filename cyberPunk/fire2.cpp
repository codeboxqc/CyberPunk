#include "fire2.h"
#include "lsd.h" // lsd.h is in the same directory (cyberPunk/)
#include <vector>
#include <cstdlib> // For rand()
#include <SDL2/SDL_pixels.h> // For SDL_Color (SDL_palettes.h is not usually needed for just SDL_Color)
#include <stdexcept> // For std::bad_alloc
#include <algorithm> // For std::min/max if used for clamping (though not explicitly in new palette code)


// Define these if not available or to scope them locally.
// These should match the LSD screen dimensions passed to initFire2.
static int FIRE_WIDTH = 0;
static int FIRE_HEIGHT = 0;

// Palette for the fire effect (e.g., 256 colors)
static SDL_Color firePalette[256];
// Buffer to store fire pixel values (heat)
static std::vector<unsigned char> fireBuffer; // Using a 1D vector for a 2D grid

namespace FireEffect2 {

// Helper to get/set pixel in fireBuffer
inline unsigned char getFirePixel(int x, int y) {
    // Pixels outside the fire buffer (e.g. y+2 for the top rows) should return 0 (cold)
    if (x < 0 || x >= FIRE_WIDTH || y < 0 || y >= FIRE_HEIGHT) {
        return 0;
    }
    return fireBuffer[(size_t)y * FIRE_WIDTH + x];
}

inline void setFirePixel(int x, int y, unsigned char value) {
    if (x >= 0 && x < FIRE_WIDTH && y >= 0 && y < FIRE_HEIGHT) {
        fireBuffer[(size_t)y * FIRE_WIDTH + x] = value;
    }
}

void createPalette() {
    for (int i = 0; i < 256; ++i) {
        firePalette[i].r = 0;
        firePalette[i].g = 0;
        firePalette[i].b = 0;
        firePalette[i].a = 255;

        if (i < 32) { // Black to dark red
            firePalette[i].r = i * 2; // 0-62
        } else if (i < 64) { // Dark red to bright red
            firePalette[i].r = (i-32) * 4 + 64; // 64-191
        } else if (i < 128) { // Bright red to orange
            firePalette[i].r = 255;
            firePalette[i].g = (i - 64) * 2; // 0-127
        } else if (i < 192) { // Orange to yellow
            firePalette[i].r = 255;
            firePalette[i].g = 128 + (i-128); // 128-191
            firePalette[i].b = (i-128) * 2; //0-127
        } else { // Yellow to white
            firePalette[i].r = 255;
            firePalette[i].g = 192 + ((i-192)*63)/64; // 192-255
            firePalette[i].b = 128 + ((i-192)*127)/64; //128-255
            if (i > 224) { // Brighter white
                 firePalette[i].g = 255;
                 // Ensure b does not exceed 255 from the multiplication
                 int temp_b = (i-224)*255/32;
                 firePalette[i].b = (temp_b > 255) ? 255 : temp_b;
            }
        }
         // Ensure full red for a significant portion for a fiery look
        if (i >= 64 && i < 128) firePalette[i].r = 255;
        if (i >= 128 && i < 256) firePalette[i].r = 255;


        // Clamp values just in case, though logic above should be okay
        // Explicit clamping for safety, can be removed if palette logic is proven correct.
        if(firePalette[i].r > 255) firePalette[i].r = 255; else if(firePalette[i].r < 0) firePalette[i].r = 0;
        if(firePalette[i].g > 255) firePalette[i].g = 255; else if(firePalette[i].g < 0) firePalette[i].g = 0;
        if(firePalette[i].b > 255) firePalette[i].b = 255; else if(firePalette[i].b < 0) firePalette[i].b = 0;
    }
     // A common approach: make the very first palette entry transparent black
     // This helps "extinguish" pixels at the top or where heat is 0.
    firePalette[0].r = 0; firePalette[0].g = 0; firePalette[0].b = 0; firePalette[0].a = 0; 
}


bool initFire2(LSD* lsdContext, int screenWidth, int screenHeight) {
    if (!lsdContext) return false;

    FIRE_WIDTH = screenWidth;
    FIRE_HEIGHT = screenHeight;
    
    if (FIRE_WIDTH <= 0 || FIRE_HEIGHT <= 0) return false;


    createPalette();

    try {
        fireBuffer.assign((size_t)FIRE_WIDTH * FIRE_HEIGHT, 0);
    } catch (const std::bad_alloc& e) {
        // logMessage("Failed to allocate memory for fireBuffer in initFire2"); // If logMessage is available
        return false;
    }

    // Seed the bottom row
    for (int x = 0; x < FIRE_WIDTH; ++x) {
        setFirePixel(x, FIRE_HEIGHT - 1, rand() % 256);
    }
    return true;
}

void updateFire2() {
    if (FIRE_WIDTH == 0 || FIRE_HEIGHT == 0) return; // Not initialized

    // Seed the bottom row with random hot values continuously for a lively effect
    for (int x = 0; x < FIRE_WIDTH; ++x) {
        setFirePixel(x, FIRE_HEIGHT - 1, (rand() % 128) + 128); // Hotter values
    }

    for (int y = 0; y < FIRE_HEIGHT - 1; ++y) { // Iterate up to the second to last row
        for (int x = 0; x < FIRE_WIDTH; ++x) {
            unsigned int heat = 0; // Use unsigned int for sum to avoid intermediate overflow
            heat += getFirePixel(x, y + 1);
            heat += getFirePixel(x - 1, y + 1);
            heat += getFirePixel(x + 1, y + 1);
            heat += getFirePixel(x, y + 2); // Sample from two rows below

            unsigned char newHeat = static_cast<unsigned char>(heat / 4.05f); 
            setFirePixel(x, y, newHeat);
        }
    }
}

void renderFire2(LSD* lsdContext) {
    if (!lsdContext || !lsdContext->pix || FIRE_WIDTH == 0 || FIRE_HEIGHT == 0) return;

    for (int y = 0; y < FIRE_HEIGHT; ++y) {
        for (int x = 0; x < FIRE_WIDTH; ++x) {
            unsigned char heatValue = getFirePixel(x, y);
            SDL_Color color = firePalette[heatValue]; // heatValue is already 0-255
            pixel(lsdContext, x, y, color.r, color.g, color.b, color.a);
        }
    }
}

void closeFire2() {
    fireBuffer.clear();
    fireBuffer.shrink_to_fit(); // Attempt to release memory
    FIRE_WIDTH = 0;
    FIRE_HEIGHT = 0;
    // Palette is static, no need to free.
}

} // namespace FireEffect2

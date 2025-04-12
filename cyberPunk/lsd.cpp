//NuGet\Install-Package sdl2.nuget -Version 2.30.7
//NuGet\Install-Package sdl2_image.nuget -Version 2.8.2
//NuGet\Install-Package sdl2_ttf.nuget -Version 2.22.0
//NuGet\Install-Package sdl2_mixer.nuget -Version 2.8.0

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>

#include <vector>   // For std::vector
#include <utility>  // For std::pair
#include <math.h>
#include <stdlib.h>


//#include "punk.h"

extern void logMessage(const char* message);

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;


///drawing fonction
/*
void pixel(int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
void circle(int xo, int yo, int rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
void filledRectangle(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
void triangle(int x1, int y1, int x2, int y2, int x3, int y3, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255) {
void ellipse(int xc, int yc, int a_axis, int b_axis, Uint8 r, Uint8 g, Uint8 b, Uint8 a_val = 255)
void arc(int xc, int yc, int rad, float startAngle, float endAngle, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
void drawDashedLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255, int dashLength = 5)
void drawline(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
void Rectangle(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)

/std::vector<std::pair<int, int>> points = { {100, 100}, {200, 100}, {200, 200}, {100, 200} };
    //polygon(points, 255, 0, 0);  // Red square
void polygon(std::vector<std::pair<int, int>>& points, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
 void clear(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)



 /Uint8 a = 255 alphA COLOR TRANSPARENCY

 while (running) {
    SDL.begin_frame();

    // Draw some pixels
    SDL.pixel(100, 100, 255, 0, 0); // red
    SDL.pixel(101, 100, 0, 255, 0); // green
    SDL.pixel(102, 100, 0, 0, 255); // blue

    SDL.end_frame();
}
 */



bool iniSDL(const char* title, int sx, int sy) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        //std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        logMessage("SDL_Init failed.\n");

  
        return false;
    }

    int numDisplays = SDL_GetNumVideoDisplays();
    if (numDisplays < 1) {
        logMessage("No video displays found.\n");
        return false;
    }

    // Use the primary display (usually 0), but fall back if needed
    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
        //std::cerr << "SDL_GetDesktopDisplayMode(0) failed: " << SDL_GetError() << "\n";
        logMessage("SDL_GetDesktopDisplayMode(0) failed\n");
        // Try display 1 if available
        if (numDisplays > 1 && SDL_GetDesktopDisplayMode(1, &dm) == 0) {
            //std::cerr << "Using display 1 instead.\n";
            logMessage("Using display 1 instead.\n");
        }
        else {
            //std::cerr << "Failed to get any display mode.\n";
            logMessage("Failed to get any display mode.\n");
            return false;
        }
    }

    window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED_DISPLAY(0), SDL_WINDOWPOS_CENTERED_DISPLAY(0),
        dm.w, dm.h,
        SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_BORDERLESS
    );

    if (!window) {
        // std::cerr << "Window creation failed: " << SDL_GetError() << "\n";
        logMessage("Window creation failed.\n");
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        //std::cerr << "Renderer creation failed: " << SDL_GetError() << "\n";
        logMessage(" Renderer creation failed.\n");
        return false;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_RenderSetLogicalSize(renderer, sx, sy);
    logMessage("Sdl ini begin true .\n");
    return true;
}










////////////////////////////////////////////////////////////
//
//
 







 

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

// Initialization
int init(LSD* self, SDL_Renderer* renderer, int width, int height,int backimage) {
    self->gRenderer = renderer;  // Set gRenderer to the passed renderer
    self->renderer = renderer;   // Also set renderer to the passed renderer
    self->screenWidth = width;
    self->screenHeight = height;


    if (backimage == 0) {
        // Create a texture for rendering
        self->screenTexture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            width, height);

        if (!self->screenTexture) return 0;  // Return 0 if texture creation fails
    }

    else {

        self->screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
        if (!self->screenTexture) {
            return 0;  // Error
        }

        // Optionally, set blend mode here if needed
        SDL_SetTextureBlendMode(self->screenTexture, SDL_BLENDMODE_BLEND);

    }

    // Set texture blend mode for transparency (optional)
    SDL_SetTextureBlendMode(self->screenTexture, SDL_BLENDMODE_BLEND);

    // Initialize pitchPixels (assumes 4 bytes per pixel)
    self->pitch = width * 4;            // Assuming 4 bytes per pixel
    self->pitchPixels = self->pitch / 4; // Store the pitch divided by 4 for faster access

    self->pix = (Uint32*)malloc(width * height * sizeof(Uint32));
    if (!self->pix) {
        logMessage("Failed to allocate pixel buffer\n");
        SDL_DestroyTexture(self->screenTexture);
        return 0;
    }
    memset(self->pix, 0, width * height * sizeof(Uint32)); // Clear to black

    return 1; // Return 1 to indicate success
}

// Cleanup
void destroy(LSD* self) {
    if (self->pix) {
        free(self->pix);
        self->pix = NULL;
    }
    if (self->screenTexture) {
        SDL_DestroyTexture(self->screenTexture);
    }
}

// Begin frame
void begin_frame(LSD* self, int methode) {
    if (methode == 0) {
        memset(self->pix, 0, self->screenWidth * self->screenHeight * sizeof(Uint32));  
    }
    else if (methode == 1) {
        // Use method 1: Lock the texture and render to it later (if needed)
        SDL_LockTexture(self->screenTexture, NULL, (void**)&self->pix, &self->pitch);
    }
}

void end_frame(LSD* self, int methode) {
    if (methode == 0) {
        // Method 0: Update texture with pixel buffer and render
        SDL_UpdateTexture(self->screenTexture, NULL, self->pix, self->pitch);
        SDL_RenderClear(self->gRenderer);
        SDL_RenderCopy(self->gRenderer, self->screenTexture, NULL, NULL);
        SDL_RenderPresent(self->gRenderer);
    }
    else if (methode == 1) {
        // Use SDL_LockTexture and SDL_UnlockTexture method (slower, but direct pixel manipulation)
        SDL_UnlockTexture(self->screenTexture);
    }
}




//////////////////////////////////////
//SDL_Texture* tileTexture = loadTexture(renderer, "tile.png");
//SDL_Texture* spriteTexture = loadTexture(renderer, "sprite.png");
// Rendering as a tile or sprite
//renderTile(&gfx, tileTexture, 50, 50, 32, 32);  // Example for tile
//renderSprite(&gfx, spriteTexture, 100, 100, 64, 64);  // Example for sprite
// SDL_Texture* wallpaper= loadTexture(renderer, "wallpaper.png");
//renderBackground(renderer, * wallpaper)

SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* loadedSurface = SDL_LoadBMP(path); // You can change this to load PNG/JPG with SDL_image
    if (!loadedSurface) {
        logMessage("Unable to load image");
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    return texture;
}

void renderBackground(LSD* self, SDL_Texture* backgroundTexture) {
    SDL_RenderCopy(self->gRenderer, backgroundTexture, NULL, NULL);  // Render the entire background texture
}

void renderTile(LSD* self, SDL_Texture* tileTexture, int x, int y,int xx,int yy) {
    SDL_Rect dstRect = { x, y, xx, yy };
    SDL_RenderCopy(self->gRenderer, tileTexture, NULL, &dstRect);
}

void renderSprite(LSD* self, SDL_Texture* spriteTexture, int x, int y, int xx, int yy) {
    SDL_Rect dstRect = { x, y, xx, yy};
    SDL_RenderCopy(self->gRenderer, spriteTexture, NULL, &dstRect);
}
///////////////////////////////////////////////////////////






void getPixelColor2(LSD* self, int x, int y, int& r, int& g, int& b) {
    if (x < 0 || x >= self->screenWidth || y < 0 || y >= self->screenHeight) {
        r = g = b = 0;
        return;
    }

    Uint32 color = self->pix[y * (self->pitch / 4) + x];

    r = (color >> 16) & 0xFF;
    g = (color >> 8) & 0xFF;
    b = color & 0xFF;
}

Uint32 getPixelColor(LSD* self, int x, int y) {
    if (x < 0 || x >= self->screenWidth || y < 0 || y >= self->screenHeight)
        return 0; // Or some default color like 0x00000000

    return self->pix[y * (self->pitch / 4) + x];
}

//uint8_t red;
//getPixelColor(&myLSD, 10, 10, &red, NULL, NULL, NULL);
// Fast version without safety checks
static inline void getPixelColor3(LSD* self, int x, int y,
    uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a) {
    uint32_t color = self->pix[y * (self->pitch / 4) + x];
    *a = (color >> 24) & 0xFF;
    *r = (color >> 16) & 0xFF;
    *g = (color >> 8) & 0xFF;
    *b = color & 0xFF;
}



// Pixel
  void pixel(LSD* self, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (x < 0 || x >= self->screenWidth || y < 0 || y >= self->screenHeight) return;
    Uint32 color = (a << 24) | (r << 16) | (g << 8) | b;
    self->pix[y * (self->pitch / 4) + x] = color;
}

Uint32 colorRGB(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    return (a << 24) | (r << 16) | (g << 8) | b;
}

static inline void pixel2(LSD* self, int x, int y, Uint32 color) {
    if ((unsigned)x >= (unsigned)self->screenWidth || (unsigned)y >= (unsigned)self->screenHeight) return;
    self->pix[y * self->pitchPixels + x] = color;
}

// Line (Bresenham)
void line(LSD* self, int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;

    while (1) {
        pixel(self, x1, y1, r, g, b, a);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

// Circle
void circle(LSD* self, int xc, int yc, int rad, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    for (float ang = 0; ang < 6.28318f; ang += 0.01f) {
        int x = (int)(xc + cosf(ang) * rad);
        int y = (int)(yc + sinf(ang) * rad);
        pixel(self, x, y, r, g, b, a);
    }
}

// Filled rectangle
void filled_rect(LSD* self, int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            pixel(self, x, y, r, g, b, a);
        }
    }
}



 

void ellipse(LSD* self, int xc, int yc, int a_axis, int b_axis, Uint8 r, Uint8 g, Uint8 b, Uint8 a_val ) {
    int x = 0;
    int y = b_axis;  // Use 'b_axis' instead of 'b'
    int a2 = a_axis * a_axis;   // Use 'a_axis' for calculations
    int b2 = b_axis * b_axis;   // Use 'b_axis' for calculations
    int err = a2 - (2 * a2 * y);

    // Draw the ellipse
    while (x <= a_axis) {  // Use 'a_axis' in the loop condition
        pixel(self, xc + x, yc + y, r, g, b, a_val);
        pixel(self, xc - x, yc + y, r, g, b, a_val);
        pixel(self, xc + x, yc - y, r, g, b, a_val);
        pixel(self, xc - x, yc - y, r, g, b, a_val);

        err += b2;
        if (2 * err + a2 * (1 - 2 * y) > 0) {
            y--;
            err += a2 * (1 - 2 * y);
        }

        if (2 * err + b2 * (2 * x + 1) < 0) {
            x++;
            err += b2 * (2 * x + 1);
        }
    }
}


void triangle(LSD* self, int x1, int y1, int x2, int y2, int x3, int y3, Uint8 r, Uint8 g, Uint8 b, Uint8 a ) {
    line(self, x1, y1, x2, y2, r, g, b, a);
    line(self, x2, y2, x3, y3, r, g, b, a);
    line(self, x3, y3, x1, y1, r, g, b, a);
}


void arc(LSD* self, int xc, int yc, int rad, float startAngle, float endAngle, Uint8 r, Uint8 g, Uint8 b, Uint8 a ) {
    float angleStep = 0.01f;
    for (float ang = startAngle; ang <= endAngle; ang += angleStep) {
        int x = (int)(xc + cos(ang) * rad);
        int y = (int)(yc + sin(ang) * rad);
        pixel(self, x, y, r, g, b, a);
    }
}

void drawDashedLine(LSD* self, int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a , int dashLength ) {
    int deltax = abs(x2 - x1);
    int deltay = abs(y2 - y1);
    int incx = (x2 > x1) ? 1 : (x2 < x1) ? -1 : 0;
    int incy = (y2 > y1) ? 1 : (y2 < y1) ? -1 : 0;

    int x = x1;
    int y = y1;

    int dashCounter = 0;
    if (deltax > deltay) {
        int error = deltax / 2;
        for (int i = 0; i <= deltax; i++) {
            if (dashCounter % (2 * dashLength) < dashLength) {
                pixel(self, x, y, r, g, b, a);  // Draw pixel for the dash
            }
            error -= deltay;
            if (error < 0) {
                y += incy;
                error += deltax;
            }
            x += incx;
            dashCounter++;
        }
    }
    else {
        int error = deltay / 2;
        for (int i = 0; i <= deltay; i++) {
            if (dashCounter % (2 * dashLength) < dashLength) {
                pixel(self, x, y, r, g, b, a);
            }
            error -= deltax;
            if (error < 0) {
                x += incx;
                error += deltay;
            }
            y += incy;
            dashCounter++;
        }
    }
}



 

typedef struct { int x, y; } Point;

void polygon(LSD* self, Point* points, int numPoints, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    for (int i = 0; i < numPoints; i++) {
        Point p1 = points[i];
        Point p2 = points[(i + 1) % numPoints];
        line(self, p1.x, p1.y, p2.x, p2.y, r, g, b, a);
    }
}



void Rectangle(LSD* self, int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a  ) {
    // Draw top and bottom horizontal lines
    for (int x = x1; x <= x2; ++x) {
        pixel(self, x, y1, r, g, b, a);  // Top line
        pixel(self, x, y2, r, g, b, a);  // Bottom line
    }

    // Draw left and right vertical lines
    for (int y = y1; y <= y2; ++y) {
        pixel(self, x1, y, r, g, b, a);  // Left line
        pixel(self, x2, y, r, g, b, a);  // Right line
    }
}



// Clear screen
void clear(LSD* self, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    Uint32 color = (a << 24) | (r << 16) | (g << 8) | b;
    int rowPitch = self->pitch / 4;
    for (int y = 0; y < self->screenHeight; y++) {
        Uint32* row = &self->pix[y * rowPitch];
        for (int x = 0; x < self->screenWidth; x++) {
            row[x] = color;
        }
    }
}

 


//
//
/////////////////////////////////////////////////////////////////

typedef struct {
    SDL_Texture* texture;  // The texture containing the sprite image
    int width;             // Width of the sprite
    int height;            // Height of the sprite
} Sprite;

// Load a PNG file into a Sprite structure
Sprite* load_sprite(SDL_Renderer* renderer, const char* filename) {
    // Initialize SDL_image if not already initialized
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image init failed: %s\n", IMG_GetError());
        return NULL;
    }

    // Load the PNG file as a surface
    SDL_Surface* surface = IMG_Load(filename);
    if (!surface) {
        printf("Failed to load image '%s': %s\n", filename, IMG_GetError());
        return NULL;
    }

    // Create a texture from the surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return NULL;
    }

    // Enable alpha blending for the texture
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    // Allocate memory for the sprite structure
    Sprite* sprite = (Sprite*)malloc(sizeof(Sprite));
    if (!sprite) {
        printf("Failed to allocate memory for sprite\n");
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
        return NULL;
    }

    // Fill the sprite structure
    sprite->texture = texture;
    sprite->width = surface->w;
    sprite->height = surface->h;

    // Free the surface as it's no longer needed
    SDL_FreeSurface(surface);

    return sprite;
}

// Free the sprite and its resources
void free_sprite(Sprite* sprite) {
    if (sprite) {
        if (sprite->texture) {
            SDL_DestroyTexture(sprite->texture);
        }
        free(sprite);
    }
}

// Draw the sprite at position (x, y) with alpha
void putimage(SDL_Renderer* renderer, Sprite* sprite, int x, int y, Uint8 alpha) {
    if (!sprite || !sprite->texture) return;

    // Set the texture's alpha modulation
    SDL_SetTextureAlphaMod(sprite->texture, alpha);

    SDL_Rect dest = { x, y, sprite->width, sprite->height };
    SDL_RenderCopy(renderer, sprite->texture, NULL, &dest);
}

// Draw the sprite at position (x, y) with scaling and alpha
void putimage_scaled(SDL_Renderer* renderer, Sprite* sprite, int x, int y, float scale, Uint8 alpha) {
    if (!sprite || !sprite->texture) return;

    // Set the texture's alpha modulation
    SDL_SetTextureAlphaMod(sprite->texture, alpha);

    SDL_Rect dest = {
        x, y,
        (int)(sprite->width * scale),
        (int)(sprite->height * scale)
    };
    SDL_RenderCopy(renderer, sprite->texture, NULL, &dest);
}

// Draw the sprite at position (x, y) with rotation (in degrees) and alpha
void putimage_rotated(SDL_Renderer* renderer, Sprite* sprite, int x, int y, double angle, Uint8 alpha) {
    if (!sprite || !sprite->texture) return;

    // Set the texture's alpha modulation
    SDL_SetTextureAlphaMod(sprite->texture, alpha);

    SDL_Rect dest = { x, y, sprite->width, sprite->height };
    SDL_RenderCopyEx(renderer, sprite->texture, NULL, &dest, angle, NULL, SDL_FLIP_NONE);
}


void putimage_tinted(SDL_Renderer* renderer, Sprite* sprite, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 alpha) {
    if (!sprite || !sprite->texture) return;

    SDL_SetTextureColorMod(sprite->texture, r, g, b);
    SDL_SetTextureAlphaMod(sprite->texture, alpha);

    SDL_Rect dest = { x, y, sprite->width, sprite->height };
    SDL_RenderCopy(renderer, sprite->texture, NULL, &dest);
}
//SDL_FLIP_HORIZONTAL + SDL_FLIP_Verticaly
void putimage_flipped(SDL_Renderer* renderer, Sprite* sprite, int x, int y, SDL_RendererFlip flip, Uint8 alpha) {
    if (!sprite || !sprite->texture) return;

    SDL_SetTextureAlphaMod(sprite->texture, alpha);

    SDL_Rect dest = { x, y, sprite->width, sprite->height };
    SDL_RenderCopyEx(renderer, sprite->texture, NULL, &dest, 0.0, NULL, flip);
}




//SDL_BLENDMODE_BLEND: Standard alpha blending(default).
//SDL_BLENDMODE_ADD : Additive blending(good for glows or lights).
//SDL_BLENDMODE_MOD : Multiplicative blending(good for shadows or darkening).
//SDL_BLENDMODE_NONE : No blending(ignores alpha).
void putimage_blend(SDL_Renderer* renderer, Sprite* sprite, int x, int y, SDL_BlendMode blend_mode, Uint8 alpha) {
    if (!sprite || !sprite->texture) return;

    // Set the texture's alpha modulation
    SDL_SetTextureAlphaMod(sprite->texture, alpha);

    // Set the blending mode
    SDL_SetTextureBlendMode(sprite->texture, blend_mode);

    SDL_Rect dest = { x, y, sprite->width, sprite->height };
    SDL_RenderCopy(renderer, sprite->texture, NULL, &dest);
}

//progress: A value from 0.0 to 1.0 representing the fade progress (0 = start, 1 = end
//float fade_progress = 0.0f;
//putimage_fade(renderer, sprite, 100, 100, 0, 255, fade_progress);
//fade_progress += 0.01f;  // Increment progress for smooth fade
//if (fade_progress > 1.0f) fade_progress = 1.0f;  // Cap at fully faded in
void putimage_fade(SDL_Renderer* renderer, Sprite* sprite, int x, int y, Uint8 start_alpha, Uint8 end_alpha, float progress) {
    if (!sprite || !sprite->texture) return;

    // Clamp progress between 0.0 and 1.0
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    // Linearly interpolate between start_alpha and end_alpha
    Uint8 alpha = (Uint8)(start_alpha + (end_alpha - start_alpha) * progress);

    // Set the texture's alpha modulation
    SDL_SetTextureAlphaMod(sprite->texture, alpha);

    SDL_Rect dest = { x, y, sprite->width, sprite->height };
    SDL_RenderCopy(renderer, sprite->texture, NULL, &dest);
}


//shear_x: Horizontal shear factor(e.g., 0.5 skews right, -0.5 skews left).
//shear_y : Vertical shear factor(e.g., 0.5 skews down, -0.5 skews up).
// Shear horizontally (leans right)
//putimage_sheared(renderer, sprite, 100, 100, 0.5f, 0.0f, 255);
// Shear vertically (leans down)
//putimage_sheared(renderer, sprite, 200, 100, 0.0f, 0.5f, 255);
// Shear both ways
//putimage_sheared(renderer, sprite, 300, 100, 0.3f, 0.3f, 128);
void putimage_sheared(SDL_Renderer* renderer, Sprite* sprite, int x, int y, float shear_x, float shear_y, Uint8 alpha) {
    if (!sprite || !sprite->texture) return;

    // Set the texture's alpha modulation
    SDL_SetTextureAlphaMod(sprite->texture, alpha);

    // Define the destination points for shearing
    SDL_Point points[4] = {
        { x, y },                                      // Top-left
        { x + sprite->width + (int)(shear_x * sprite->height), y },  // Top-right
        { x + sprite->width, y + sprite->height },   // Bottom-right
        { x + (int)(shear_y * sprite->height), y + sprite->height }  // Bottom-left
    };

    // Note: SDL2 doesn't directly support shearing with SDL_RenderCopy.
    // This is a basic approximation. For true shearing, you'd need SDL_gpu or OpenGL.
    // Here, we'll stretch the sprite to simulate a shear effect.
    SDL_Rect dest = {
        x,
        y,
        sprite->width + (int)(shear_x * sprite->height),
        sprite->height + (int)(shear_y * sprite->width)
    };
    SDL_RenderCopy(renderer, sprite->texture, NULL, &dest);
}



////////////////////////
//Structure for Light
/*
typedef struct {
    int x, y;            // Position of the light
    Uint8 r, g, b, a;    // Color (RGBA) of the light
    float intensity;     // Intensity of the light (0 to 1)
    float radius;        // Radius of the light's effect
} Light;



// Adjusts the color of a tile based on light source
void applyLightingToTile(Uint32* pixel, int x, int y, Light* light) {
    // Calculate the distance from the light source to the tile
    float dx = x - light->x;
    float dy = y - light->y;
    float distance = sqrt(dx * dx + dy * dy);

    // If the tile is within the light's radius
    if (distance <= light->radius) {
        // Calculate the lighting factor based on the distance (fading effect)
        float lightingFactor = 1.0f - (distance / light->radius);
        lightingFactor *= light->intensity;  // Apply intensity

        // Extract current pixel color (assuming ARGB format)
        Uint8 r = (*pixel >> 16) & 0xFF;
        Uint8 g = (*pixel >> 8) & 0xFF;
        Uint8 b = (*pixel >> 0) & 0xFF;

        // Apply the lighting effect (simple additive blend)
        r = (Uint8)fmin(r + light->r * lightingFactor, 255);
        g = (Uint8)fmin(g + light->g * lightingFactor, 255);
        b = (Uint8)fmin(b + light->b * lightingFactor, 255);

        // Store the new pixel color
        *pixel = (r << 16) | (g << 8) | b;
    }
}


void renderSceneWithLighting(SDL_Renderer* renderer, Uint32* screenPixels, int width, int height, Light* lights, int lightCount) {
    // Loop through every pixel in the scene
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Uint32* pixel = &screenPixels[y * width + x];

            // Apply lighting for each light source
            for (int i = 0; i < lightCount; i++) {
                applyLightingToTile(pixel, x, y, &lights[i]);
            }
        }
    }

    // Once lighting is applied, you can render the scene to the screen
    SDL_UpdateTexture(screenTexture, NULL, screenPixels, width * 4);  // Update the texture
    SDL_RenderCopy(renderer, screenTexture, NULL, NULL);  // Render the texture to the screen
    SDL_RenderPresent(renderer);  // Present the rendered frame
}
*/
/*
Light lights[2];
lights[0] = {320, 240, 255, 255, 0, 255, 1.0f, 100.0f};  // Yellow light
lights[1] = {100, 100, 0, 0, 255, 255, 0.8f, 50.0f};   // Blue light
Uint32* screenPixels = (Uint32*)malloc(width * height * sizeof(Uint32));

// Clear screen
memset(screenPixels, 0, width * height * sizeof(Uint32));

// Render tiles and apply lighting
renderSceneWithLighting(renderer, screenPixels, width, height, lights, 2);


*/
 
#pragma once


#include <SDL2/SDL.h>
#include <vector>
#include <utility>


#include <SDL2/SDL_Main.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>


extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern int X, Y;

void logMessage(const char* message);
bool iniSDL(const char* title, int sx, int sy);

#ifndef LSD_H
#define LSD_H

 

// LSD struct definition
 

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

// Function declarations here...
int init(LSD* self, SDL_Renderer* renderer, int width, int height, int backimage);
void destroy(LSD*);
void begin_frame(LSD* self, int methode);
void end_frame(LSD* self, int methode);
void clear(LSD*, Uint8, Uint8, Uint8, Uint8);
 
//Uint8 getR(Uint32 color) { return (color >> 16) & 0xFF; }
//Uint8 getG(Uint32 color) { return (color >> 8) & 0xFF; }
//Uint8 getB(Uint32 color) { return color & 0xFF; }
//Uint8 getA(Uint32 color) { return (color >> 24) & 0xFF; }
Uint32 getPixelColor(LSD* self, int x, int y);
void getPixelColor2(LSD* self, int x, int y, int& r, int& g, int& b);

//uint8_t red;
//getPixelColor(&myLSD, 10, 10, &red, NULL, NULL, NULL);
static inline void getPixelColor3(LSD* self, int x, int y,
    uint8_t* r, uint8_t* g, uint8_t* b, uint8_t* a);


void line(LSD*, int, int, int, int, Uint8, Uint8, Uint8, Uint8);
void circle(LSD*, int, int, int, Uint8, Uint8, Uint8, Uint8);
void filled_rect(LSD*, int, int, int, int, Uint8, Uint8, Uint8, Uint8);
void ellipse(LSD*, int, int, int, int, Uint8, Uint8, Uint8, Uint8);
void triangle(LSD*, int, int, int, int, int, int, Uint8, Uint8, Uint8, Uint8);
void arc(LSD*, int, int, int, float, float, Uint8, Uint8, Uint8, Uint8);
void drawDashedLine(LSD*, int, int, int, int, Uint8, Uint8, Uint8, Uint8, int);
typedef struct { int x, y; } Point;
void polygon(LSD*, Point*, int, Uint8, Uint8, Uint8, Uint8);
void Rectangle(LSD*, int, int, int, int, Uint8, Uint8, Uint8, Uint8);


Uint32 colorRGB(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
static inline void pixel2(LSD* self, int x, int y, Uint32 color);
 void pixel(LSD* self, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a);






///////////////////////////////////////////////////////

 

typedef struct {
    SDL_Texture* texture;  // The texture containing the sprite image
    int width;             // Width of the sprite
    int height;            // Height of the sprite
} Sprite;

// Function declarations
Sprite* load_sprite(SDL_Renderer* renderer, const char* filename);
void free_sprite(Sprite* sprite);
void putimage(SDL_Renderer* renderer, Sprite* sprite, int x, int y, Uint8 alpha);
void putimage_scaled(SDL_Renderer* renderer, Sprite* sprite, int x, int y, float scale, Uint8 alpha);
void putimage_rotated(SDL_Renderer* renderer, Sprite* sprite, int x, int y, double angle, Uint8 alpha);

//SDL_BLENDMODE_BLEND: Standard alpha blending(default).
//SDL_BLENDMODE_ADD : Additive blending(good for glows or lights).
//SDL_BLENDMODE_MOD : Multiplicative blending(good for shadows or darkening).
//SDL_BLENDMODE_NONE : No blending(ignores alpha).
void putimage_blend(SDL_Renderer* renderer, Sprite* sprite, int x, int y, SDL_BlendMode blend_mode, Uint8 alpha);
void putimage_tinted(SDL_Renderer* renderer, Sprite* sprite, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 alpha);
void putimage_flipped(SDL_Renderer* renderer, Sprite* sprite, int x, int y, SDL_RendererFlip flip, Uint8 alpha);

//progress: A value from 0.0 to 1.0 representing the fade progress(0 = start, 1 = end
void putimage_fade(SDL_Renderer* renderer, Sprite* sprite, int x, int y, Uint8 start_alpha, Uint8 end_alpha, float progress);
void putimage_sheared(SDL_Renderer* renderer, Sprite* sprite, int x, int y, float shear_x, float shear_y, Uint8 alpha);





////////////////////////audio

 const int audiox =20 ;

// AudioManager structure
typedef struct {
    Mix_Chunk* soundEffects[audiox];  // Array for sound effects
    Mix_Music* bgMusic;                  // Background music
} AudioManager;

// Function declarations
int initAudio(AudioManager* audio);
void loadSound(AudioManager* audio, int index, const char* path);
void loadMusic(AudioManager* audio, const char* path);
int playSound(AudioManager* audio, int index);
void playMusic(AudioManager* audio);
void cleanupAudio(AudioManager* audio);

void setSoundVolume(AudioManager* audio, int index, int volume);
void setMusicVolume(AudioManager* audio, int volume);

void pauseMusic(AudioManager* audio);
void resumeMusic(AudioManager* audio);
void stopMusic(AudioManager* audio);

void fadeInMusic(AudioManager* audio, int fadeTimeMs);
void fadeOutMusic(AudioManager* audio, int fadeTimeMs);

int playSoundLooped(AudioManager* audio, int index, int loops);
void stopSound(int channel);

int playSoundPanned(AudioManager* audio, int index, int pan);
int playSoundDistance(AudioManager* audio, int index, float distance, float maxDistance);

// Test function (optional, remove if not needed in other files)
void testaudio(void);


////////////////////////////////////






// Define a dead zone threshold for joysticks
#define JOYSTICK_DEADZONE 8000
#define DOUBLE_TAP_TIME 300

// Input structure to store all key states
typedef struct {
    int up, down, left, right;
    int escape, ctrl, space, alt, enter;
    int mouseLeft, mouseRight;
    int mouseX, mouseY;
    int sprint, dash;  // Sprint/Dash mechanics
    int scrollUp, scrollDown;  // Mouse wheel
    int specialMove;  // Gesture-based input detection
} InputState;

// Joystick pointer (shared across input functions)
extern SDL_Joystick* joystick;

// Function prototypes
int initInput(InputState* input);   // Initialize input system
void processInput(InputState* input);  // Process SDL events and update input state
void cleanupInput(void);  // Cleanup input system








typedef enum {
    TEXT_RENDER_SOLID,
    TEXT_RENDER_BLENDED,
    TEXT_RENDER_SHADED
} TextRenderMode;

typedef enum {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
} TextAlign;

typedef enum {
    ANIM_NONE,
    ANIM_FADE,
    ANIM_SCALE,
    ANIM_GLOW
} TextAnimationType;

typedef struct {
    TextAnimationType type;
    float duration;
    float elapsed;
    float startValue;
    float endValue;
    float glowIntensity;
    SDL_Color glowColor;
} TextAnimation;

typedef struct {
    SDL_Point offset;
    SDL_Color color;
    SDL_Texture* texture;
    int width, height;
} ShadowConfig;

typedef struct {
    TTF_Font* font;
    SDL_Color color;
    SDL_Point position;
    int size;
    char fontPath[512];
    TextRenderMode renderMode;
    TextAlign alignment;
    SDL_Texture* cachedTexture;
    int cachedWidth;
    int cachedHeight;
    char* cachedText;
    ShadowConfig shadow;
    TextAnimation animation;
    int wrapWidth;
} TextRenderer;

int initText(TextRenderer* tr, const char* fontPath, int size);
void setColorText(TextRenderer* tr, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void setShadowConfig(TextRenderer* tr, int offsetX, int offsetY, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void setPositionText(TextRenderer* tr, int x, int y);
void setTextSize(TextRenderer* tr, int newSize);
void setTextAlignment(TextRenderer* tr, TextAlign align);
void setRenderMode(TextRenderer* tr, TextRenderMode mode);
void setTextWrap(TextRenderer* tr, int wrapWidth);
void setAnimation(TextRenderer* tr, TextAnimationType type, float duration, float startValue, float endValue);
void setGlowAnimation(TextRenderer* tr, float duration, float intensity, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void updateAnimation(TextRenderer* tr, float dt);
void cacheText(SDL_Renderer* renderer, TextRenderer* tr, const char* text);
void drawCachedText(SDL_Renderer* renderer, TextRenderer* tr);
void putText(SDL_Renderer* renderer, TextRenderer* tr, const char* text);
void putTextf(SDL_Renderer* renderer, TextRenderer* tr, const char* fmt, ...);
void putTextWithShadow(SDL_Renderer* renderer, TextRenderer* tr, const char* text);
void putOneChar(SDL_Renderer* renderer, TextRenderer* tr, char c);
void cleanupText(TextRenderer* tr);





#endif  




////////////////////////////////////////////
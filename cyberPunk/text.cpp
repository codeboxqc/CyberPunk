#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
 

extern void logMessage(const char* message);


#define PI 3.141592653589793238462643383279502884197169399375105820974944592

static int ttfInitialized = 0;

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

#ifdef _MSC_VER
#define STRNCPY_S(dest, dest_size, src, count) strncpy_s(dest, dest_size, src, count)
#define STRDUP _strdup
#else
#define STRNCPY_S(dest, dest_size, src, count) strncpy(dest, src, count)
#define STRDUP strdup
#endif

 
int initText(TextRenderer* tr, const char* fontPath, int size) {
    if (!ttfInitialized) {
        if (TTF_Init() == -1) {
            logMessage(TTF_GetError());
            return 0;
        }
        ttfInitialized = 1;
    }

    STRNCPY_S(tr->fontPath, sizeof(tr->fontPath), fontPath, sizeof(tr->fontPath) - 1);
    tr->fontPath[sizeof(tr->fontPath) - 1] = '\0';

    tr->font = TTF_OpenFont(fontPath, size);
    if (!tr->font) {
        logMessage(TTF_GetError());
        return 0;
    }

    tr->color.r = 255;
    tr->color.g = 255;
    tr->color.b = 255;
    tr->color.a = 255;

    tr->shadow.offset.x = 2;
    tr->shadow.offset.y = 2;
    tr->shadow.color.r = 0;
    tr->shadow.color.g = 0;
    tr->shadow.color.b = 0;
    tr->shadow.color.a = 255;
    tr->shadow.texture = NULL;
    tr->shadow.width = 0;
    tr->shadow.height = 0;

    tr->position.x = 0;
    tr->position.y = 0;
    tr->size = size;
    tr->alignment = ALIGN_LEFT;
    tr->renderMode = TEXT_RENDER_BLENDED;
    tr->cachedTexture = NULL;
    tr->cachedWidth = 0;
    tr->cachedHeight = 0;
    tr->cachedText = NULL;
    tr->animation.type = ANIM_NONE;
    tr->animation.duration = 0.0f;
    tr->animation.elapsed = 0.0f;
    tr->animation.startValue = 0.0f;
    tr->animation.endValue = 0.0f;
    tr->animation.glowIntensity = 0.0f;
    tr->animation.glowColor.r = 0;
    tr->animation.glowColor.g = 0;
    tr->animation.glowColor.b = 0;
    tr->animation.glowColor.a = 0;
    tr->wrapWidth = 0;

    return 1;
}

void setColorText(TextRenderer* tr, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    tr->color.r = r;
    tr->color.g = g;
    tr->color.b = b;
    tr->color.a = a;
}

void setShadowConfig(TextRenderer* tr, int offsetX, int offsetY, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    tr->shadow.offset.x = offsetX;
    tr->shadow.offset.y = offsetY;
    tr->shadow.color.r = r;
    tr->shadow.color.g = g;
    tr->shadow.color.b = b;
    tr->shadow.color.a = a;
    if (tr->shadow.texture) {
        SDL_DestroyTexture(tr->shadow.texture);
        tr->shadow.texture = NULL;
    }
}

void setPositionText(TextRenderer* tr, int x, int y) {
    tr->position.x = x;
    tr->position.y = y;
}

void setTextSize(TextRenderer* tr, int newSize) {
    if (newSize == tr->size) return;
    TTF_Font* oldFont = tr->font;
    tr->font = TTF_OpenFont(tr->fontPath, newSize);
    if (tr->font) {
        tr->size = newSize;
        if (oldFont) TTF_CloseFont(oldFont);
        if (tr->cachedTexture) {
            SDL_DestroyTexture(tr->cachedTexture);
            tr->cachedTexture = NULL;
        }
        if (tr->shadow.texture) {
            SDL_DestroyTexture(tr->shadow.texture);
            tr->shadow.texture = NULL;
        }
        if (tr->cachedText) {
            free(tr->cachedText);
            tr->cachedText = NULL;
        }
    }
    else {
        logMessage(TTF_GetError());
        tr->font = oldFont;
    }
}

void setTextAlignment(TextRenderer* tr, TextAlign align) {
    tr->alignment = align;
}

void setRenderMode(TextRenderer* tr, TextRenderMode mode) {
    tr->renderMode = mode;
    if (tr->cachedTexture) {
        SDL_DestroyTexture(tr->cachedTexture);
        tr->cachedTexture = NULL;
    }
    if (tr->shadow.texture) {
        SDL_DestroyTexture(tr->shadow.texture);
        tr->shadow.texture = NULL;
    }
}

void setTextWrap(TextRenderer* tr, int wrapWidth) {
    tr->wrapWidth = wrapWidth;
}

void setAnimation(TextRenderer* tr, TextAnimationType type, float duration, float startValue, float endValue) {
    tr->animation.type = type;
    tr->animation.duration = duration;
    tr->animation.elapsed = 0.0f;
    tr->animation.startValue = startValue;
    tr->animation.endValue = endValue;
    tr->animation.glowIntensity = 0.0f;
    tr->animation.glowColor.r = 0;
    tr->animation.glowColor.g = 0;
    tr->animation.glowColor.b = 0;
    tr->animation.glowColor.a = 0;
}

void setGlowAnimation(TextRenderer* tr, float duration, float intensity, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    tr->animation.type = ANIM_GLOW;
    tr->animation.duration = duration;
    tr->animation.elapsed = 0.0f;
    tr->animation.startValue = 0.0f;
    tr->animation.endValue = 1.0f;
    tr->animation.glowIntensity = intensity;
    tr->animation.glowColor.r = r;
    tr->animation.glowColor.g = g;
    tr->animation.glowColor.b = b;
    tr->animation.glowColor.a = a;
}

void updateAnimation(TextRenderer* tr, float dt) {
    if (tr->animation.type == ANIM_NONE) return;

    tr->animation.elapsed += dt;
    float t = tr->animation.elapsed / tr->animation.duration;
    if (t >= 1.0f) t = 1.0f;

    float value = tr->animation.startValue + (tr->animation.endValue - tr->animation.startValue) * t;

    float glow = 0.0f;
    switch (tr->animation.type) {
    case ANIM_FADE:
        setColorText(tr, tr->color.r, tr->color.g, tr->color.b, (Uint8)(value * 255.0f));
        break;
    case ANIM_SCALE:
        setTextSize(tr, (int)(tr->size * value));
        break;
    case ANIM_GLOW:
        glow = sinf(t * PI * 2.0f) * tr->animation.glowIntensity;
        setShadowConfig(tr, (int)(2 + glow * 2), (int)(2 + glow * 2),
            tr->animation.glowColor.r, tr->animation.glowColor.g,
            tr->animation.glowColor.b, tr->animation.glowColor.a);
        break;
    default:
        break;
    }

    if (t >= 1.0f && tr->animation.type != ANIM_GLOW) {
        tr->animation.type = ANIM_NONE;
    }
}

void cacheText(SDL_Renderer* renderer, TextRenderer* tr, const char* text) {
    if (!tr->font || !text) {
        logMessage("cacheText: Invalid font or text\n");
        return;
    }

    if (tr->cachedText && strcmp(tr->cachedText, text) == 0 && tr->cachedTexture) {
        return;
    }

    if (tr->cachedTexture) {
        SDL_DestroyTexture(tr->cachedTexture);
        tr->cachedTexture = NULL;
    }
    if (tr->shadow.texture) {
        SDL_DestroyTexture(tr->shadow.texture);
        tr->shadow.texture = NULL;
    }
    if (tr->cachedText) {
        free(tr->cachedText);
        tr->cachedText = NULL;
    }

    tr->cachedText = STRDUP(text);
    if (!tr->cachedText) {
        logMessage("Failed to allocate text buffer\n");
        return;
    }

    SDL_Surface* surface = NULL;
    switch (tr->renderMode) {
    case TEXT_RENDER_SOLID:
        surface = tr->wrapWidth ?
            TTF_RenderUTF8_Solid_Wrapped(tr->font, text, tr->color, tr->wrapWidth) :
            TTF_RenderUTF8_Solid(tr->font, text, tr->color);
        break;
    case TEXT_RENDER_BLENDED:
        surface = tr->wrapWidth ?
            TTF_RenderUTF8_Blended_Wrapped(tr->font, text, tr->color, tr->wrapWidth) :
            TTF_RenderUTF8_Blended(tr->font, text, tr->color);
        break;
    case TEXT_RENDER_SHADED:
        surface = tr->wrapWidth ?
            TTF_RenderUTF8_Shaded_Wrapped(tr->font, text, tr->color, tr->shadow.color, tr->wrapWidth) :
            TTF_RenderUTF8_Shaded(tr->font, text, tr->color, tr->shadow.color);
        break;
    }

    if (!surface) {
        logMessage(TTF_GetError());
        free(tr->cachedText);
        tr->cachedText = NULL;
        return;
    }

    tr->cachedTexture = SDL_CreateTextureFromSurface(renderer, surface);
    tr->cachedWidth = surface->w;
    tr->cachedHeight = surface->h;
    SDL_FreeSurface(surface);

    if (!tr->cachedTexture) {
        logMessage(SDL_GetError());
        free(tr->cachedText);
        tr->cachedText = NULL;
        return;
    }

    if (tr->shadow.offset.x != 0 || tr->shadow.offset.y != 0) {
        surface = tr->wrapWidth ?
            TTF_RenderUTF8_Blended_Wrapped(tr->font, text, tr->shadow.color, tr->wrapWidth) :
            TTF_RenderUTF8_Blended(tr->font, text, tr->shadow.color);
        if (surface) {
            tr->shadow.texture = SDL_CreateTextureFromSurface(renderer, surface);
            tr->shadow.width = surface->w;
            tr->shadow.height = surface->h;
            SDL_FreeSurface(surface);
            if (!tr->shadow.texture) {
                logMessage(SDL_GetError());
            }
        }
    }
}

void drawCachedText(SDL_Renderer* renderer, TextRenderer* tr) {
    if (!tr->cachedTexture) {
        logMessage("drawCachedText: No cached texture\n");
        return;
    }

    if (tr->shadow.texture) {
        SDL_Rect shadowDst = {
            tr->position.x + tr->shadow.offset.x,
            tr->position.y + tr->shadow.offset.y,
            tr->shadow.width,
            tr->shadow.height
        };
        if (tr->alignment == ALIGN_CENTER) {
            shadowDst.x -= tr->shadow.width / 2;
        }
        else if (tr->alignment == ALIGN_RIGHT) {
            shadowDst.x -= tr->shadow.width;
        }
        SDL_SetTextureAlphaMod(tr->shadow.texture, tr->shadow.color.a);
        SDL_RenderCopy(renderer, tr->shadow.texture, NULL, &shadowDst);
    }

    SDL_Rect dst = { tr->position.x, tr->position.y, tr->cachedWidth, tr->cachedHeight };
    if (tr->alignment == ALIGN_CENTER) {
        dst.x -= tr->cachedWidth / 2;
    }
    else if (tr->alignment == ALIGN_RIGHT) {
        dst.x -= tr->cachedWidth;
    }
    SDL_SetTextureAlphaMod(tr->cachedTexture, tr->color.a);
    SDL_RenderCopy(renderer, tr->cachedTexture, NULL, &dst);
}

void putText(SDL_Renderer* renderer, TextRenderer* tr, const char* text) {
    if (!tr->font || !text) {
        logMessage("putText: Invalid font or text\n");
        return;
    }
    cacheText(renderer, tr, text);
    drawCachedText(renderer, tr);
    logMessage("putText: Rendered\n");
}

void putTextf(SDL_Renderer* renderer, TextRenderer* tr, const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    putText(renderer, tr, buffer);
}

void putTextWithShadow(SDL_Renderer* renderer, TextRenderer* tr, const char* text) {
    putText(renderer, tr, text);
}

void putOneChar(SDL_Renderer* renderer, TextRenderer* tr, char c) {
    char str[2] = { c, '\0' };
    putText(renderer, tr, str);
}

void cleanupText(TextRenderer* tr) {
    if (tr->font) {
        TTF_CloseFont(tr->font);
        tr->font = NULL;
    }
    if (tr->cachedTexture) {
        SDL_DestroyTexture(tr->cachedTexture);
        tr->cachedTexture = NULL;
    }
    if (tr->shadow.texture) {
        SDL_DestroyTexture(tr->shadow.texture);
        tr->shadow.texture = NULL;
    }
    if (tr->cachedText) {
        free(tr->cachedText);
        tr->cachedText = NULL;
    }
}





TextRenderer tr;

int testini()
{
    // Initialize TextRenderer
#ifdef _MSC_VER
    const char* fontPath = "C:\\Windows\\Fonts\\Arial.ttf";
#else
    const char* fontPath = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
#endif
    if (!initText(&tr, fontPath, 80)) {
        logMessage("Text init failed\n");
        return 0;
    }

    // Configure text
    setPositionText(&tr, 960, 540); // Center at 1920x1080
    setTextAlignment(&tr, ALIGN_CENTER);
    setColorText(&tr, 255, 255, 255, 255); // White text
    setGlowAnimation(&tr, 2.0f, 1.5f, 255, 0, 0, 128); // Red glow
    setTextWrap(&tr, 1000); // Wrap long text


}

void example4(SDL_Renderer* renderer, float dt)
{
    updateAnimation(&tr,  dt);
    setGlowAnimation(&tr, 2.0f, 1.5f, 255, 222, 0, 128); // Red glo
    setColorText(&tr, 255, 0, 0, 100);
    putText(renderer, &tr, "CyberPunk Text");
   
}


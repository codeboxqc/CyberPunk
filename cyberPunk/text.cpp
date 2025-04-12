#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>

extern void logMessage(const char* message);

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

typedef struct {
    TTF_Font* font;
    SDL_Color color;
    SDL_Color shadowColor;
    SDL_Point position;
    int size;
    char fontPath[512];  // store path to reload with new size
    TextRenderMode renderMode;
    TextAlign alignment;
    SDL_Texture* cachedTexture;  // Cached texture for efficiency
    int cachedWidth;
    int cachedHeight;
    const char* cachedText;  // Track the text currently cached
} TextRenderer;

// Function declarations
int initText(TextRenderer* tr, const char* fontPath, int size);
void setColorText(TextRenderer* tr, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void setPositionText(TextRenderer* tr, int x, int y);
void setTextSize(TextRenderer* tr, int newSize);
void setTextAlignment(TextRenderer* tr, TextAlign align);
void setRenderMode(TextRenderer* tr, TextRenderMode mode);
void cacheText(SDL_Renderer* renderer, TextRenderer* tr, const char* text);
void drawCachedText(SDL_Renderer* renderer, TextRenderer* tr);
void putText(SDL_Renderer* renderer, TextRenderer* tr, const char* text);
void putTextWithShadow(SDL_Renderer* renderer, TextRenderer* tr, const char* text);
void putOneChar(SDL_Renderer* renderer, TextRenderer* tr, char c);
void cleanupText(TextRenderer* tr);

// Function definitions

int initText(TextRenderer* tr, const char* fontPath, int size) {
    if (TTF_Init() == -1) {
        logMessage(TTF_GetError());
        return 0;
    }

    strncpy_s(tr->fontPath, sizeof(tr->fontPath), fontPath, _TRUNCATE);
    //strncpy(tr->fontPath, fontPath, sizeof(tr->fontPath));
    tr->fontPath[sizeof(tr->fontPath) - 1] = '\0';

    tr->font = TTF_OpenFont(fontPath, size);
    if (!tr->font) {
        logMessage(TTF_GetError());
        return 0;
    }

  
    tr->color.r = 255;
    tr->color.g = 255;
    tr->color.b = 255;
    tr->color.a = 255;  // default white

    tr->shadowColor.r = 0;
    tr->shadowColor.g = 0;
    tr->shadowColor.b = 0;
    tr->shadowColor.a = 255;  // default black for shadow

    tr->position.x = 0;
    tr->position.y = 0;  // position set to (0, 0)



    tr->size = size;
    tr->alignment = ALIGN_LEFT;
    tr->renderMode = TEXT_RENDER_BLENDED;
    tr->cachedTexture = NULL;
    tr->cachedWidth = 0;
    tr->cachedHeight = 0;
    tr->cachedText = "";

    return 1;
}

void setColorText(TextRenderer* tr, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    tr->color.r = r;
    tr->color.g = g;
    tr->color.b = b;
    tr->color.a = a;
}

void setPositionText(TextRenderer* tr, int x, int y) {
    tr->position.x = x;
    tr->position.y = y;
}

void setTextSize(TextRenderer* tr, int newSize) {
    if (tr->font) TTF_CloseFont(tr->font);
    tr->font = TTF_OpenFont(tr->fontPath, newSize);
    if (tr->font) {
        tr->size = newSize;
    }
    else {
        logMessage(TTF_GetError());
    }
}

void setTextAlignment(TextRenderer* tr, TextAlign align) {
    tr->alignment = align;
}

void setRenderMode(TextRenderer* tr, TextRenderMode mode) {
    tr->renderMode = mode;
}

void cacheText(SDL_Renderer* renderer, TextRenderer* tr, const char* text) {
    if (tr->cachedText && strcmp(tr->cachedText, text) == 0) {
        return;  // No need to cache the same text again
    }

    SDL_Surface* surface = NULL;
    switch (tr->renderMode) {
    case TEXT_RENDER_SOLID:
        surface = TTF_RenderUTF8_Solid(tr->font, text, tr->color);
        break;
    case TEXT_RENDER_BLENDED:
        surface = TTF_RenderUTF8_Blended(tr->font, text, tr->color);
        break;
    case TEXT_RENDER_SHADED:
        surface = TTF_RenderUTF8_Shaded(tr->font, text, tr->color, tr->shadowColor);
        break;
    }

    if (surface) {
        if (tr->cachedTexture) {
            SDL_DestroyTexture(tr->cachedTexture);
        }
        tr->cachedTexture = SDL_CreateTextureFromSurface(renderer, surface);
        tr->cachedWidth = surface->w;
        tr->cachedHeight = surface->h;
        tr->cachedText = text;
        SDL_FreeSurface(surface);
    }
}

void drawCachedText(SDL_Renderer* renderer, TextRenderer* tr) {
    if (tr->cachedTexture) {
        SDL_Rect dst = { tr->position.x, tr->position.y, tr->cachedWidth, tr->cachedHeight };

        // Adjust position based on alignment
        if (tr->alignment == ALIGN_CENTER) {
            dst.x -= tr->cachedWidth / 2;
        }
        else if (tr->alignment == ALIGN_RIGHT) {
            dst.x -= tr->cachedWidth;
        }

        SDL_RenderCopy(renderer, tr->cachedTexture, NULL, &dst);
    }
}

void putText(SDL_Renderer* renderer, TextRenderer* tr, const char* text) {
    if (!tr->font || !text) return;

    cacheText(renderer, tr, text);  // Cache the text to avoid recreating it each time
    drawCachedText(renderer, tr);   // Draw the cached texture
}

void putTextWithShadow(SDL_Renderer* renderer, TextRenderer* tr, const char* text) {
    SDL_Point originalPosition = tr->position;

    // Draw shadow first (slightly offset)
    tr->position.x += 2;  // X offset for shadow
    tr->position.y += 2;  // Y offset for shadow
    setColorText(tr, tr->shadowColor.r, tr->shadowColor.g, tr->shadowColor.b, tr->shadowColor.a);
    putText(renderer, tr, text);

    // Restore position and color
    tr->position = originalPosition;
    setColorText(tr, 255, 255, 255, 255);  // Default white for text

    // Draw main text
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
    TTF_Quit();
}

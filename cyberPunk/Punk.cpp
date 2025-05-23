#define SDL_MAIN_HANDLED
//#include <SDL2/SDL.h>

//#include <SDL2/SDL_Main.h>
//#include <SDL2/SDL_image.h>
//#include <SDL2/SDL_mixer.h>
//#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <cstdlib>

//#pragma comment(lib,"SDL2main.lib")

#include "punk.h"
#include "fire2.h" // Include the new fire effect header


int X = 1920, Y = 1080;


 
/// //////////////////////////////////////////////////
 
void example1(int x, int y, LSD* gfx, float dt);
void ini1(int x, int y);
void example2(LSD* gfx, float dt);
void ini2(int x, int y);
void example3(LSD* gfx, float dt);
void init3(int x, int y);

 
void star3d(LSD* ssgfx);
void example5ini(LSD* ssgfx);
/////////////////////////////////////////////

int testini();
void example4(SDL_Renderer* renderer, float dt);

void plasma2ini(LSD *iii);
void plasma2(LSD* iii, int offset);

int asmini();
void asmgo(LSD* iii);
/////////////////////////////////////////////////////////////////////////


LSD gfx;
int plasma_offset = 0;
bool fire2Initialized = false; // Flag for FireEffect2 initialization

int loop() {
    
    int effect_mode = 0; // 0 = old flame, 1 = old plasma, 2 = plasma2, 3 = star3d, 4 = asmplasma, 5 = fire2
    int num_effects = 6; // Total number of effects including the new fire2

    // Track initialization state for effects that need it.
    // For simplicity, we'll re-initialize if switched back, or assume one-time init is fine for these demos.
    // A more robust system would track init state for each and clean up/re-init.
    // For this integration, fire2Initialized handles FireEffect2 specifically.

    if (!init(&gfx, renderer, X, Y,0)) {
        logMessage("Failed to init LSD Engine\n");
        return 1;
    }

    // Load a sprite (replace "sprite.png" with your PNG file path)
   // Sprite* sprite = load_sprite(renderer, "sprite.png");
    
    Uint32 lastTime = SDL_GetTicks();
    float dt=0;
    float dtext = 0;

    
    bool running = true;
    SDL_Event event;

    logMessage("loop.\n");

    // Simple loop to test window
    while (running) {

        Uint32 currentTime = SDL_GetTicks();
        dt = (currentTime - lastTime) / 1000.0f; // Update dt
        lastTime = currentTime;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = false;
                logMessage("exit loop.\n");
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
                if (event.key.keysym.sym == SDLK_SPACE) {
                    effect_mode = (effect_mode + 1) % num_effects;
                    if (effect_mode != 5) { // If switching away from FireEffect2
                        if (fire2Initialized) {
                            FireEffect2::closeFire2();
                            fire2Initialized = false;
                            logMessage("Closed Fire Effect 2.");
                        }
                    }
                }
            }
        }

        // Begin frame: clear screen (optional) and set up for drawing
        begin_frame(&gfx,0);

        // Clear to black - This is a brute-force clear. 
        // `clear(&gfx, 0,0,0,255)` or `memset` in `begin_frame` is more efficient.
        // Assuming begin_frame with methode 0 handles clearing.
        // If not, this loop is very slow. For now, keeping as is.
        // The current begin_frame(..., 0) does a memset. So this loop is redundant.
        // I will remove this redundant clear loop.
        /*
        for (int y = 0; y < gfx.screenHeight; y++) {
            for (int x = 0; x < gfx.screenWidth; x++) {
                pixel(&gfx, x, y, 0, 0, 0, 255);
            }
        }
        */

        // Run selected effect
        if (effect_mode == 0) {
             example1(960, 900, &gfx, dt); // Flame near bottom-center
        }
        else if (effect_mode == 1) {
            example2(&gfx, dt);
        }
        else if (effect_mode == 2) {
            plasma2(&gfx, plasma_offset);
            plasma_offset = (plasma_offset + 1) % 256;
        }
        else if (effect_mode == 3) {
            star3d(&gfx);
        }
        else if (effect_mode == 4) {
            asmgo(&gfx);
        }
        else if (effect_mode == 5) { // New FireEffect2
            if (!fire2Initialized) {
                if (!FireEffect2::initFire2(&gfx, X, Y)) { // Use global X, Y for screen dimensions
                    logMessage("Failed to initialize Fire Effect 2");
                    running = false; // Exit loop on critical init failure
                } else {
                    fire2Initialized = true;
                    logMessage("Fire Effect 2 Initialized.");
                }
            }
            if (fire2Initialized) { // Only run if initialized
                FireEffect2::updateFire2();
                FireEffect2::renderFire2(&gfx);
            }
        }

        // SDL_Delay(16); // ~60 FPS - This can be problematic with variable workloads.
        // The dt calculation at the start of the loop is better for smooth updates if effects use dt.
        // For fixed updates like some of these demos, it might be okay.
        end_frame(&gfx, 0); // This updates texture from gfx.pix and renders

        // Present the renderer (flip the buffers)
        // SDL_RenderClear(renderer); //This is done in end_frame or should be.
        // SDL_RenderCopy(renderer, gfx.screenTexture, NULL, NULL); // Also done in end_frame for methode 0
        // example4(renderer, dtext);  //text - This draws directly to renderer, after gfx is copied.
        SDL_RenderPresent(renderer); // Final present
    }

    // Cleanup for FireEffect2 if it was active
    if (fire2Initialized) {
        FireEffect2::closeFire2();
        fire2Initialized = false;
        logMessage("Closed Fire Effect 2 on exit.");
    }
    return 0;
}

 


int main(int argc, char* argv[])
{
    std::cout << "Hello World!\n";
    logMessage("CyberPunk Hello World!\n");

    if (!iniSDL("CyberPunk", X, Y)) {
        // std::cerr << "Failed to initialize SDL.\n";
        logMessage("Failed to initialize SDL.\n");
        return 1;
    }
     
    logMessage("begin loop.\n");
    
    // Initialize other effects
    ini1(X,Y); //flame
    ini2(960, 540);//plasma
    testini(); //text
    plasma2ini(&gfx); // This takes &gfx, implies it might use screen buffer directly or for info
    asmini();
    example5ini(&gfx); //3d, also takes &gfx
      
    int result = loop();
    logMessage("end loop.\n");

    // General cleanup already present
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;

}


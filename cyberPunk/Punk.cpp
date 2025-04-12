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


int X = 1920, Y = 1080;


 
/// //////////////////////////////////////////////////
 
void example1(int x, int y, LSD* gfx, float dt);
void ini1(int x, int y);
void example2(LSD* gfx, float dt);
void ini2(int x, int y);
void example3(LSD* gfx, float dt);
void init3(int x, int y);

int testini();
void example4(SDL_Renderer* renderer, float dt);
/////////////////////////////////////////////////////////////////////////

int loop() {
    LSD gfx;
    int effect_mode = 0; // 0 = flame, 1 = plasma
    bool space_pressed = false; // Track spacebar state


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
                    effect_mode = (effect_mode + 1) % 3;
                   
                }
                 
            }

        }

        // Begin frame: clear screen (optional) and set up for drawing
        begin_frame(&gfx,0);

        // Clear to black
        for (int y = 0; y < gfx.screenHeight; y++) {
            for (int x = 0; x < gfx.screenWidth; x++) {
                pixel(&gfx, x, y, 0, 0, 0, 255);
            }
        }

        // Run selected effect
        if (effect_mode == 0) {
            example1(960, 900, &gfx, dt); // Flame near bottom-center
           
        }
        if (effect_mode == 1) {
            example2(&gfx, dt);
            
        }

        if (effect_mode == 2) {
            example3(&gfx, dt);

        }

        


        SDL_Delay(16); // ~60 FPS
        end_frame(&gfx, 0);

       

        // Present the renderer (flip the buffers)
       // Copy texture to renderer and present
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, gfx.screenTexture, NULL, NULL);


        example4(renderer, dtext);  //text


        SDL_RenderPresent(renderer);
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
    
    ini1(X,Y); //flame
    ini2(960, 540);//plasma
    init3(960, 540);//fluid
    testini(); //text
      
     

    int result = loop();
    logMessage("end loop.\n");

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;

}


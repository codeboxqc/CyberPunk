#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>  // For memset
#include <SDL.h>
#include <stdio.h>

#include <SDL.h>
#include <stdio.h>
#include <string.h>


extern void logMessage(const char* message);

#define JOYSTICK_DEADZONE 8000
#define DOUBLE_TAP_TIME 300

typedef struct {
    int up, down, left, right;
    int escape, ctrl, space, alt, enter;
    int mouseLeft, mouseRight;
    int mouseX, mouseY;
    int sprint, dash;  // Sprint/Dash mechanics
    int scrollUp, scrollDown;  // Mouse wheel
    int specialMove; // Gesture-based input detection
} InputState;

static SDL_Joystick* joystick = NULL;
static Uint32 lastPressTime = 0;

// Initialize input system (including joystick)
int initInput(InputState* input) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC) < 0) {
        logMessage("SDL init failed: %s\n" );
        return 0;
    }

    if (SDL_NumJoysticks() > 0) {
        joystick = SDL_JoystickOpen(0);
        if (!joystick) {
            logMessage("Failed to open joystick: %s\n" );
        }
    }

    memset(input, 0, sizeof(InputState));
    return 1;
}

void processInput(InputState* input) {
    SDL_Event event;
    Uint32 currentTime = SDL_GetTicks();

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            input->escape = 1;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_UP:    input->up = 1; break;
            case SDLK_DOWN:  input->down = 1; break;
            case SDLK_LEFT:  input->left = 1; break;
            case SDLK_RIGHT: input->right = 1; break;
            case SDLK_ESCAPE: input->escape = 1; break;
            case SDLK_LCTRL: case SDLK_RCTRL: input->ctrl = 1; break;
            case SDLK_SPACE: input->space = 1; break;
            case SDLK_LALT: case SDLK_RALT: input->alt = 1; break;
            case SDLK_RETURN: input->enter = 1; break;
            }
            // Sprint detection (hold for longer time)
            if (event.key.keysym.sym == SDLK_RIGHT && currentTime - lastPressTime < DOUBLE_TAP_TIME) {
                input->sprint = 1;
            }
            lastPressTime = currentTime;
            break;

        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
            case SDLK_UP:    input->up = 0; break;
            case SDLK_DOWN:  input->down = 0; break;
            case SDLK_LEFT:  input->left = 0; break;
            case SDLK_RIGHT: input->right = 0; break;
            case SDLK_ESCAPE: input->escape = 0; break;
            case SDLK_LCTRL: case SDLK_RCTRL: input->ctrl = 0; break;
            case SDLK_SPACE: input->space = 0; break;
            case SDLK_LALT: case SDLK_RALT: input->alt = 0; break;
            case SDLK_RETURN: input->enter = 0; break;
            }
            input->sprint = 0;
            break;

        case SDL_JOYAXISMOTION:
            if (event.jaxis.axis == 0) {
                input->left = (event.jaxis.value < -JOYSTICK_DEADZONE);
                input->right = (event.jaxis.value > JOYSTICK_DEADZONE);
            }
            else if (event.jaxis.axis == 1) {
                input->up = (event.jaxis.value < -JOYSTICK_DEADZONE);
                input->down = (event.jaxis.value > JOYSTICK_DEADZONE);
            }
            break;

        case SDL_JOYBUTTONDOWN:
            switch (event.jbutton.button) {
            case 0: input->space = 1; break;
            case 1: input->ctrl = 1; break;
            case 2: input->alt = 1; break;
            case 9: input->escape = 1; break;
            case 10: input->enter = 1; break;
            }
            break;

        case SDL_JOYBUTTONUP:
            switch (event.jbutton.button) {
            case 0: input->space = 0; break;
            case 1: input->ctrl = 0; break;
            case 2: input->alt = 0; break;
            case 9: input->escape = 0; break;
            case 10: input->enter = 0; break;
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            switch (event.button.button) {
            case SDL_BUTTON_LEFT: input->mouseLeft = 1; break;
            case SDL_BUTTON_RIGHT: input->mouseRight = 1; break;
            }
            break;

        case SDL_MOUSEBUTTONUP:
            switch (event.button.button) {
            case SDL_BUTTON_LEFT: input->mouseLeft = 0; break;
            case SDL_BUTTON_RIGHT: input->mouseRight = 0; break;
            }
            break;

        case SDL_MOUSEMOTION:
            input->mouseX = event.motion.x;
            input->mouseY = event.motion.y;
            break;

        case SDL_MOUSEWHEEL:
            input->scrollUp = (event.wheel.y > 0);
            input->scrollDown = (event.wheel.y < 0);
            break;
        }
    }
}

void cleanupInput(void) {
    if (joystick) {
        SDL_JoystickClose(joystick);
        joystick = NULL;
    }
    SDL_Quit();
}

int keytest() {


    InputState input = { 0 };
    if (!initInput(&input)) {
        logMessage("error key mouse input init\n");
            return 1;
    }

   
    int running = 1;
    while (running) {
        processInput(&input);
        running = !input.escape;

      
        if (input.up) printf("Up Pressed\n");
        if (input.down) printf("Down Pressed\n");
        if (input.left) printf("Left Pressed\n");
        if (input.right) printf("Right Pressed\n");
        if (input.space) printf("Space Pressed\n");
        if (input.ctrl) printf("Ctrl Pressed\n");
        if (input.alt) printf("Alt Pressed\n");
        if (input.enter) printf("Enter Pressed\n");
        if (input.sprint) printf("Sprint Activated\n");
        if (input.scrollUp) printf("Mouse Wheel Up\n");
        if (input.scrollDown) printf("Mouse Wheel Down\n");
 
        SDL_Delay(16);
    }

    cleanupInput();
 
    return 0;
}

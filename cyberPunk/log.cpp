#include <stdio.h>
#include <SDL2/SDL.h>


void logMessage(const char* message) {
    FILE* logFile;

#ifdef _WIN32
    errno_t err = fopen_s(&logFile, "application.log", "a");
    if (err != 0) {
        fprintf(stderr, "Could not open log file for writing: %s\n", SDL_GetError());
        return;
    }
#else
    logFile = fopen("application.log", "a");
    if (logFile == NULL) {
        fprintf(stderr, "Could not open log file for writing: %s\n", SDL_GetError());
        return;
    }
#endif

    fprintf(logFile, "%s\n", message);
    fclose(logFile);

}
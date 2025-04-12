#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL.h>


extern void logMessage(const char* message);

const int audiox = 20;

typedef struct {
    Mix_Chunk* soundEffects[audiox];
    Mix_Music* bgMusic;
} AudioManager;

int initAudio(AudioManager* audio) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) return 0;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return 0;
    memset(audio, 0, sizeof(AudioManager));
    return 1;
}

void loadSound(AudioManager* audio, int index, const char* path) {

    if (index >= audiox) {
        logMessage("max sound load\n");
        return;
    }
    audio->soundEffects[index] = Mix_LoadWAV(path);
    if (!audio->soundEffects[index]) {
        logMessage("Failed to load sound: %s! Mix_Error\n");
    }
}

void loadMusic(AudioManager* audio, const char* path) {
    audio->bgMusic = Mix_LoadMUS(path);
    if (!audio->bgMusic) {
        logMessage("Failed to load music: %s! Mix_Error");
    }
}

int playSound(AudioManager* audio, int index) {
    if (audio->soundEffects[index]) {
        return Mix_PlayChannel(-1, audio->soundEffects[index], 0);
    }
    return -1;
}

void playMusic(AudioManager* audio) {
    if (audio->bgMusic) {
        Mix_PlayMusic(audio->bgMusic, -1);
    }
}

void cleanupAudio(AudioManager* audio) {
    for (int i = 0; i < 10; i++) {
        if (audio->soundEffects[i]) Mix_FreeChunk(audio->soundEffects[i]);
    }
    if (audio->bgMusic) Mix_FreeMusic(audio->bgMusic);
    Mix_CloseAudio();
    SDL_Quit();
}


void setSoundVolume(AudioManager* audio, int index, int volume) {
    if (index < audiox && audio->soundEffects[index]) {
        Mix_VolumeChunk(audio->soundEffects[index], volume); // 0–128
    }
}

void setMusicVolume(AudioManager* audio, int volume) {
    if (audio->bgMusic) {
        Mix_VolumeMusic(volume); // 0–128
    }
}

void pauseMusic(AudioManager* audio) {
    if (audio->bgMusic && Mix_PlayingMusic()) {
        Mix_PauseMusic();
    }
}

void resumeMusic(AudioManager* audio) {
    if (audio->bgMusic && Mix_PausedMusic()) {
        Mix_ResumeMusic();
    }
}

void stopMusic(AudioManager* audio) {
    if (audio->bgMusic) {
        Mix_HaltMusic();
    }
}


void fadeInMusic(AudioManager* audio, int fadeTimeMs) {
    if (audio->bgMusic) {
        Mix_FadeInMusic(audio->bgMusic, -1, fadeTimeMs); // Fade in over fadeTimeMs milliseconds
    }
}

void fadeOutMusic(AudioManager* audio, int fadeTimeMs) {
    if (audio->bgMusic && Mix_PlayingMusic()) {
        Mix_FadeOutMusic(fadeTimeMs); // Fade out over fadeTimeMs milliseconds
    }
}

int playSoundLooped(AudioManager* audio, int index, int loops) {
    if (index < audiox && audio->soundEffects[index]) {
        return Mix_PlayChannel(-1, audio->soundEffects[index], loops); // loops: -1 for infinite, 0 for once, etc.
    }
    return -1;
}

void stopSound(int channel) {
    if (channel >= 0) {
        Mix_HaltChannel(channel);
    }
}


int playSoundPanned(AudioManager* audio, int index, int pan) {
    if (index < audiox && audio->soundEffects[index]) {
        int channel = Mix_PlayChannel(-1, audio->soundEffects[index], 0);
        if (channel >= 0) {
            Mix_SetPanning(channel, (pan < 0 ? 255 + pan : 255), (pan > 0 ? 255 - pan : 255)); // -255 (left) to 255 (right)
        }
        return channel;
    }
    return -1;
}


int playSoundDistance(AudioManager* audio, int index, float distance, float maxDistance) {
    if (index < audiox && audio->soundEffects[index]) {
        int channel = Mix_PlayChannel(-1, audio->soundEffects[index], 0);
        if (channel >= 0) {
            int volume = (int)(MIX_MAX_VOLUME * (1.0f - distance / maxDistance));
            if (volume < 0) volume = 0;
            Mix_Volume(channel, volume);
        }
        return channel;
    }
    return -1;
}




void testaudio() {

    AudioManager audio;
    if (!initAudio(&audio)) {

        logMessage("Failed initAudio\n");
        return ;
    }

    loadSound(&audio, 0, "shoot.wav");
    loadMusic(&audio, "background.mp3");


    setSoundVolume(&audio, 0, 64);  // 50% volume for shoot
    setMusicVolume(&audio, 32);     // 25% volume for music
    playMusic(&audio);
    fadeInMusic(&audio, 2000);      // Fade in music over 2 seconds

    playSound(&audio, 0);
   

 
    
        SDL_Delay(1000);
  

    cleanupAudio(&audio);

}
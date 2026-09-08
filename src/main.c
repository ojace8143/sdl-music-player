#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <stdbool.h>
#include <stdio.h>
#include "config.h"

int main(void)
{
    // Variables
    bool running = true;
    SDL_Event event;


    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    if (!MIX_Init()) {
        SDL_Log("MIX_Init failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    MIX_Mixer *mixer = MIX_CreateMixerDevice(
            SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
            NULL
    ); // Create audio mixer

    // Check if mixer isn't working
    if (mixer == NULL) {
        SDL_Log("MIX_CreateMixerDevice failed: %s", SDL_GetError());
        MIX_Quit();
        SDL_Quit();
        return 1;
    }

    
    // Creates a window
    SDL_Window *window = SDL_CreateWindow(
        "ojace8143's music player",
        800,
        600,
        0
    );

    MIX_Track *button_track = MIX_CreateTrack(mixer);

    // creates renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    // Checks if the renderer failed
    if (renderer == NULL) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }


    // Check button track error yada yada of course 
    if (button_track == NULL) {
        SDL_Log("MIX_CreateTrack failed: %s", SDL_GetError());
        MIX_DestroyAudio(button_sound);
        Mix_DestroyMixer(mixer);
        MIX_Quit();
        SDL_Quit();
        return 1;
    }

    // Checks if the window failed or not
    if (window == NULL) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Define a button "button"
    SDL_FRect button = {
        .x = 300,
        .y = 250,
        .w = 200,
        .h = 100
    };

    // Sets window title
    SDL_SetWindowTitle(window, "ojace8143's music player");

    MIX_Audio *button_sound = MIX_LoadAudio(mixer, "assets/button.mp3", true); // set button sound to be the goofy noises

    while (running) {

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Sets color R,G,B,A
        SDL_RenderClear(renderer); // Clears the "renderer"

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set color again
        SDL_RenderFillRect(renderer, &button); // Draw the button, takes the pointer to the button created earlier

        SDL_RenderPresent(renderer);

        // Event loop
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                printf("a key was pressed wow so impressive\n");
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }

                if (event.key.key == SDLK_SPACE) {
                    printf("you pressed space nice job lil bro\n");
                }

            }

            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                float x = event.button.x; // Takes the x position of the "event"
                float y = event.button.y;  // Takes the y position of the "event"
                
                if ( x >= button.x &&                  // if block checks if the cursor is in the button's width or height, if so then printf
                     x <= button.x + button.w &&
                     y >= button.y &&
                     y <= button.y + button.h) {
                    printf("you clicked the button nice job\n");
                }
            } 
        }
    }

    MIX_Quit();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;

}

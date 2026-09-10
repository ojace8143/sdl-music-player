#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <stdbool.h>
#include <stdio.h>
#include "../config.h"

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

    // Create audio mixer thing
    MIX_Mixer *mixer = MIX_CreateMixerDevice(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
        NULL
    );

    // Check if mixer isn't working
    if (mixer == NULL) {
        SDL_Log("MIX_CreateMixerDevice failed: %s", SDL_GetError());
        MIX_Quit();
        SDL_Quit();
        return 1;
    }

    // Create a window
    SDL_Window *window = SDL_CreateWindow(
        "ojace8143's music player",
        600,
        200,
        0
    );

    // Check if the window failed
    if (window == NULL) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        MIX_DestroyMixer(mixer);
        MIX_Quit();
        SDL_Quit();
        return 1;
    }

    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    // Check if the renderer failed
    if (renderer == NULL) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        MIX_DestroyMixer(mixer);
        MIX_Quit();
        SDL_Quit();
        return 1;
    }

    // Create a track for the button
    MIX_Track *button_track = MIX_CreateTrack(mixer);

    // Check button track error
    if (button_track == NULL) {
        SDL_Log("MIX_CreateTrack failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        MIX_DestroyMixer(mixer);
        MIX_Quit();
        SDL_Quit();
        return 1;
    }

    // Load the goofy button sound
    MIX_Audio *button_sound = MIX_LoadAudio(
        mixer,
        "assets/goofy-sound-effects.mp3",
        true
    );

    // Check if the button sound loaded
    if (button_sound == NULL) {
        SDL_Log("MIX_LoadAudio failed: %s", SDL_GetError());
        MIX_DestroyTrack(button_track);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        MIX_DestroyMixer(mixer);
        MIX_Quit();
        SDL_Quit();
        return 1;
    }

    // Connect the sound to the track
    if (!MIX_SetTrackAudio(button_track, button_sound)) {
        SDL_Log("MIX_SetTrackAudio failed: %s", SDL_GetError());
        MIX_DestroyAudio(button_sound);
        MIX_DestroyTrack(button_track);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        MIX_DestroyMixer(mixer);
        MIX_Quit();
        SDL_Quit();
        return 1;
    }


    int button_center = 150; // y value that buttons will be centered on

    // Define a button
    // Screen is 800x200 (note)
    SDL_FRect play_pause_button = {
        .x = (600 - 50) / 2,
        .y = button_center - 50 / 2,
        .w = 50,
        .h = 50
    };

    SDL_FRect next_button = {
        .x = play_pause_button.x + play_pause_button.w + 10, 
        .y = button_center - 35 / 2,
        .w = 35,
        .h = 35
    };

    SDL_FRect previous_button = {
        .x = play_pause_button.x - 35 - 10,
        .y = button_center - 35 / 2,
        .w = 35,
        .h = 35
    };
    // Sets window title
    SDL_SetWindowTitle(window, "ojace8143's music player");

    // Main loop
    while (running) {

        // Set background to black
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Set button to white
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &play_pause_button);
        SDL_RenderFillRect(renderer, &next_button);
        SDL_RenderFillRect(renderer, &previous_button);

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
                float x = event.button.x;
                float y = event.button.y;

                // Check if the cursor is inside the button
                if (x >= play_pause_button.x &&
                    x <= play_pause_button.x + play_pause_button.w &&
                    y >= play_pause_button.y &&
                    y <= play_pause_button.y + play_pause_button.h) {

                    printf("you clicked the play pause button nice job\n");

                    // Play the goofy sound
                    // will soon be to toggle current sound
                    MIX_PlayTrack(button_track, 0);
                }
                if (x >= next_button.x &&
                    x <= next_button.x + next_button.w &&
                    y >= next_button.y &&
                    y >= next_button.y + next_button.h) {

                    printf("you clicked on the next button nice job\n");
                }
                if (x >= previous_button.x &&
                    x <= previous_button.x + previous_button.w &&
                    y >= previous_button.y &&
                    y <= previous_button.y + previous_button.h) {

                    printf("you clicked on the previous button nice job\n");
                }
            }
        }
    }

    // Cleanup
    MIX_DestroyTrack(button_track);
    MIX_DestroyAudio(button_sound);
    MIX_DestroyMixer(mixer);
    MIX_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

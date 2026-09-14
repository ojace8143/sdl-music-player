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

    bool playing = false; // Is the song playing?

    const char *queue[] = {
        "/home/ojace8143/media/music/Tool-10,000_Days/01.Vicarious.ogg",
        "/home/ojace8143/media/music/Tool-10,000_Days/02.Jambi.ogg"
    };

    int queue_index = 0; // Queue position

    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    // Initialize SDL_mixer
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

    // Create a track for the current song
    MIX_Track *track = MIX_CreateTrack(mixer);

    // Check track error
    if (track == NULL) {
        SDL_Log("MIX_CreateTrack failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        MIX_DestroyMixer(mixer);
        MIX_Quit();
        SDL_Quit();
        return 1;
    }

    // Load the current song
    MIX_Audio *track_audio = MIX_LoadAudio(
        mixer,
        queue[queue_index],
        true
    );

    // Check if the track loaded
    if (track_audio == NULL) {
        SDL_Log("MIX_LoadAudio failed: %s", SDL_GetError());
        printf("Track not found!\n");
        MIX_DestroyTrack(track);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        MIX_DestroyMixer(mixer);
        MIX_Quit();
        SDL_Quit();
        return 1;
    }

    // Connect the audio to the track
    if (!MIX_SetTrackAudio(track, track_audio)) {
        SDL_Log("MIX_SetTrackAudio failed: %s", SDL_GetError());
        MIX_DestroyAudio(track_audio);
        MIX_DestroyTrack(track);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        MIX_DestroyMixer(mixer);
        MIX_Quit();
        SDL_Quit();
        return 1;
    }

    // Y value that buttons will be centered on
    int button_center = 150;

    // Define a button
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

        // Set buttons to white
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

                // Play / pause button
                if (x >= play_pause_button.x &&
                    x <= play_pause_button.x + play_pause_button.w &&
                    y >= play_pause_button.y &&
                    y <= play_pause_button.y + play_pause_button.h) {

                    printf("you clicked the play pause button nice job\n");

                    MIX_PlayTrack(track, 0);
                }

                // Next button
                if (x >= next_button.x &&
                    x <= next_button.x + next_button.w &&
                    y >= next_button.y &&
                    y <= next_button.y + next_button.h) {

                    printf("you clicked on the next button nice job\n");
                }

                // Previous button
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
    MIX_DestroyTrack(track);
    MIX_DestroyAudio(track_audio);
    MIX_DestroyMixer(mixer);
    MIX_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

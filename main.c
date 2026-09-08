#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdio.h>

int main(void)
{
    // Variables
    bool running = true;
    SDL_Event event;


    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    // Creates a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    // Creates a window
    SDL_Window *window = SDL_CreateWindow(
        "ojace8143's music player",
        800,
        600,
        0
    );

    // Checks if the renderer failed
    if (renderer == NULL) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
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

    while (running) {

        // Clears the "renderer"
        SDL_SetRenderClear(renderer);

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
        }

        
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;

}

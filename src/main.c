#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "config.h"

typedef struct {
    MIX_Mixer   *mixer;
    MIX_Track   *track;           // the song loaded
    MIX_Audio   *audio;
    MIX_Track   *retiring;        // the previous song fading out
    MIX_Audio   *retiring_audio;
    const char **queue;
    int          queue_size;
    int          index;
} Player;

TTF_Font *global_font = NULL;

// Creates a track and loads the audio
static MIX_Track *create_track(MIX_Mixer *mixer, const char *filename)
{
    // Load the audio
    MIX_Audio *audio = MIX_LoadAudio(
        mixer,
        filename,
        false
    );

    // Check if audio loaded
    if (audio == NULL) {
        SDL_Log("MIX_LoadAudio failed: %s", SDL_GetError());
        return NULL;
    }

    // Create a track
    MIX_Track *track = MIX_CreateTrack(mixer);

    // Check if track was created
    if (track == NULL) {
        SDL_Log("MIX_CreateTrack failed: %s", SDL_GetError());
        MIX_DestroyAudio(audio);
        return NULL;
    }

    // Connect the audio to the track
    if (!MIX_SetTrackAudio(track, audio)) {
        SDL_Log("MIX_SetTrackAudio failed: %s", SDL_GetError());
        MIX_DestroyAudio(audio);
        MIX_DestroyTrack(track);
        return NULL;
    }

    return track;
}

// Loads queue[index], fades out whatever was playing, and makes it current
static bool player_load(Player *p, int index, bool play)
{
    MIX_Track *track = create_track(p->mixer, p->queue[index]);

    // Leave the current song alone if the new one didn't load
    if (track == NULL) {
        SDL_Log("Failed to load \"%s\"", p->queue[index]);
        return false;
    }

    // A track left over from two loads ago has had more than enough time to
    // finish fading, so it can go now
    if (p->retiring != NULL) {
        if (MIX_TrackPlaying(p->retiring)) {
            MIX_StopTrack(p->retiring, 0);
        }
        MIX_DestroyTrack(p->retiring);
        MIX_DestroyAudio(p->retiring_audio);
    }

    // The current track has to outlive this function or the fade gets cut off
    p->retiring = p->track;
    p->retiring_audio = p->audio;

    if (p->track != NULL) {
        MIX_StopTrack(p->track, MIX_TrackMSToFrames(p->track, FADE_OUT_MS));
    }

    p->track = track;
    p->audio = MIX_GetTrackAudio(track);
    p->index = index;

    if (play) {
        MIX_PlayTrack(p->track, 0);
    }

    return true;
}

// Moves the queue position by delta, wrapping around at both ends
static void player_skip(Player *p, int delta)
{
    int index = (p->index + delta + p->queue_size) % p->queue_size;

    if (player_load(p, index, true)) {
        printf("Queue index: %d\n", p->index);
    }
}

static void player_destroy(Player *p)
{
    if (p->retiring != NULL) {
        MIX_DestroyTrack(p->retiring);
        MIX_DestroyAudio(p->retiring_audio);
        p->retiring = NULL;
        p->retiring_audio = NULL;
    }

    if (p->track != NULL) {
        MIX_DestroyTrack(p->track);
        MIX_DestroyAudio(p->audio);
        p->track = NULL;
        p->audio = NULL;
    }
}

// Free the previous song once the mixer has finished fading it out
static void player_reap(Player *p)
{
    if (p->retiring != NULL && !MIX_TrackPlaying(p->retiring)) {
        MIX_DestroyTrack(p->retiring);
        MIX_DestroyAudio(p->retiring_audio);
        p->retiring = NULL;
        p->retiring_audio = NULL;
    }
}

// Was the mouse at x, y inside this button?
static bool button_hit(SDL_FRect r, float x, float y)
{
    return x >= r.x &&
           x <= r.x + r.w &&
           y >= r.y &&
           y <= r.y + r.h;
}

int main(int argc, char* argv[])
{
  // Variables
  bool running = true;
  SDL_Event event;

  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;

  const char *queue[] = {
      MUSIC_QUEUE
  };

  // The rest of main talks to the player through this
  Player player = {0};
  player.queue = queue;
  player.queue_size = sizeof(queue) / sizeof(queue[0]);
  player.index = 0;

  // Initialize SDL
  if (!SDL_Init(SDL_INIT_VIDEO)) {
      SDL_Log("SDL_Init failed: %s", SDL_GetError());
      return 1;
  }

  // Init SDL3_ttf
  TTF_Init();
  TTF_TextEngine *text_engine = TTF_CreateRendererTextEngine(renderer);
  global_font = TTF_OpenFont(FONT_PATH, FONT_SIZE);
  if (!global_font) {
    SDL_Log("Font failed to load: %s:", SDL_GetError());
    return 1;
  }

  // Initialize SDL_mixer
  if (!MIX_Init()) {
      SDL_Log("MIX_Init failed: %s", SDL_GetError());
      SDL_Quit();
      return 1;
  }

  // Create audio mixer
  player.mixer = MIX_CreateMixerDevice(
      SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
      NULL
  );

  // Check if mixer isn't working
  if (player.mixer == NULL) {
      SDL_Log("MIX_CreateMixerDevice failed: %s", SDL_GetError());
      goto cleanup;
  }

  /// Create a window
  window = SDL_CreateWindow(
      WINDOW_TITLE,
      WINDOW_WIDTH,
      WINDOW_HEIGHT,
      0
  );

  // Check if the window failed
  if (window == NULL) {
      SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
      goto cleanup;
  }

  // Create renderer
  renderer = SDL_CreateRenderer(window, NULL);

  // Check if the renderer failed
  if (renderer == NULL) {
      SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
      goto cleanup;
  }

  // Enable Vsync if available
  if (!SDL_SetRenderVSync(renderer, 1)) {
  SDL_Log("SDL_SetRenderVSync Vsync unavailable: %s", SDL_GetError());
  }

  // Load the first song. It stays paused until the play button is pressed
  if (!player_load(&player, 0, false)) {
      SDL_Log("Track not found!");
      goto cleanup;
  }

  int button_center = BUTTON_CENTER_Y; // y value that buttons will be centered on

  // Define the buttons
  SDL_FRect play_pause_button = {
      .x = (WINDOW_WIDTH - PLAY_PAUSE_SIZE) / 2,
      .y = button_center - PLAY_PAUSE_SIZE / 2,
      .w = PLAY_PAUSE_SIZE,
      .h = PLAY_PAUSE_SIZE
  };

  SDL_FRect next_button = {
      .x = play_pause_button.x + play_pause_button.w + BUTTON_GAP,
      .y = button_center - SKIP_BUTTON_SIZE / 2,
      .w = SKIP_BUTTON_SIZE,
      .h = SKIP_BUTTON_SIZE
  };

  SDL_FRect previous_button = {
      .x = play_pause_button.x - SKIP_BUTTON_SIZE - BUTTON_GAP,
      .y = button_center - SKIP_BUTTON_SIZE / 2,
      .w = SKIP_BUTTON_SIZE,
      .h = SKIP_BUTTON_SIZE
  };


  // Main loop
  while (running) {

    // Stop hanging on to a song that finished fading out
    player_reap(&player);

    bool space_pressed;
    space_pressed = false;
    // Set background to black
    SDL_SetRenderDrawColor(renderer, COLOR_BACKGROUND);
    SDL_RenderClear(renderer);

    // Set buttons to white
    SDL_SetRenderDrawColor(renderer, COLOR_BUTTON);
    SDL_RenderFillRect(renderer, &play_pause_button);
    SDL_RenderFillRect(renderer, &next_button);
    SDL_RenderFillRect(renderer, &previous_button);

    SDL_RenderPresent(renderer);
 
    SDL_PropertiesID props = MIX_GetAudioProperties(player.audio);
    const char *title = SDL_GetStringProperty(props, MIX_PROP_METADATA_TITLE_STRING, "Unknown Title");
    const char *artist = SDL_GetStringProperty(props, MIX_PROP_METADATA_ARTIST_STRING, "Unknown Artist");
    TTF_Text *track_text = TTF_CreateText(text_engine, global_font, title, 0);
    TTF_DrawRendererText(track_text, 50.0f, 50.0f);

    // Event loop
    while (SDL_PollEvent(&event)) {

      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }

      if (event.type == SDL_EVENT_KEY_DOWN) {

        if (event.key.key == SDLK_ESCAPE) {
            running = false;
        }

        if (event.key.key == SDLK_SPACE) {
            printf("you pressed space\n");
            space_pressed = true;
        }
    }

      if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_KEY_DOWN) {
        float x = event.button.x;
        float y = event.button.y;

        // Play / pause button
        if (button_hit(play_pause_button, x, y) || space_pressed) {

            printf("you clicked the play pause button nice job\n");
    
            SDL_PropertiesID props = MIX_GetAudioProperties(player.audio);
            const char *title = SDL_GetStringProperty(props, MIX_PROP_METADATA_TITLE_STRING, "Unknown Title");
            const char *artist = SDL_GetStringProperty(props, MIX_PROP_METADATA_ARTIST_STRING, "Unknown Artist");

            // A track that was loaded but never started is neither
            // playing nor paused, so this handles the first press too
            if (MIX_TrackPaused(player.track)) {
                MIX_ResumeTrack(player.track);
                printf("Playing Track: %s\n", title);
                printf("Artist: %s\n", artist);
            } else if (MIX_TrackPlaying(player.track)) {
                MIX_PauseTrack(player.track);
                printf("Paused Track: %s\n", title);
            } else {
                MIX_PlayTrack(player.track, 0);
                printf("Playing track: %s\n", title);
                printf("Artist: %s\n", artist);
            }

          }

          // Next button
          if (button_hit(next_button, x, y)) {
            printf("you clicked on the next button nice job\n");
            player_skip(&player, 1);

            SDL_PropertiesID props = MIX_GetAudioProperties(player.audio);
            const char *title = SDL_GetStringProperty(props, MIX_PROP_METADATA_TITLE_STRING, "Unknown Title");
            const char *artist = SDL_GetStringProperty(props, MIX_PROP_METADATA_ARTIST_STRING, "Unknown Artist");
            printf("Track Title: %s\n", title);
            printf("Artist: %s\n", artist);
        }

          // Previous button
          if (button_hit(previous_button, x, y)) {
            printf("you clicked on the previous button nice job\n");
            player_skip(&player, -1);

            SDL_PropertiesID props = MIX_GetAudioProperties(player.audio);
            const char *title = SDL_GetStringProperty(props, MIX_PROP_METADATA_TITLE_STRING, "Unknown Title");
            const char *artist = SDL_GetStringProperty(props, MIX_PROP_METADATA_ARTIST_STRING, "Unknown Artist");
            printf("Track Title: %s\n", title);
            printf("Artist: %s\n", artist);
        }
      }
  }

  }

// Cleanup
cleanup:
  player_destroy(&player);

  if (player.mixer != NULL) {
    MIX_DestroyMixer(player.mixer);
  }
  MIX_Quit();

  if (renderer != NULL) {
    SDL_DestroyRenderer(renderer);
  }
  if (window != NULL) {
    SDL_DestroyWindow(window);
  }
  SDL_Quit();

  return 0;
}

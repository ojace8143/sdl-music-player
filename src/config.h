#ifndef CONFIG_H
#define CONFIG_H

#include <SDL3/SDL.h>

// Window settings
#define WINDOW_TITLE "ojace8143's music playerrrrr3"
#define WINDOW_WIDTH  600
#define WINDOW_HEIGHT 200

// Colors                r,  g,    b,   a
#define COLOR_BACKGROUND 112, 128, 144, 255
#define COLOR_BUTTON     255, 255, 255, 255

// Buttons
#define BUTTON_CENTER_Y  150
#define PLAY_PAUSE_SIZE  50
#define SKIP_BUTTON_SIZE 35
#define BUTTON_GAP       10

// Track fade in ms
#define FADE_OUT_MS 50

// Keybinds
#define KEY_QUIT       SDLK_ESCAPE
#define KEY_PLAY_PAUSE SDLK_SPACE

#define MUSIC_DIR "/home/ojace8143/Music/Tool-10,000_Days"

// The queue
#define MUSIC_QUEUE \
    MUSIC_DIR "/01.Vicarious.ogg",                 \
    MUSIC_DIR "/02.Jambi.ogg",                     \
    MUSIC_DIR "/03.Wings_for_Marie,_Pt_1.ogg",     \
    MUSIC_DIR "/04.10,000_Days_(Wings,_Pt_2).ogg", \
    MUSIC_DIR "/05.The_Pot.ogg",                   \
    MUSIC_DIR "/06.Lipan_Conjuring.ogg",           \
    MUSIC_DIR "/07.Lost_Keys_(Blame_Hofmann).ogg", \
    MUSIC_DIR "/08.Rosetta_Stoned.ogg",            \
    MUSIC_DIR "/09.Intension.ogg",                 \
    MUSIC_DIR "/10.Right_in_Two.ogg",              \
    MUSIC_DIR "/11.Viginti_Tres.ogg"

#endif

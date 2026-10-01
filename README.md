# SDL Music Player
random project that will eventually be a music player with sdl

just for personal use so I (and you) don't have to deal with online services like spotify and all their bullshit

## How to compile

clone it  
`git clone https://github.com/ojace8143/sdl-music-player.git`

compile it 
`./compile`
or
`gcc -o sdl-music-player src/main.c $(pkg-config --cflags --libs sdl3-mixer)`

then run it
`./sdl-music-player`

if you change anything in config.h, recompile, then run `./sdl-music-player'` and it should work

## TODO List
- [x] Get a window
- [x] Event handler
- [x] Clickable button
- [x] Audio with button
- [x] Multiple buttons
- [ ] Create basic UI
- [ ] Create UI Assets
- [x] Functionality
- [ ] song progress bar
- [ ] show song title and metadata
- [x] add more stuff todo later
- [ ] clean up structure and redundant code

## Currently working on
functionality. my dad also has an old tool cd (10,000 days), so i'm using that as the testing audio tracks.

## Notes
keybinds not working rn

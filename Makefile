main: main.c
	gcc -lSDL2 -lSDL2_image -lm main.c -o main

js: main.c
	emcc -O2 main.c -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' --embed-file res -o web/tetris.html

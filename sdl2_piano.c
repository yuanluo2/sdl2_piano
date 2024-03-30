#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#undef main

/* frame rate. */
#define FRAME_RATE    30
#define FRAME_DELAY   (1000 / FRAME_RATE)

/* piano keys' attributes. */
#define BLACK_KEY_WIDTH  40
#define BLACK_KEY_HEIGHT 254
#define WHITE_KEY_WIDTH  56
#define WHITE_KEY_HEIGHT 390

/* C3 -> B5 tones. */
#define BLACK_KEY_NUM   15
#define WHITE_KEY_NUM   21
#define PIANO_KEY_NUM   (BLACK_KEY_NUM + WHITE_KEY_NUM)

/* window attributes. */
#define WINDOW_TITLE    "Piano"
#define WINDOW_HEIGHT   WHITE_KEY_HEIGHT
#define WINDOW_WIDTH    (WHITE_KEY_WIDTH * WHITE_KEY_NUM)

/* config about the sound. */
#define DEFAULT_FREQUENCY     48000
#define DEFAULT_CHANNEL_NUM   8
#define DEFAULT_CHUNK_SIZE    2048
#define SOUND_PATH_MAX_LEN    32

/* text. */
#define FONT_PATH           "./resources/arial.ttf"
#define DEFAULT_FONT_SIZE   15
#define KEY_NAME_DISTANCE   22
#define TONE_NAME_DISTANCE  42

/* piano key only contains 2 types: black and white. */
typedef enum KeyType {
	KT_BLACK,
	KT_WHITE
} KeyType;

typedef struct PianoKey {
	KeyType keyType;         /* black or white ? */
	const char* keyName;     /* key name on the keyboard. */
	const char* toneName;    /* tone name. */
	Mix_Chunk* chunk;        /* the sound to be played. */
	int isPressed;           /* is this key pressed ? */
	int initX;               /* x position, used to render. */
} PianoKey;

/* colors. */
static const SDL_Color COLOR_WHITE = { 255, 255, 255, 255 };
static const SDL_Color COLOR_BLACK = {   0,   0,   0, 255 };
static const SDL_Color COLOR_MIKU  = {  57, 197, 187, 255 };

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static TTF_Font* font = NULL;
static PianoKey* pianoKeys = NULL;
static char soundPath[SOUND_PATH_MAX_LEN];

int init_graphics(void) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0){
		SDL_Log("SDL_Init() failed: %s\n", SDL_GetError());
		return 0;
	}

	window = SDL_CreateWindow(WINDOW_TITLE, 
								SDL_WINDOWPOS_CENTERED, 
								SDL_WINDOWPOS_CENTERED, 
								WINDOW_WIDTH, 
								WINDOW_HEIGHT, 
								0);
							
	if (window == NULL){
		SDL_Log("create window failed: %s\n", SDL_GetError());
		return 0;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (renderer == NULL){
		SDL_Log("create renderer failed: %s\n", SDL_GetError());
		return 0;
	}

	return 1;
}

void init_pianoKey(PianoKey* pk, KeyType kt, const char* keyName, const char* toneName, int initX){
	pk->keyType = kt;
	pk->keyName = keyName;
	pk->toneName = toneName;
	pk->chunk = NULL;
	pk->isPressed = 0;
	pk->initX = initX;
}

int init_pianoKeys(void){
	pianoKeys = (PianoKey*)malloc(PIANO_KEY_NUM * sizeof(PianoKey));
	if (pianoKeys == NULL){
		SDL_Log("out of memory.\n");
		return 0;
	}

	init_pianoKey(&(pianoKeys[0]),  KT_WHITE, "1", "C3",  WHITE_KEY_WIDTH * 0);
	init_pianoKey(&(pianoKeys[1]),  KT_WHITE, "3", "D3",  WHITE_KEY_WIDTH * 1);
	init_pianoKey(&(pianoKeys[2]),  KT_WHITE, "5", "E3",  WHITE_KEY_WIDTH * 2);
	init_pianoKey(&(pianoKeys[3]),  KT_WHITE, "6", "F3",  WHITE_KEY_WIDTH * 3);
	init_pianoKey(&(pianoKeys[4]),  KT_WHITE, "8", "G3",  WHITE_KEY_WIDTH * 4);
	init_pianoKey(&(pianoKeys[5]),  KT_WHITE, "0", "A3",  WHITE_KEY_WIDTH * 5);
	init_pianoKey(&(pianoKeys[6]),  KT_WHITE, "W", "B3",  WHITE_KEY_WIDTH * 6);
	init_pianoKey(&(pianoKeys[7]),  KT_WHITE, "E", "C4",  WHITE_KEY_WIDTH * 7);
	init_pianoKey(&(pianoKeys[8]),  KT_WHITE, "T", "D4",  WHITE_KEY_WIDTH * 8);
	init_pianoKey(&(pianoKeys[9]),  KT_WHITE, "U", "E4",  WHITE_KEY_WIDTH * 9);
	init_pianoKey(&(pianoKeys[10]), KT_WHITE, "I", "F4",  WHITE_KEY_WIDTH * 10);
	init_pianoKey(&(pianoKeys[11]), KT_WHITE, "P", "G4",  WHITE_KEY_WIDTH * 11);
	init_pianoKey(&(pianoKeys[12]), KT_WHITE, "S", "A4",  WHITE_KEY_WIDTH * 12);
	init_pianoKey(&(pianoKeys[13]), KT_WHITE, "F", "B4",  WHITE_KEY_WIDTH * 13);
	init_pianoKey(&(pianoKeys[14]), KT_WHITE, "G", "C5",  WHITE_KEY_WIDTH * 14);
	init_pianoKey(&(pianoKeys[15]), KT_WHITE, "J", "D5",  WHITE_KEY_WIDTH * 15);
	init_pianoKey(&(pianoKeys[16]), KT_WHITE, "L", "E5",  WHITE_KEY_WIDTH * 16);
	init_pianoKey(&(pianoKeys[17]), KT_WHITE, "Z", "F5",  WHITE_KEY_WIDTH * 17);
	init_pianoKey(&(pianoKeys[18]), KT_WHITE, "C", "G5",  WHITE_KEY_WIDTH * 18);
	init_pianoKey(&(pianoKeys[19]), KT_WHITE, "B", "A5",  WHITE_KEY_WIDTH * 19);
	init_pianoKey(&(pianoKeys[20]), KT_WHITE, "M", "B5",  WHITE_KEY_WIDTH * 20);
	init_pianoKey(&(pianoKeys[21]), KT_BLACK, "2", "Db3", WHITE_KEY_WIDTH * 1 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(pianoKeys[22]), KT_BLACK, "4", "Eb3", WHITE_KEY_WIDTH * 2 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(pianoKeys[23]), KT_BLACK, "7", "Gb3", WHITE_KEY_WIDTH * 4 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(pianoKeys[24]), KT_BLACK, "9", "Ab3", WHITE_KEY_WIDTH * 5 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(pianoKeys[25]), KT_BLACK, "Q", "Bb3", WHITE_KEY_WIDTH * 6 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(pianoKeys[26]), KT_BLACK, "R", "Db4", WHITE_KEY_WIDTH * 8 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(pianoKeys[27]), KT_BLACK, "Y", "Eb4", WHITE_KEY_WIDTH * 9 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(pianoKeys[28]), KT_BLACK, "O", "Gb4", WHITE_KEY_WIDTH * 11 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(pianoKeys[29]), KT_BLACK, "A", "Ab4", WHITE_KEY_WIDTH * 12 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(pianoKeys[30]), KT_BLACK, "D", "Bb4", WHITE_KEY_WIDTH * 13 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(pianoKeys[31]), KT_BLACK, "H", "Db5", WHITE_KEY_WIDTH * 15 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(pianoKeys[32]), KT_BLACK, "K", "Eb5", WHITE_KEY_WIDTH * 16 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(pianoKeys[33]), KT_BLACK, "X", "Gb5", WHITE_KEY_WIDTH * 18 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(pianoKeys[34]), KT_BLACK, "V", "Ab5", WHITE_KEY_WIDTH * 19 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(pianoKeys[35]), KT_BLACK, "N", "Bb5", WHITE_KEY_WIDTH * 20 - BLACK_KEY_WIDTH / 2);

	return 1;
}

int init_ttf(void) {
	if (TTF_Init() == -1) {
        	SDL_Log("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        	return 0;
    	}

	font = TTF_OpenFont(FONT_PATH, DEFAULT_FONT_SIZE);
    	if (font == NULL) {
        	SDL_Log("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
		return 0;
    	}

	return 1;
}

int init_audio(void) {
	if (Mix_OpenAudio(DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, DEFAULT_CHANNEL_NUM, DEFAULT_CHUNK_SIZE) < 0) {
        	SDL_Log("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        	return 0;
    	}

	return 1;
}

int init_resources(void){
	return init_graphics() && init_pianoKeys() && init_ttf() && init_audio();
}

void clean_resources(void){
	int i;
	for (i = 0; i < PIANO_KEY_NUM; ++i){
		if (pianoKeys[i].chunk != NULL) {
			Mix_FreeChunk(pianoKeys[i].chunk);
		}
	}

	Mix_CloseAudio();
	free(pianoKeys);

	if (font != NULL){
		TTF_CloseFont(font);
	}

	if (renderer != NULL){
		SDL_DestroyRenderer(renderer);
	}
	
	if (window != NULL){
		SDL_DestroyWindow(window);
	}

	Mix_Quit();
	TTF_Quit();
	SDL_Quit();
}

PianoKey* get_piano_key_mapping(SDL_KeyCode key){
	switch (key) {
		case SDLK_1: return &(pianoKeys[0]);
		case SDLK_3: return &(pianoKeys[1]);
		case SDLK_5: return &(pianoKeys[2]);
		case SDLK_6: return &(pianoKeys[3]);
		case SDLK_8: return &(pianoKeys[4]);
		case SDLK_0: return &(pianoKeys[5]);
		case SDLK_w: return &(pianoKeys[6]);
		case SDLK_e: return &(pianoKeys[7]);
		case SDLK_t: return &(pianoKeys[8]);
		case SDLK_u: return &(pianoKeys[9]);
		case SDLK_i: return &(pianoKeys[10]);
		case SDLK_p: return &(pianoKeys[11]);
		case SDLK_s: return &(pianoKeys[12]);
		case SDLK_f: return &(pianoKeys[13]);
		case SDLK_g: return &(pianoKeys[14]);
		case SDLK_j: return &(pianoKeys[15]);
		case SDLK_l: return &(pianoKeys[16]);
		case SDLK_z: return &(pianoKeys[17]);
		case SDLK_c: return &(pianoKeys[18]);
		case SDLK_b: return &(pianoKeys[19]);
		case SDLK_m: return &(pianoKeys[20]);
		case SDLK_2: return &(pianoKeys[21]);
		case SDLK_4: return &(pianoKeys[22]);
		case SDLK_7: return &(pianoKeys[23]);
		case SDLK_9: return &(pianoKeys[24]);
		case SDLK_q: return &(pianoKeys[25]);
		case SDLK_r: return &(pianoKeys[26]);
		case SDLK_y: return &(pianoKeys[27]);
		case SDLK_o: return &(pianoKeys[28]);
		case SDLK_a: return &(pianoKeys[29]);
		case SDLK_d: return &(pianoKeys[30]);
		case SDLK_h: return &(pianoKeys[31]);
		case SDLK_k: return &(pianoKeys[32]);
		case SDLK_x: return &(pianoKeys[33]);
		case SDLK_v: return &(pianoKeys[34]);
		case SDLK_n: return &(pianoKeys[35]);
		default: return NULL;
	}
}

int render_key(PianoKey* pk){
	SDL_Rect rect;
	SDL_Color textColor;
	SDL_Surface* keyNameSurface, *toneNameSurface;
	SDL_Texture* keyNameTexture, *toneNameTexture;
	rect.x = pk->initX;
	rect.y = 0;
	int width, height;

	if (pk->keyType == KT_BLACK){
		rect.w = BLACK_KEY_WIDTH;
		rect.h = BLACK_KEY_HEIGHT;
		textColor = COLOR_WHITE;

		if (pk->isPressed){
			SDL_SetRenderDrawColor(renderer, COLOR_MIKU.r, COLOR_MIKU.g, COLOR_MIKU.b, COLOR_MIKU.a);
		}
		else {
			SDL_SetRenderDrawColor(renderer, COLOR_BLACK.r, COLOR_BLACK.g, COLOR_BLACK.b, COLOR_BLACK.a);
		}
	}
	else {
		rect.w = WHITE_KEY_WIDTH;
		rect.h = WHITE_KEY_HEIGHT;
		textColor = COLOR_BLACK;
		
		if (pk->isPressed){
			SDL_SetRenderDrawColor(renderer, COLOR_MIKU.r, COLOR_MIKU.g, COLOR_MIKU.b, COLOR_MIKU.a);
		}
		else {
			SDL_SetRenderDrawColor(renderer, COLOR_WHITE.r, COLOR_WHITE.g, COLOR_WHITE.b, COLOR_WHITE.a);
		}
	}

	SDL_RenderFillRect(renderer, &rect);

	if ((keyNameSurface = TTF_RenderText_Solid(font, pk->keyName, textColor)) == NULL) {
		SDL_Log("Unable to create key name surface: %s, SDL_ttf Error: %s\n", pk->keyName, TTF_GetError());
		return 0;
	}

	if ((toneNameSurface = TTF_RenderText_Solid(font, pk->toneName, textColor)) == NULL) {
		SDL_Log("Unable to create tone name surface: %s, SDL_ttf Error: %s\n", pk->toneName, TTF_GetError());
		return 0;
	}

	if ((keyNameTexture = SDL_CreateTextureFromSurface(renderer, keyNameSurface)) == NULL){
		SDL_Log("Unable to create key name texture: %s, SDL_ttf Error: %s\n", pk->keyName, TTF_GetError());
		return 0;
	}

	if ((toneNameTexture = SDL_CreateTextureFromSurface(renderer, toneNameSurface)) == NULL){
		SDL_Log("Unable to create tone name texture: %s, SDL_ttf Error: %s\n", pk->toneName, TTF_GetError());
		exit(EXIT_FAILURE);
	}

	SDL_Rect keyNameRect = { 
		pk->initX + rect.w / 2 - keyNameSurface->w / 2, 
		rect.h - KEY_NAME_DISTANCE, 
		keyNameSurface->w, 
		keyNameSurface->h 
	};

	SDL_Rect toneNameRect = { 
		pk->initX + rect.w / 2 - toneNameSurface->w / 2, 
		rect.h - TONE_NAME_DISTANCE, 
		toneNameSurface->w, 
		toneNameSurface->h 
	};

	SDL_RenderCopy(renderer, keyNameTexture, NULL, &keyNameRect);
	SDL_RenderCopy(renderer, toneNameTexture, NULL, &toneNameRect);

	SDL_FreeSurface(keyNameSurface);
	SDL_FreeSurface(toneNameSurface);
	SDL_DestroyTexture(keyNameTexture);
	SDL_DestroyTexture(toneNameTexture);

	return 1;
}

int render(void){
	SDL_RenderClear(renderer);

	int i;
	/* render white keys. */
	for (i = 0; i < WHITE_KEY_NUM; ++i) {
		if (!render_key(&(pianoKeys[i]))){
			return 0;
		}
	}

	/* render lines. */
	SDL_SetRenderDrawColor(renderer, COLOR_BLACK.r, COLOR_BLACK.g, COLOR_BLACK.b, COLOR_BLACK.a);
	for (i = 0; i < WHITE_KEY_NUM; ++i){
		SDL_RenderDrawLine(renderer, i * WHITE_KEY_WIDTH, 0, i * WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT);
	}

	/* render black keys. */
	for (i = WHITE_KEY_NUM; i < PIANO_KEY_NUM; ++i){
		if (!render_key(&(pianoKeys[i]))){
			return 0;
		}
	}

	SDL_RenderPresent(renderer);
	return 1;
}

int load_sound(PianoKey* pk){
	snprintf(soundPath, SOUND_PATH_MAX_LEN, "./resources/%s.wav", pk->toneName);

	if ((pk->chunk = Mix_LoadWAV(soundPath)) == NULL){
		SDL_Log("Can't load sound resource: %s, error: %s\n", soundPath, Mix_GetError());
		return 0;
	}

	return 1;
}

int main(){
	if (!init_resources()){
		goto finally;
	}

	PianoKey* pk;
	Uint32 startTime, endTime, frameTime;
    	int running = 1;
    	SDL_Event event;

    	while (running) {
		startTime = SDL_GetTicks();

        	while (SDL_PollEvent(&event)) {
            		if (event.type == SDL_QUIT) {
                		running = 0;
			} else if (event.type == SDL_KEYDOWN) {
                		pk = get_piano_key_mapping(event.key.keysym.sym);

				if (pk != NULL){
					pk->isPressed = 1;

					if (pk->chunk == NULL){
						if (!load_sound(pk)){
							goto finally;
						}
					}

					Mix_PlayChannel(event.key.keysym.sym % DEFAULT_CHANNEL_NUM, pk->chunk, 0);
				}
            		} else if (event.type == SDL_KEYUP) {
                		pk = get_piano_key_mapping(event.key.keysym.sym);

				if (pk != NULL){
					pk->isPressed = 0;
				}
            		}
        	}

		if (!render()){
			goto finally;
		}

		endTime = SDL_GetTicks();
        	frameTime = endTime - startTime;

        	if (frameTime < FRAME_DELAY) {
            		SDL_Delay(FRAME_DELAY - frameTime);
        	}
    	}

finally:
	clean_resources();
	return 0;
}

#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#undef main

/* frame rate. */
#define FRAME_RATE    60
#define FRAME_DELAY   (1000 / FRAME_RATE)

/* colors. */
#define COLOR_RGBA_WHITE   255, 255, 255, 255
#define COLOR_RGBA_BLACK     0,   0,   0, 255
#define COLOR_RGBA_MIKU     57, 197, 187, 255

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
#define DEFAULT_FREQUENCY       48000
#define DEFAULT_CHANNEL_NUM     8
#define DEFAULT_CHUNK_SIZE      2048
#define RESOURCE_PATH_MAX_LEN   32

/* text. */
#define DEFAULT_FONT_SIZE   15
#define KEY_NAME_DISTANCE   22
#define TONE_NAME_DISTANCE  42

/* piano key only contains 2 types: black and white. */
typedef int KeyType;
#define BLACK_KEY 0
#define WHITE_KEY 1

typedef struct PianoKey {
	KeyType ktype;           /* black or white ? */
	const char* keyName;     /* key name on the keyboard. */
	const char* toneName;    /* tone name. */
	Mix_Chunk* chunk;        /* the sound to be played. */
	SDL_Texture* keyNameTexture;
	SDL_Texture* toneNameTexture;
	int isPressed;           /* is this key pressed ? */
	int initX;               /* x position, used to render. */
} PianoKey;

static PianoKey pianoKeys[] = {
	{ WHITE_KEY, "1", "C3",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 0 },
	{ WHITE_KEY, "3", "D3",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 1 },
	{ WHITE_KEY, "5", "E3",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 2 },
	{ WHITE_KEY, "6", "F3",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 3 },
	{ WHITE_KEY, "8", "G3",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 4 },
	{ WHITE_KEY, "0", "A3",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 5 },
	{ WHITE_KEY, "W", "B3",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 6 },
	{ WHITE_KEY, "E", "C4",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 7 },
	{ WHITE_KEY, "T", "D4",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 8 },
	{ WHITE_KEY, "U", "E4",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 9 },
	{ WHITE_KEY, "I", "F4",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 10 },
	{ WHITE_KEY, "P", "G4",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 11 },
	{ WHITE_KEY, "S", "A4",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 12 },
	{ WHITE_KEY, "F", "B4",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 13 },
	{ WHITE_KEY, "G", "C5",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 14 },
	{ WHITE_KEY, "J", "D5",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 15 },
	{ WHITE_KEY, "L", "E5",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 16 },
	{ WHITE_KEY, "Z", "F5",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 17 },
	{ WHITE_KEY, "C", "G5",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 18 },
	{ WHITE_KEY, "B", "A5",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 19 },
	{ WHITE_KEY, "M", "B5",  NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 20 },
	{ BLACK_KEY, "2", "Db3", NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 1 - BLACK_KEY_WIDTH / 2 },
	{ BLACK_KEY, "4", "Eb3", NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 2 - BLACK_KEY_WIDTH / 2 },
	{ BLACK_KEY, "7", "Gb3", NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 4 - BLACK_KEY_WIDTH / 2 },
	{ BLACK_KEY, "9", "Ab3", NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 5 - BLACK_KEY_WIDTH / 2 },
	{ BLACK_KEY, "Q", "Bb3", NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 6 - BLACK_KEY_WIDTH / 2 },
	{ BLACK_KEY, "R", "Db4", NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 8 - BLACK_KEY_WIDTH / 2 },
	{ BLACK_KEY, "Y", "Eb4", NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 9 - BLACK_KEY_WIDTH / 2 },
	{ BLACK_KEY, "O", "Gb4", NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 11 - BLACK_KEY_WIDTH / 2 },
	{ BLACK_KEY, "A", "Ab4", NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 12 - BLACK_KEY_WIDTH / 2 },
	{ BLACK_KEY, "D", "Bb4", NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 13 - BLACK_KEY_WIDTH / 2 },
	{ BLACK_KEY, "H", "Db5", NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 15 - BLACK_KEY_WIDTH / 2 },
	{ BLACK_KEY, "K", "Eb5", NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 16 - BLACK_KEY_WIDTH / 2 },
	{ BLACK_KEY, "X", "Gb5", NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 18 - BLACK_KEY_WIDTH / 2 },
	{ BLACK_KEY, "V", "Ab5", NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 19 - BLACK_KEY_WIDTH / 2 },
	{ BLACK_KEY, "N", "Bb5", NULL, NULL, NULL, 0, WHITE_KEY_WIDTH * 20 - BLACK_KEY_WIDTH / 2 },
};

static SDL_Window* window;
static SDL_Renderer* renderer;
static TTF_Font* font;

void init_graphics(void){
	if (SDL_Init(SDL_INIT_VIDEO) < 0){
		SDL_Log("SDL_Init() failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	window = SDL_CreateWindow(WINDOW_TITLE, 
								SDL_WINDOWPOS_CENTERED, 
								SDL_WINDOWPOS_CENTERED, 
								WINDOW_WIDTH, 
								WINDOW_HEIGHT, 
								0);
							
	if (window == NULL){
		SDL_Log("create window failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL){
		SDL_Log("create renderer failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
}

void init_ttf(void){
	int i;
	PianoKey* pk;
	SDL_Surface* keyTextSurface;
	SDL_Surface* toneTextSurface;
	SDL_Color colorBlack = { COLOR_RGBA_BLACK };
	SDL_Color colorWhite = { COLOR_RGBA_WHITE };

	if (TTF_Init() == -1) {
        	SDL_Log("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        	exit(EXIT_FAILURE);
    	}

	font = TTF_OpenFont("./resources/arial.ttf", DEFAULT_FONT_SIZE);
    	if (font == NULL) {
        	SDL_Log("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        	exit(EXIT_FAILURE);
    	}

	for (i = 0; i < WHITE_KEY_NUM; ++i){
		pk = &(pianoKeys[i]);
	
		if ((keyTextSurface = TTF_RenderText_Solid(font, pk->keyName, colorBlack)) == NULL) {
			SDL_Log("Unable to create key name surface: %s, SDL_ttf Error: %s\n", pk->keyName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		if ((toneTextSurface = TTF_RenderText_Solid(font, pk->toneName, colorBlack)) == NULL) {
			SDL_Log("Unable to create tone name surface: %s, SDL_ttf Error: %s\n", pk->toneName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		if ((pk->keyNameTexture = SDL_CreateTextureFromSurface(renderer, keyTextSurface)) == NULL){
			SDL_Log("Unable to create key name texture: %s, SDL_ttf Error: %s\n", pk->keyName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		if ((pk->toneNameTexture = SDL_CreateTextureFromSurface(renderer, toneTextSurface)) == NULL){
			SDL_Log("Unable to create tone name texture: %s, SDL_ttf Error: %s\n", pk->toneName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		SDL_FreeSurface(keyTextSurface);
		SDL_FreeSurface(toneTextSurface);
	}

	for (i = WHITE_KEY_NUM; i < PIANO_KEY_NUM; ++i){
		pk = &(pianoKeys[i]);
	
		if ((keyTextSurface = TTF_RenderText_Solid(font, pk->keyName, colorWhite)) == NULL) {
			SDL_Log("Unable to create key name surface: %s, SDL_ttf Error: %s\n", pk->keyName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		if ((toneTextSurface = TTF_RenderText_Solid(font, pk->toneName, colorWhite)) == NULL) {
			SDL_Log("Unable to create tone name surface: %s, SDL_ttf Error: %s\n", pk->toneName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		if ((pk->keyNameTexture = SDL_CreateTextureFromSurface(renderer, keyTextSurface)) == NULL){
			SDL_Log("Unable to create key name texture: %s, SDL_ttf Error: %s\n", pk->keyName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		if ((pk->toneNameTexture = SDL_CreateTextureFromSurface(renderer, toneTextSurface)) == NULL){
			SDL_Log("Unable to create tone name texture: %s, SDL_ttf Error: %s\n", pk->toneName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		SDL_FreeSurface(keyTextSurface);
		SDL_FreeSurface(toneTextSurface);
	}
}

void init_audio(void){
	if (Mix_OpenAudio(DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, DEFAULT_CHANNEL_NUM, DEFAULT_CHUNK_SIZE) < 0) {
        	SDL_Log("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        	exit(EXIT_FAILURE);
    	}

	int i;
	PianoKey* pk;
	char resourcePath[RESOURCE_PATH_MAX_LEN];
	for (i = 0; i < PIANO_KEY_NUM; ++i){
		pk = &(pianoKeys[i]);

		sprintf(resourcePath, "./resources/%s.wav", pk->toneName);
		pk->chunk = Mix_LoadWAV(resourcePath);

		if (pk->chunk == NULL) {
			SDL_Log("Can't load sound resource: %s, error: %s\n", resourcePath, Mix_GetError());
			exit(EXIT_FAILURE);
		}
	}
}

void init(void){
	init_graphics();
	init_ttf();
	init_audio();
}

void clean(void){
	int i;
	for (i = 0; i < PIANO_KEY_NUM; ++i){
		Mix_FreeChunk(pianoKeys[i].chunk);
		SDL_DestroyTexture(pianoKeys[i].keyNameTexture);
		SDL_DestroyTexture(pianoKeys[i].toneNameTexture);
	}
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	Mix_CloseAudio();
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
}

PianoKey* getPianoKeyMapping(SDL_KeyCode key){
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

void renderPianoKeys(void){
	int i, textWidth, textHeight;
	PianoKey* pk;
	SDL_Rect rect = { 0, 0, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT };

	SDL_RenderClear(renderer);

	/* render white keys. */
	for (i = 0; i < WHITE_KEY_NUM; ++i){
		pk = &(pianoKeys[i]);
		rect.x = pk->initX;

		if (pk->isPressed){
			/* simulate the effect of pressing piano keys in real life by changing the color of the keys. */
			SDL_SetRenderDrawColor(renderer, COLOR_RGBA_MIKU);	
		}
		else {
			SDL_SetRenderDrawColor(renderer, COLOR_RGBA_WHITE);	
		}

		SDL_RenderFillRect(renderer, &rect);

		/* render text. */
		SDL_QueryTexture(pk->keyNameTexture, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect keyNameRect = { pk->initX + WHITE_KEY_WIDTH / 2 - textWidth / 2, WHITE_KEY_HEIGHT - KEY_NAME_DISTANCE, textWidth, textHeight };
		SDL_RenderCopy(renderer, pk->keyNameTexture, NULL, &keyNameRect);

		SDL_QueryTexture(pk->toneNameTexture, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect toneNameRect = { pk->initX + WHITE_KEY_WIDTH / 2 - textWidth / 2, WHITE_KEY_HEIGHT - TONE_NAME_DISTANCE, textWidth, textHeight };
		SDL_RenderCopy(renderer, pk->toneNameTexture, NULL, &toneNameRect);
	}

	/* render lines. */
	SDL_SetRenderDrawColor(renderer, COLOR_RGBA_BLACK);
	for (i = 0; i < WHITE_KEY_NUM; ++i){
		SDL_RenderDrawLine(renderer, i * WHITE_KEY_WIDTH, 0, i * WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT);
	}

	/* render black keys. */
	rect.w = BLACK_KEY_WIDTH;
	rect.h = BLACK_KEY_HEIGHT;
	for (i = WHITE_KEY_NUM; i < PIANO_KEY_NUM; ++i){
		pk = &(pianoKeys[i]);
		rect.x = pk->initX;
	
		if (pk->isPressed){
			SDL_SetRenderDrawColor(renderer, COLOR_RGBA_MIKU);	
		}
		else {
			SDL_SetRenderDrawColor(renderer, COLOR_RGBA_BLACK);	
		}
	
		SDL_RenderFillRect(renderer, &rect);

		SDL_QueryTexture(pk->keyNameTexture, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect keyNameRect = { pk->initX + BLACK_KEY_WIDTH / 2 - textWidth / 2, BLACK_KEY_HEIGHT - KEY_NAME_DISTANCE, textWidth, textHeight };
		SDL_RenderCopy(renderer, pk->keyNameTexture, NULL, &keyNameRect);

		SDL_QueryTexture(pk->toneNameTexture, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect toneNameRect = { pk->initX + BLACK_KEY_WIDTH / 2 - textWidth / 2, BLACK_KEY_HEIGHT - TONE_NAME_DISTANCE, textWidth, textHeight };
		SDL_RenderCopy(renderer, pk->toneNameTexture, NULL, &toneNameRect);
	}

	SDL_RenderPresent(renderer);
}

int main() {
	init();
	
	PianoKey* pk;
	Uint32 startTime, endTime, frameTime;
	int running = 1;
    	SDL_Event event;
	
	while (running) {
		startTime = SDL_GetTicks();

		while (SDL_PollEvent(&event) != 0) {
            		if (event.type == SDL_QUIT) {
                		running = 0;
			} else if (event.type == SDL_KEYDOWN) {
                		pk = getPianoKeyMapping(event.key.keysym.sym);

				if (pk != NULL){
					pk->isPressed = 1;

					/* disperse sound onto different channels based on the integer value of the pressed key. */
					Mix_PlayChannel(event.key.keysym.sym % DEFAULT_CHANNEL_NUM, pk->chunk, 0);
				}
            		} else if (event.type == SDL_KEYUP) {
                		pk = getPianoKeyMapping(event.key.keysym.sym);

				if (pk != NULL){
					pk->isPressed = 0;
				}
            		}
        	}

		renderPianoKeys();

		endTime = SDL_GetTicks();
        	frameTime = endTime - startTime;

        	if (frameTime < FRAME_DELAY) {
            		SDL_Delay(FRAME_DELAY - frameTime);
        	}
    	}

	clean();
    	return 0;
}

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

typedef struct PianoContext {
	PianoKey pianoKeys[PIANO_KEY_NUM];
	SDL_Window* window;
	SDL_Renderer* renderer;
	TTF_Font* font;
} PianoContext;

void init_pianoKey(PianoKey* pk, KeyType ktype, const char* keyName, const char* toneName, int initX){
	pk->ktype = ktype;
	pk->keyName = keyName;
	pk->toneName = toneName;
	pk->chunk = NULL;
	pk->keyNameTexture = NULL;
	pk->toneNameTexture = NULL;
	pk->isPressed = 0;
	pk->initX = initX;
}

void init_pianoKeys(PianoContext* context){
	init_pianoKey(&(context->pianoKeys[0]),  WHITE_KEY, "1", "C3",  WHITE_KEY_WIDTH * 0);
	init_pianoKey(&(context->pianoKeys[1]),  WHITE_KEY, "3", "D3",  WHITE_KEY_WIDTH * 1);
	init_pianoKey(&(context->pianoKeys[2]),  WHITE_KEY, "5", "E3",  WHITE_KEY_WIDTH * 2);
	init_pianoKey(&(context->pianoKeys[3]),  WHITE_KEY, "6", "F3",  WHITE_KEY_WIDTH * 3);
	init_pianoKey(&(context->pianoKeys[4]),  WHITE_KEY, "8", "G3",  WHITE_KEY_WIDTH * 4);
	init_pianoKey(&(context->pianoKeys[5]),  WHITE_KEY, "0", "A3",  WHITE_KEY_WIDTH * 5);
	init_pianoKey(&(context->pianoKeys[6]),  WHITE_KEY, "W", "B3",  WHITE_KEY_WIDTH * 6);
	init_pianoKey(&(context->pianoKeys[7]),  WHITE_KEY, "E", "C4",  WHITE_KEY_WIDTH * 7);
	init_pianoKey(&(context->pianoKeys[8]),  WHITE_KEY, "T", "D4",  WHITE_KEY_WIDTH * 8);
	init_pianoKey(&(context->pianoKeys[9]),  WHITE_KEY, "U", "E4",  WHITE_KEY_WIDTH * 9);
	init_pianoKey(&(context->pianoKeys[10]), WHITE_KEY, "I", "F4",  WHITE_KEY_WIDTH * 10);
	init_pianoKey(&(context->pianoKeys[11]), WHITE_KEY, "P", "G4",  WHITE_KEY_WIDTH * 11);
	init_pianoKey(&(context->pianoKeys[12]), WHITE_KEY, "S", "A4",  WHITE_KEY_WIDTH * 12);
	init_pianoKey(&(context->pianoKeys[13]), WHITE_KEY, "F", "B4",  WHITE_KEY_WIDTH * 13);
	init_pianoKey(&(context->pianoKeys[14]), WHITE_KEY, "G", "C5",  WHITE_KEY_WIDTH * 14);
	init_pianoKey(&(context->pianoKeys[15]), WHITE_KEY, "J", "D5",  WHITE_KEY_WIDTH * 15);
	init_pianoKey(&(context->pianoKeys[16]), WHITE_KEY, "L", "E5",  WHITE_KEY_WIDTH * 16);
	init_pianoKey(&(context->pianoKeys[17]), WHITE_KEY, "Z", "F5",  WHITE_KEY_WIDTH * 17);
	init_pianoKey(&(context->pianoKeys[18]), WHITE_KEY, "C", "G5",  WHITE_KEY_WIDTH * 18);
	init_pianoKey(&(context->pianoKeys[19]), WHITE_KEY, "B", "A5",  WHITE_KEY_WIDTH * 19);
	init_pianoKey(&(context->pianoKeys[20]), WHITE_KEY, "M", "B5",  WHITE_KEY_WIDTH * 20);
	init_pianoKey(&(context->pianoKeys[21]), BLACK_KEY, "2", "Db3", WHITE_KEY_WIDTH * 1 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(context->pianoKeys[22]), BLACK_KEY, "4", "Eb3", WHITE_KEY_WIDTH * 2 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(context->pianoKeys[23]), BLACK_KEY, "7", "Gb3", WHITE_KEY_WIDTH * 4 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(context->pianoKeys[24]), BLACK_KEY, "9", "Ab3", WHITE_KEY_WIDTH * 5 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(context->pianoKeys[25]), BLACK_KEY, "Q", "Bb3", WHITE_KEY_WIDTH * 6 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(context->pianoKeys[26]), BLACK_KEY, "R", "Db4", WHITE_KEY_WIDTH * 8 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(context->pianoKeys[27]), BLACK_KEY, "Y", "Eb4", WHITE_KEY_WIDTH * 9 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(context->pianoKeys[28]), BLACK_KEY, "O", "Gb4", WHITE_KEY_WIDTH * 11 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(context->pianoKeys[29]), BLACK_KEY, "A", "Ab4", WHITE_KEY_WIDTH * 12 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(context->pianoKeys[30]), BLACK_KEY, "D", "Bb4", WHITE_KEY_WIDTH * 13 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(context->pianoKeys[31]), BLACK_KEY, "H", "Db5", WHITE_KEY_WIDTH * 15 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(context->pianoKeys[32]), BLACK_KEY, "K", "Eb5", WHITE_KEY_WIDTH * 16 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(context->pianoKeys[33]), BLACK_KEY, "X", "Gb5", WHITE_KEY_WIDTH * 18 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(context->pianoKeys[34]), BLACK_KEY, "V", "Ab5", WHITE_KEY_WIDTH * 19 - BLACK_KEY_WIDTH / 2);
	init_pianoKey(&(context->pianoKeys[35]), BLACK_KEY, "N", "Bb5", WHITE_KEY_WIDTH * 20 - BLACK_KEY_WIDTH / 2);
}

void init_graphics(PianoContext* context){
	if (SDL_Init(SDL_INIT_VIDEO) < 0){
		SDL_Log("SDL_Init() failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	context->window = SDL_CreateWindow(WINDOW_TITLE, 
								SDL_WINDOWPOS_CENTERED, 
								SDL_WINDOWPOS_CENTERED, 
								WINDOW_WIDTH, 
								WINDOW_HEIGHT, 
								0);
							
	if (context->window == NULL){
		SDL_Log("create window failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	context->renderer = SDL_CreateRenderer(context->window, -1, SDL_RENDERER_ACCELERATED);
	if (context->renderer == NULL){
		SDL_Log("create renderer failed: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
}

void init_ttf(PianoContext* context){
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

	context->font = TTF_OpenFont("./resources/arial.ttf", DEFAULT_FONT_SIZE);
    	if (context->font == NULL) {
        	SDL_Log("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        	exit(EXIT_FAILURE);
    	}

	for (i = 0; i < WHITE_KEY_NUM; ++i){
		pk = &(context->pianoKeys[i]);
	
		if ((keyTextSurface = TTF_RenderText_Solid(context->font, pk->keyName, colorBlack)) == NULL) {
			SDL_Log("Unable to create key name surface: %s, SDL_ttf Error: %s\n", pk->keyName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		if ((toneTextSurface = TTF_RenderText_Solid(context->font, pk->toneName, colorBlack)) == NULL) {
			SDL_Log("Unable to create tone name surface: %s, SDL_ttf Error: %s\n", pk->toneName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		if ((pk->keyNameTexture = SDL_CreateTextureFromSurface(context->renderer, keyTextSurface)) == NULL){
			SDL_Log("Unable to create key name texture: %s, SDL_ttf Error: %s\n", pk->keyName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		if ((pk->toneNameTexture = SDL_CreateTextureFromSurface(context->renderer, toneTextSurface)) == NULL){
			SDL_Log("Unable to create tone name texture: %s, SDL_ttf Error: %s\n", pk->toneName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		SDL_FreeSurface(keyTextSurface);
		SDL_FreeSurface(toneTextSurface);
	}

	for (i = WHITE_KEY_NUM; i < PIANO_KEY_NUM; ++i){
		pk = &(context->pianoKeys[i]);
	
		if ((keyTextSurface = TTF_RenderText_Solid(context->font, pk->keyName, colorWhite)) == NULL) {
			SDL_Log("Unable to create key name surface: %s, SDL_ttf Error: %s\n", pk->keyName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		if ((toneTextSurface = TTF_RenderText_Solid(context->font, pk->toneName, colorWhite)) == NULL) {
			SDL_Log("Unable to create tone name surface: %s, SDL_ttf Error: %s\n", pk->toneName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		if ((pk->keyNameTexture = SDL_CreateTextureFromSurface(context->renderer, keyTextSurface)) == NULL){
			SDL_Log("Unable to create key name texture: %s, SDL_ttf Error: %s\n", pk->keyName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		if ((pk->toneNameTexture = SDL_CreateTextureFromSurface(context->renderer, toneTextSurface)) == NULL){
			SDL_Log("Unable to create tone name texture: %s, SDL_ttf Error: %s\n", pk->toneName, TTF_GetError());
			exit(EXIT_FAILURE);
		}

		SDL_FreeSurface(keyTextSurface);
		SDL_FreeSurface(toneTextSurface);
	}
}

void init_audio(PianoContext* context){
	if (Mix_OpenAudio(DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, DEFAULT_CHANNEL_NUM, DEFAULT_CHUNK_SIZE) < 0) {
        	SDL_Log("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        	exit(EXIT_FAILURE);
    	}

	int i;
	PianoKey* pk;
	char resourcePath[RESOURCE_PATH_MAX_LEN];
	for (i = 0; i < PIANO_KEY_NUM; ++i){
		pk = &(context->pianoKeys[i]);

		sprintf(resourcePath, "./resources/%s.wav", pk->toneName);
		pk->chunk = Mix_LoadWAV(resourcePath);

		if (pk->chunk == NULL) {
			SDL_Log("Can't load sound resource: %s, error: %s\n", resourcePath, Mix_GetError());
			exit(EXIT_FAILURE);
		}
	}
}

PianoContext* init_context(void){
	PianoContext* context = (PianoContext*)malloc(sizeof(PianoContext));
	if (context == NULL){
		SDL_Log("init_context() failed, can't allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	init_graphics(context);
	init_pianoKeys(context);
	init_ttf(context);
	init_audio(context);

	return context;
}

void free_context(PianoContext* context){
	TTF_CloseFont(context->font);
	TTF_Quit();

	int i;
	for (i = 0; i < PIANO_KEY_NUM; ++i){
		Mix_FreeChunk(context->pianoKeys[i].chunk);
		SDL_DestroyTexture(context->pianoKeys[i].keyNameTexture);
		SDL_DestroyTexture(context->pianoKeys[i].toneNameTexture);
	}

	Mix_CloseAudio();
	Mix_Quit();

	SDL_DestroyRenderer(context->renderer);
	SDL_DestroyWindow(context->window);
	SDL_Quit();

	free(context);
}

PianoKey* get_piano_Key_mapping(PianoContext* context, SDL_KeyCode key){
	switch (key) {
		case SDLK_1: return &(context->pianoKeys[0]);
		case SDLK_3: return &(context->pianoKeys[1]);
		case SDLK_5: return &(context->pianoKeys[2]);
		case SDLK_6: return &(context->pianoKeys[3]);
		case SDLK_8: return &(context->pianoKeys[4]);
		case SDLK_0: return &(context->pianoKeys[5]);
		case SDLK_w: return &(context->pianoKeys[6]);
		case SDLK_e: return &(context->pianoKeys[7]);
		case SDLK_t: return &(context->pianoKeys[8]);
		case SDLK_u: return &(context->pianoKeys[9]);
		case SDLK_i: return &(context->pianoKeys[10]);
		case SDLK_p: return &(context->pianoKeys[11]);
		case SDLK_s: return &(context->pianoKeys[12]);
		case SDLK_f: return &(context->pianoKeys[13]);
		case SDLK_g: return &(context->pianoKeys[14]);
		case SDLK_j: return &(context->pianoKeys[15]);
		case SDLK_l: return &(context->pianoKeys[16]);
		case SDLK_z: return &(context->pianoKeys[17]);
		case SDLK_c: return &(context->pianoKeys[18]);
		case SDLK_b: return &(context->pianoKeys[19]);
		case SDLK_m: return &(context->pianoKeys[20]);
		case SDLK_2: return &(context->pianoKeys[21]);
		case SDLK_4: return &(context->pianoKeys[22]);
		case SDLK_7: return &(context->pianoKeys[23]);
		case SDLK_9: return &(context->pianoKeys[24]);
		case SDLK_q: return &(context->pianoKeys[25]);
		case SDLK_r: return &(context->pianoKeys[26]);
		case SDLK_y: return &(context->pianoKeys[27]);
		case SDLK_o: return &(context->pianoKeys[28]);
		case SDLK_a: return &(context->pianoKeys[29]);
		case SDLK_d: return &(context->pianoKeys[30]);
		case SDLK_h: return &(context->pianoKeys[31]);
		case SDLK_k: return &(context->pianoKeys[32]);
		case SDLK_x: return &(context->pianoKeys[33]);
		case SDLK_v: return &(context->pianoKeys[34]);
		case SDLK_n: return &(context->pianoKeys[35]);
		default: return NULL;
	}
}

void render_piano_keys(PianoContext* context){
	int i, textWidth, textHeight;
	PianoKey* pk;
	SDL_Rect rect = { 0, 0, WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT };

	SDL_RenderClear(context->renderer);

	/* render white keys. */
	for (i = 0; i < WHITE_KEY_NUM; ++i){
		pk = &(context->pianoKeys[i]);
		rect.x = pk->initX;

		if (pk->isPressed){
			/* simulate the effect of pressing piano keys in real life by changing the color of the keys. */
			SDL_SetRenderDrawColor(context->renderer, COLOR_RGBA_MIKU);	
		}
		else {
			SDL_SetRenderDrawColor(context->renderer, COLOR_RGBA_WHITE);	
		}

		SDL_RenderFillRect(context->renderer, &rect);

		/* render text. */
		SDL_QueryTexture(pk->keyNameTexture, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect keyNameRect = { pk->initX + WHITE_KEY_WIDTH / 2 - textWidth / 2, WHITE_KEY_HEIGHT - KEY_NAME_DISTANCE, textWidth, textHeight };
		SDL_RenderCopy(context->renderer, pk->keyNameTexture, NULL, &keyNameRect);

		SDL_QueryTexture(pk->toneNameTexture, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect toneNameRect = { pk->initX + WHITE_KEY_WIDTH / 2 - textWidth / 2, WHITE_KEY_HEIGHT - TONE_NAME_DISTANCE, textWidth, textHeight };
		SDL_RenderCopy(context->renderer, pk->toneNameTexture, NULL, &toneNameRect);
	}

	/* render lines. */
	SDL_SetRenderDrawColor(context->renderer, COLOR_RGBA_BLACK);
	for (i = 0; i < WHITE_KEY_NUM; ++i){
		SDL_RenderDrawLine(context->renderer, i * WHITE_KEY_WIDTH, 0, i * WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT);
	}

	/* render black keys. */
	rect.w = BLACK_KEY_WIDTH;
	rect.h = BLACK_KEY_HEIGHT;
	for (i = WHITE_KEY_NUM; i < PIANO_KEY_NUM; ++i){
		pk = &(context->pianoKeys[i]);
		rect.x = pk->initX;
	
		if (pk->isPressed){
			SDL_SetRenderDrawColor(context->renderer, COLOR_RGBA_MIKU);	
		}
		else {
			SDL_SetRenderDrawColor(context->renderer, COLOR_RGBA_BLACK);	
		}
	
		SDL_RenderFillRect(context->renderer, &rect);

		SDL_QueryTexture(pk->keyNameTexture, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect keyNameRect = { pk->initX + BLACK_KEY_WIDTH / 2 - textWidth / 2, BLACK_KEY_HEIGHT - KEY_NAME_DISTANCE, textWidth, textHeight };
		SDL_RenderCopy(context->renderer, pk->keyNameTexture, NULL, &keyNameRect);

		SDL_QueryTexture(pk->toneNameTexture, NULL, NULL, &textWidth, &textHeight);
		SDL_Rect toneNameRect = { pk->initX + BLACK_KEY_WIDTH / 2 - textWidth / 2, BLACK_KEY_HEIGHT - TONE_NAME_DISTANCE, textWidth, textHeight };
		SDL_RenderCopy(context->renderer, pk->toneNameTexture, NULL, &toneNameRect);
	}

	SDL_RenderPresent(context->renderer);
}

int main() {
	PianoContext* context = init_context();

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
                		pk = get_piano_Key_mapping(context, event.key.keysym.sym);

				if (pk != NULL){
					pk->isPressed = 1;

					/* disperse sound onto different channels based on the integer value of the pressed key. */
					Mix_PlayChannel(event.key.keysym.sym % DEFAULT_CHANNEL_NUM, pk->chunk, 0);
				}
            		} else if (event.type == SDL_KEYUP) {
                		pk = get_piano_Key_mapping(context, event.key.keysym.sym);

				if (pk != NULL){
					pk->isPressed = 0;
				}
            		}
        	}

		render_piano_keys(context);

		endTime = SDL_GetTicks();
        	frameTime = endTime - startTime;

        	if (frameTime < FRAME_DELAY) {
            		SDL_Delay(FRAME_DELAY - frameTime);
        	}
    	}

	free_context(context);
    	return 0;
}

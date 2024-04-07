#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <random>
#include <string>
#include <memory>
#include <array>

#undef main

// frame rate.
constexpr int FRAME_RATE = 60;
constexpr int FRAME_DELAY_MILLISEC = 1000 / FRAME_RATE;

// piano keys' attributes.
constexpr int BLACK_KEY_WIDTH  = 40;
constexpr int BLACK_KEY_HEIGHT = 254;
constexpr int WHITE_KEY_WIDTH  = 56;
constexpr int WHITE_KEY_HEIGHT = 390;

// C3 -> B5 tones.
constexpr int BLACK_KEY_NUM = 15;
constexpr int WHITE_KEY_NUM = 21;
constexpr int PIANO_KEY_NUM = BLACK_KEY_NUM + WHITE_KEY_NUM;

// window attributes.
const std::string WINDOW_TITLE = "Piano";
constexpr int WINDOW_HEIGHT = WHITE_KEY_HEIGHT;
constexpr int WINDOW_WIDTH = WHITE_KEY_WIDTH * WHITE_KEY_NUM;

// sound configurations.
constexpr int MIXER_DEFAULT_FREQUENCY    = 48000;
constexpr int MIXER_DEFAULT_CHANNEL_NUM  = 8;
constexpr int MIXER_DEFAULT_CHUNK_SIZE   = 2048;

const std::string SOUND_FILE_PATH = "./resources/";
const std::string SOUND_FILE_SUFFIX = ".Ogg";

// text configurations.
const std::string FONT_PATH = "./resources/arial.ttf";
constexpr int DEFAULT_FONT_SIZE  = 15;
constexpr int KEY_NAME_DISTANCE  = 22;
constexpr int TONE_NAME_DISTANCE = 42;

// colors.
const SDL_Color COLOR_WHITE = { 255, 255, 255, 255 };
const SDL_Color COLOR_BLACK = {   0,   0,   0, 255 };
const SDL_Color COLOR_MIKU  = {  57, 197, 187, 255 };

/**
 * SDL2 raii class, used to init the sdl, ttf, mixer.
*/
class SDLEnvWrapper {
    bool all_right = true;
public:
    SDLEnvWrapper() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0){
            SDL_Log("SDL_Init() failed: %s\n", SDL_GetError());
            all_right = false;
        }

        if (TTF_Init() == -1){
            SDL_Log("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        	all_right = false;
        }

        if (Mix_OpenAudio(MIXER_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIXER_DEFAULT_CHANNEL_NUM, MIXER_DEFAULT_CHUNK_SIZE) < 0) {
        	SDL_Log("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        	all_right = false;
    	}
    }

    ~SDLEnvWrapper() noexcept {
        Mix_CloseAudio();
        Mix_Quit();
	    TTF_Quit();
	    SDL_Quit();
    }

    bool valid() const noexcept {
        return all_right;
    }
};

enum class KeyType {
    Black, White
};

inline int setRenderDrawColor(SDL_Renderer* renderer, SDL_Color const& color) {
    return SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

class Key {
    KeyType type;
    std::string keyName;
    std::string toneName;
    Mix_Chunk* chunk = nullptr;
    bool pressed = false;
    int initX;

    bool loadSound() noexcept {
        std::string soundPath = SOUND_FILE_PATH + toneName + SOUND_FILE_SUFFIX;

        SDL_RWops *rw = SDL_RWFromFile(soundPath.c_str(), "rb");
	    if (rw == nullptr) {
		    SDL_Log("SDL_RWFromFile() failed on: %s, error: %s\n", soundPath, SDL_GetError());
		    return false;
	    }

	    // the 2nd parameter of the Mix_LoadWAV_RW() will free the rw automatically.
	    if ((chunk = Mix_LoadWAV_RW(rw, 1)) == nullptr){
		    SDL_Log("Can't load sound resource: %s, error: %s\n", soundPath, Mix_GetError());
		    return false;
	    }

        return true;
    }

    bool renderText(SDL_Renderer* renderer, TTF_Font* font, SDL_Color const& color, int width, int height) noexcept {
        bool succeed = true;
        SDL_Surface* keyNameSurface, *toneNameSurface;
	    SDL_Texture* keyNameTexture, *toneNameTexture;
        SDL_Rect keyNameRect;
        SDL_Rect toneNameRect;

        if ((keyNameSurface = TTF_RenderText_Solid(font, keyName.c_str(), color)) == nullptr) {
            SDL_Log("Unable to create key name surface: %s, SDL_ttf Error: %s\n", keyName.c_str(), TTF_GetError());
            succeed = false;
            goto clean_surface_texture;
        }

        if ((toneNameSurface = TTF_RenderText_Solid(font, toneName.c_str(), color)) == nullptr) {
            SDL_Log("Unable to create tone name surface: %s, SDL_ttf Error: %s\n", toneName.c_str(), TTF_GetError());
            succeed = false;
            goto clean_surface_texture;
        }

        if ((keyNameTexture = SDL_CreateTextureFromSurface(renderer, keyNameSurface)) == nullptr){
            SDL_Log("Unable to create key name texture: %s, SDL_ttf Error: %s\n", keyName.c_str(), TTF_GetError());
            succeed = false;
            goto clean_surface_texture;
        }

        if ((toneNameTexture = SDL_CreateTextureFromSurface(renderer, toneNameSurface)) == nullptr){
            SDL_Log("Unable to create tone name texture: %s, SDL_ttf Error: %s\n", toneName.c_str(), TTF_GetError());
            succeed = false;
            goto clean_surface_texture;
        }
        
        keyNameRect.x = initX + width / 2 - keyNameSurface->w / 2;
        keyNameRect.y = height - KEY_NAME_DISTANCE;
        keyNameRect.w = keyNameSurface->w;
        keyNameRect.h = keyNameSurface->h;
        
        toneNameRect.x = initX + width / 2 - toneNameSurface->w / 2;
        toneNameRect.y = height - TONE_NAME_DISTANCE;
        toneNameRect.w = toneNameSurface->w;
        toneNameRect.h = toneNameSurface->h;

        SDL_RenderCopy(renderer, keyNameTexture, nullptr, &keyNameRect);
        SDL_RenderCopy(renderer, toneNameTexture, nullptr, &toneNameRect);

    clean_surface_texture:
        if (keyNameSurface != nullptr){
            SDL_FreeSurface(keyNameSurface);
        }

        if (toneNameSurface != nullptr){
            SDL_FreeSurface(toneNameSurface);
        }

        if (keyNameTexture != nullptr){
            SDL_DestroyTexture(keyNameTexture);
        }

        if (toneNameTexture != nullptr){
            SDL_DestroyTexture(toneNameTexture);
        }
        
        return succeed;
    }
public:
    Key(){}

    Key(KeyType _type, std::string const& _keyName, std::string const& _toneName, int _initX)
        : type{ _type }, keyName{ _keyName }, toneName{ _toneName }, initX{ _initX }
    {}

    ~Key() noexcept {
        if (chunk != nullptr) {
            Mix_FreeChunk(chunk);
        }
    }

    void setPressed(bool isPressed) noexcept {
        pressed = isPressed;
    }

    bool playSound(int channel) noexcept {
        if (chunk == nullptr) {
            if (!loadSound()){
                return false;
            }
        }

        Mix_PlayChannel(channel, chunk, 0);
        return true;
    }

    bool render(SDL_Renderer* renderer, TTF_Font* font) noexcept {
        SDL_Rect rect;
        SDL_Color textColor;

        rect.x = initX;
        rect.y = 0;

        if (type == KeyType::Black){
            rect.w = BLACK_KEY_WIDTH;
            rect.h = BLACK_KEY_HEIGHT;
            textColor = COLOR_WHITE;

            if (pressed){
                setRenderDrawColor(renderer, COLOR_MIKU);
            }
            else {
                setRenderDrawColor(renderer, COLOR_BLACK);
            }
        }
        else {
            rect.w = WHITE_KEY_WIDTH;
            rect.h = WHITE_KEY_HEIGHT;
            textColor = COLOR_BLACK;
            
            if (pressed){
                setRenderDrawColor(renderer, COLOR_MIKU);
            }
            else {
                setRenderDrawColor(renderer, COLOR_WHITE);
            }
        }

        // filled rect.
        SDL_RenderFillRect(renderer, &rect);

        // outlined rect.
        setRenderDrawColor(renderer, COLOR_BLACK);
        SDL_RenderDrawRect(renderer, &rect);

        return renderText(renderer, font, textColor, rect.w, rect.h);
    }
};

class Piano {
    bool all_right = true;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;
    std::array<Key, PIANO_KEY_NUM> keys;

    void initResource() noexcept {
        window = SDL_CreateWindow(WINDOW_TITLE.c_str(), 
								SDL_WINDOWPOS_CENTERED, 
								SDL_WINDOWPOS_CENTERED, 
								WINDOW_WIDTH, 
								WINDOW_HEIGHT, 
								0);
							
	    if (window == nullptr){
		    SDL_Log("create window failed: %s\n", SDL_GetError());
		    all_right = false;
	    }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        if (renderer == nullptr){
            SDL_Log("create renderer failed: %s\n", SDL_GetError());
            all_right = false;
        }

        font = TTF_OpenFont(FONT_PATH.c_str(), DEFAULT_FONT_SIZE);
    	if (font == nullptr) {
        	SDL_Log("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
            all_right = false;
    	}
    }

    void initKeys() noexcept {
        keys[0]  = Key{ KeyType::White, "1", "C3",  WHITE_KEY_WIDTH * 0 };
        keys[1]  = Key{ KeyType::White, "3", "D3",  WHITE_KEY_WIDTH * 1 };
        keys[2]  = Key{ KeyType::White, "5", "E3",  WHITE_KEY_WIDTH * 2 };
        keys[3]  = Key{ KeyType::White, "6", "F3",  WHITE_KEY_WIDTH * 3 };
        keys[4]  = Key{ KeyType::White, "8", "G3",  WHITE_KEY_WIDTH * 4 };
        keys[5]  = Key{ KeyType::White, "0", "A3",  WHITE_KEY_WIDTH * 5 };
        keys[6]  = Key{ KeyType::White, "W", "B3",  WHITE_KEY_WIDTH * 6 };
        keys[7]  = Key{ KeyType::White, "E", "C4",  WHITE_KEY_WIDTH * 7 };
        keys[8]  = Key{ KeyType::White, "T", "D4",  WHITE_KEY_WIDTH * 8 };
        keys[9]  = Key{ KeyType::White, "U", "E4",  WHITE_KEY_WIDTH * 9 };
        keys[10] = Key{ KeyType::White, "I", "F4",  WHITE_KEY_WIDTH * 10 };
        keys[11] = Key{ KeyType::White, "P", "G4",  WHITE_KEY_WIDTH * 11 };
        keys[12] = Key{ KeyType::White, "S", "A4",  WHITE_KEY_WIDTH * 12 };
        keys[13] = Key{ KeyType::White, "F", "B4",  WHITE_KEY_WIDTH * 13 };
        keys[14] = Key{ KeyType::White, "G", "C5",  WHITE_KEY_WIDTH * 14 };
        keys[15] = Key{ KeyType::White, "J", "D5",  WHITE_KEY_WIDTH * 15 };
        keys[16] = Key{ KeyType::White, "L", "E5",  WHITE_KEY_WIDTH * 16 };
        keys[17] = Key{ KeyType::White, "Z", "F5",  WHITE_KEY_WIDTH * 17 };
        keys[18] = Key{ KeyType::White, "C", "G5",  WHITE_KEY_WIDTH * 18 };
        keys[19] = Key{ KeyType::White, "B", "A5",  WHITE_KEY_WIDTH * 19 };
        keys[20] = Key{ KeyType::White, "M", "B5",  WHITE_KEY_WIDTH * 20 };
        keys[21] = Key{ KeyType::Black, "2", "Db3", WHITE_KEY_WIDTH * 1 - BLACK_KEY_WIDTH / 2 };
        keys[22] = Key{ KeyType::Black, "4", "Eb3", WHITE_KEY_WIDTH * 2 - BLACK_KEY_WIDTH / 2 };
        keys[23] = Key{ KeyType::Black, "7", "Gb3", WHITE_KEY_WIDTH * 4 - BLACK_KEY_WIDTH / 2 };
        keys[24] = Key{ KeyType::Black, "9", "Ab3", WHITE_KEY_WIDTH * 5 - BLACK_KEY_WIDTH / 2 };
        keys[25] = Key{ KeyType::Black, "Q", "Bb3", WHITE_KEY_WIDTH * 6 - BLACK_KEY_WIDTH / 2 };
        keys[26] = Key{ KeyType::Black, "R", "Db4", WHITE_KEY_WIDTH * 8 - BLACK_KEY_WIDTH / 2 };
        keys[27] = Key{ KeyType::Black, "Y", "Eb4", WHITE_KEY_WIDTH * 9 - BLACK_KEY_WIDTH / 2 };
        keys[28] = Key{ KeyType::Black, "O", "Gb4", WHITE_KEY_WIDTH * 11 - BLACK_KEY_WIDTH / 2 };
        keys[29] = Key{ KeyType::Black, "A", "Ab4", WHITE_KEY_WIDTH * 12 - BLACK_KEY_WIDTH / 2 };
        keys[30] = Key{ KeyType::Black, "D", "Bb4", WHITE_KEY_WIDTH * 13 - BLACK_KEY_WIDTH / 2 };
        keys[31] = Key{ KeyType::Black, "H", "Db5", WHITE_KEY_WIDTH * 15 - BLACK_KEY_WIDTH / 2 };
        keys[32] = Key{ KeyType::Black, "K", "Eb5", WHITE_KEY_WIDTH * 16 - BLACK_KEY_WIDTH / 2 };
        keys[33] = Key{ KeyType::Black, "X", "Gb5", WHITE_KEY_WIDTH * 18 - BLACK_KEY_WIDTH / 2 };
        keys[34] = Key{ KeyType::Black, "V", "Ab5", WHITE_KEY_WIDTH * 19 - BLACK_KEY_WIDTH / 2 };
        keys[35] = Key{ KeyType::Black, "N", "Bb5", WHITE_KEY_WIDTH * 20 - BLACK_KEY_WIDTH / 2 };
    }

    Key* getKeyMapping(SDL_Keycode key){
        switch (key) {
            case SDLK_1: return &(keys[0]);
            case SDLK_3: return &(keys[1]);
            case SDLK_5: return &(keys[2]);
            case SDLK_6: return &(keys[3]);
            case SDLK_8: return &(keys[4]);
            case SDLK_0: return &(keys[5]);
            case SDLK_w: return &(keys[6]);
            case SDLK_e: return &(keys[7]);
            case SDLK_t: return &(keys[8]);
            case SDLK_u: return &(keys[9]);
            case SDLK_i: return &(keys[10]);
            case SDLK_p: return &(keys[11]);
            case SDLK_s: return &(keys[12]);
            case SDLK_f: return &(keys[13]);
            case SDLK_g: return &(keys[14]);
            case SDLK_j: return &(keys[15]);
            case SDLK_l: return &(keys[16]);
            case SDLK_z: return &(keys[17]);
            case SDLK_c: return &(keys[18]);
            case SDLK_b: return &(keys[19]);
            case SDLK_m: return &(keys[20]);
            case SDLK_2: return &(keys[21]);
            case SDLK_4: return &(keys[22]);
            case SDLK_7: return &(keys[23]);
            case SDLK_9: return &(keys[24]);
            case SDLK_q: return &(keys[25]);
            case SDLK_r: return &(keys[26]);
            case SDLK_y: return &(keys[27]);
            case SDLK_o: return &(keys[28]);
            case SDLK_a: return &(keys[29]);
            case SDLK_d: return &(keys[30]);
            case SDLK_h: return &(keys[31]);
            case SDLK_k: return &(keys[32]);
            case SDLK_x: return &(keys[33]);
            case SDLK_v: return &(keys[34]);
            case SDLK_n: return &(keys[35]);
            default: return nullptr;
        }
    }

    bool render() noexcept {
        setRenderDrawColor(renderer, COLOR_BLACK);
        SDL_RenderClear(renderer);

        /* render white keys. */
        for (int i = 0; i < WHITE_KEY_NUM; ++i) {
            if (!keys[i].render(renderer, font)){
                return false;
            }
        }

        /* render lines. */
        setRenderDrawColor(renderer, COLOR_BLACK);
        for (int i = 0; i < WHITE_KEY_NUM; ++i){
            SDL_RenderDrawLine(renderer, i * WHITE_KEY_WIDTH, 0, i * WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT);
        }

        /* render black keys. */
        for (int i = WHITE_KEY_NUM; i < PIANO_KEY_NUM; ++i){
            if (!keys[i].render(renderer, font)){
                return false;
            }
        }

        SDL_RenderPresent(renderer);
        return true;
    }
public:
    Piano() {
        initResource();
        initKeys();
    }

    ~Piano() noexcept {
        if (font != nullptr) {
            TTF_CloseFont(font);
        }

        if (renderer != nullptr) {
		    SDL_DestroyRenderer(renderer);
	    }
	
	    if (window != nullptr) {
		    SDL_DestroyWindow(window);
	    }
    }

    bool valid() const noexcept {
        return all_right;
    }

    void start() noexcept {
        Uint32 startTime, endTime, frameTime;
        bool running = true;
        SDL_Event event;

        while (running) {
		    startTime = SDL_GetTicks();

		    while (SDL_PollEvent(&event)) {
			    if (event.type == SDL_QUIT) {
				    running = false;
			    } else if (event.type == SDL_KEYDOWN) {
                    Key* key = getKeyMapping(event.key.keysym.sym);

                    if (key != nullptr) {
                        key->setPressed(true);
                        key->playSound(event.key.keysym.sym % MIXER_DEFAULT_CHANNEL_NUM);
                    }
			    } else if (event.type == SDL_KEYUP) {
				    Key* key = getKeyMapping(event.key.keysym.sym);

                    if (key != nullptr) {
                        key->setPressed(false);
                    }
			    }
		    }

            if (!render()){
                return;
            }

            endTime = SDL_GetTicks();
            frameTime = endTime - startTime;

            if (frameTime < FRAME_DELAY_MILLISEC) {
                SDL_Delay(FRAME_DELAY_MILLISEC - frameTime);
            }
        }
    }
};

int main(){
    SDLEnvWrapper env;
    if (!env.valid()){
        return 1;
    }

    auto piano = std::make_unique<Piano>();
    if (!piano->valid()){
        return 1;
    }

    piano->start();
    return 0;
}

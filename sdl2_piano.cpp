#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <algorithm>
#include <exception>
#include <string>
#include <memory>
#include <array>

#undef main

using namespace std::string_literals;

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

enum class KeyType {
    Black, White
};

inline int set_render_draw_color(SDL_Renderer* renderer, SDL_Color const& color) {
    return SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

class TextResource {
    SDL_Surface* surface = nullptr;
    SDL_Texture* texture = nullptr;
public:
    TextResource(){}

    ~TextResource(){
        if (surface != nullptr){
            SDL_FreeSurface(surface);
        }

        if (texture != nullptr){
            SDL_DestroyTexture(texture);
        }
    }

    void create_text(SDL_Renderer* renderer, TTF_Font* font, std::string const& text, SDL_Color const& color) {
        if ((surface = TTF_RenderText_Solid(font, text.c_str(), color)) == nullptr) {
            throw std::runtime_error{ "Unable to create key name surface: "s + text + ", SDL_ttf Error: "s + TTF_GetError() };
        }

        if ((texture = SDL_CreateTextureFromSurface(renderer, surface)) == nullptr){
            throw std::runtime_error { "Unable to create key name texture: "s + text + ", SDL_ttf Error: "s + TTF_GetError() };
        }
    }

    SDL_Surface* get_surface() noexcept {
        return surface;
    }

    SDL_Texture* get_texture() noexcept{
        return texture;
    } 
};

class Key {
    KeyType type;
    std::string keyName;
    std::string toneName;
    Mix_Chunk* chunk = nullptr;
    bool pressed = false;
    int initX;

    void load_sound() {
        std::string soundPath = SOUND_FILE_PATH + toneName + SOUND_FILE_SUFFIX;

        SDL_RWops *rw = SDL_RWFromFile(soundPath.c_str(), "rb");
	    if (rw == nullptr) {
		    throw std::runtime_error { "SDL_RWFromFile() failed on: "s + soundPath + ", error: "s + SDL_GetError() };
	    }

	    // the 2nd parameter of the Mix_LoadWAV_RW() will free the rw automatically.
	    if ((chunk = Mix_LoadWAV_RW(rw, 1)) == nullptr){
		    throw std::runtime_error { "Can't load sound resource: "s + soundPath + ", error: "s + Mix_GetError() };
	    }
    }

    void render_text(SDL_Renderer* renderer, TTF_Font* font, SDL_Color const& color, int width, int height) noexcept {
        SDL_Rect keyNameRect, toneNameRect;
        TextResource keyNameText, toneNameText;

        keyNameText.create_text(renderer, font, keyName, color);
        toneNameText.create_text(renderer, font, toneName, color);
        
        SDL_Surface* keyNameSurface = keyNameText.get_surface();
        SDL_Surface* toneNameSurface = toneNameText.get_surface();

        keyNameRect.x = initX + width / 2 - keyNameSurface->w / 2;
        keyNameRect.y = height - KEY_NAME_DISTANCE;
        keyNameRect.w = keyNameSurface->w;
        keyNameRect.h = keyNameSurface->h;
        
        toneNameRect.x = initX + width / 2 - toneNameSurface->w / 2;
        toneNameRect.y = height - TONE_NAME_DISTANCE;
        toneNameRect.w = toneNameSurface->w;
        toneNameRect.h = toneNameSurface->h;

        SDL_RenderCopy(renderer, keyNameText.get_texture(), nullptr, &keyNameRect);
        SDL_RenderCopy(renderer, toneNameText.get_texture(), nullptr, &toneNameRect);
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

    void set_pressed(bool isPressed) noexcept {
        pressed = isPressed;
    }

    void play_sound(int channel) {
        if (chunk == nullptr) {
            load_sound();
        }

        Mix_PlayChannel(channel, chunk, 0);
    }

    void render(SDL_Renderer* renderer, TTF_Font* font) {
        SDL_Rect rect;
        SDL_Color textColor;

        rect.x = initX;
        rect.y = 0;

        if (type == KeyType::Black){
            rect.w = BLACK_KEY_WIDTH;
            rect.h = BLACK_KEY_HEIGHT;
            textColor = COLOR_WHITE;

            if (pressed){
                set_render_draw_color(renderer, COLOR_MIKU);
            }
            else {
                set_render_draw_color(renderer, COLOR_BLACK);
            }
        }
        else {
            rect.w = WHITE_KEY_WIDTH;
            rect.h = WHITE_KEY_HEIGHT;
            textColor = COLOR_BLACK;
            
            if (pressed){
                set_render_draw_color(renderer, COLOR_MIKU);
            }
            else {
                set_render_draw_color(renderer, COLOR_WHITE);
            }
        }

        // filled rect.
        SDL_RenderFillRect(renderer, &rect);

        // outlined rect.
        set_render_draw_color(renderer, COLOR_BLACK);
        SDL_RenderDrawRect(renderer, &rect);

        render_text(renderer, font, textColor, rect.w, rect.h);
    }
};

class Piano {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;
    std::array<Key, PIANO_KEY_NUM> keys;

    void init_graphics_ttf_mixer(){
        if (SDL_Init(SDL_INIT_VIDEO) < 0){
            throw std::runtime_error { "SDL_Init() failed: "s + SDL_GetError() };
        }

        if (TTF_Init() == -1){
            throw std::runtime_error { "SDL_ttf could not initialize! SDL_ttf Error: "s + TTF_GetError() };
        }

        if (Mix_OpenAudio(MIXER_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIXER_DEFAULT_CHANNEL_NUM, MIXER_DEFAULT_CHUNK_SIZE) < 0) {
        	throw std::runtime_error { "SDL_mixer could not initialize! SDL_mixer Error: "s + Mix_GetError() };
    	}
    }

    void init_resources() {
        window = SDL_CreateWindow(WINDOW_TITLE.c_str(), 
								SDL_WINDOWPOS_CENTERED, 
								SDL_WINDOWPOS_CENTERED, 
								WINDOW_WIDTH, 
								WINDOW_HEIGHT, 
								0);
							
	    if (window == nullptr){
		    throw std::runtime_error { "create window failed: "s + SDL_GetError() };
	    }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        if (renderer == nullptr){
            throw std::runtime_error{ "create renderer failed: "s + SDL_GetError() };
        }

        font = TTF_OpenFont(FONT_PATH.c_str(), DEFAULT_FONT_SIZE);
    	if (font == nullptr) {
        	throw std::runtime_error{ "Failed to load font! SDL_ttf Error: "s + TTF_GetError() };
    	}
    }

    void init_keys() noexcept {
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

    Key* get_key_mapping(SDL_Keycode key) {
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

    void render() {
        set_render_draw_color(renderer, COLOR_BLACK);
        SDL_RenderClear(renderer);

        // render white keys.
        for (int i = 0; i < WHITE_KEY_NUM; ++i) {
            keys[i].render(renderer, font);
        }

        // render lines.
        set_render_draw_color(renderer, COLOR_BLACK);
        for (int i = 0; i < WHITE_KEY_NUM; ++i){
            SDL_RenderDrawLine(renderer, i * WHITE_KEY_WIDTH, 0, i * WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT);
        }

        // render black keys.
        for (int i = WHITE_KEY_NUM; i < PIANO_KEY_NUM; ++i){
            keys[i].render(renderer, font);
        }

        SDL_RenderPresent(renderer);
    }
public:
    Piano() {}

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

        Mix_CloseAudio();
        Mix_Quit();
        TTF_Quit();
        SDL_Quit();        
    }

    void start() {
        init_graphics_ttf_mixer();
        init_resources();
        init_keys();

        Uint32 startTime, endTime, frameTime;
        bool running = true;
        SDL_Event event;

        while (running) {
		startTime = SDL_GetTicks();

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			} else if (event.type == SDL_KEYDOWN) {
                    		Key* key = get_key_mapping(event.key.keysym.sym);

                    		if (key != nullptr) {
                        		key->set_pressed(true);
                        		key->play_sound(event.key.keysym.sym % MIXER_DEFAULT_CHANNEL_NUM);
                    		}
			} else if (event.type == SDL_KEYUP) {
				Key* key = get_key_mapping(event.key.keysym.sym);

                    		if (key != nullptr) {
                        		key->set_pressed(false);
                    		}
			}
		}

		render();

            	endTime = SDL_GetTicks();
            	frameTime = endTime - startTime;

            	if (frameTime < FRAME_DELAY_MILLISEC) {
                	SDL_Delay(FRAME_DELAY_MILLISEC - frameTime);
            	}
	}
    }
};

int main(){
    try {
        auto piano = std::make_unique<Piano>();
        piano->start();
    }
    catch(std::exception const& e){
        std::cerr << e.what() << "\n";
    }
    
    return 0;
}

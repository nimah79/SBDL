/**
* SBDL: Sadegh & Borjian Directmedia Layer!
*/

#include <string>

#if defined(_WIN32) || defined(_WIN64) // Windows
#pragma once
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"

#elif defined(__linux__) // Linux
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL_mixer.h"

#else // MacOS
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#endif
#undef main

/**
 * Represent a Sound
 */
using Sound = Mix_Chunk;

/**
 * Represent a Music
 */
using Music = Mix_Music;

/**
 * Represent a Font
 */
using Font = TTF_Font;

/**
 * Texture living on the graphics card that can be used for drawing.
 */
struct Texture {
    /**
     * Underneath texture which is loaded with SDL.
     * Don't use it directly in your code.
     */
    SDL_Texture *underneathTexture = nullptr;

    /**
     * Width of Texture
     */
    int width;

    /**
     * Height of Texture
     */
    int height;

    /**
     * alpha map of Texture
     */
    int* alphaMap = nullptr;
};

namespace SBDL {
    /**
     * Get alpha channel of a pixel of a surface
     * @param surface source surface
     * @param x x coordinate of pixel
     * @param y y coordinate of pixel
     * @return alpha channel value of pixel
     */
    int getAlpha(SDL_Surface *surface, int x, int y)
    {
        int bpp = surface->format->BytesPerPixel;
        Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
        Uint32 pixelColor;
        switch(bpp) {
            case 1:
                pixelColor = *p;
                break;
            case 2:
                pixelColor = *(Uint16*)p;
                break;
            case 3:
                if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
                    pixelColor = p[0] << 16 | p[1] << 8 | p[2];
                else
                    pixelColor = p[0] | p[1] << 8 | p[2] << 16;
                break;
            case 4:
                pixelColor = *(Uint32*)p;
                break;
        }
        Uint8 red, green, blue, alpha;
        SDL_GetRGBA(pixelColor, surface->format, &red, &green, &blue, &alpha);
        return alpha;
    }

    /**
     * Don't import this namespace.
     * This namespace is used to handle underneath SDL functions.
     */
    namespace Core {
        /**
         * current state of SDL
         */
        bool running = true;

        /**
         * SDL keyboard state array size
         */
        int keystate_size = -1;

        /**
         * SDL current keyboard state
         */
        const Uint8 *keystate = nullptr;

        /**
         * SDL last keyboard state
         */
        Uint8 *old_keystate = nullptr;

        /**
         * SDL current event
         */
        SDL_Event event;

        /**
         * SDL windows
         */
        SDL_Window *window = nullptr;

        /**
         * SDL renderer
         */
        SDL_Renderer *renderer = nullptr;

        /**
         * Create texture with given features
         * @param path path of texture
         * @param changeColor true if given color must be replaced with transparent color
         * @param r red color
         * @param g green color
         * @param b blue color
         * @param alpha transparency level
         * @return texture which is created
         */
        Texture loadTextureUnderneath(const std::string &path, bool changeColor, Uint8 r, Uint8 g, Uint8 b,
            Uint8 alpha = 255) {
            // Check existence of image
            SDL_Surface *pic = IMG_Load(path.c_str());
            if (pic == nullptr) {
                const std::string message = "Missing Image file: " + path;
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SBDL load image error", message.c_str(), nullptr);
                exit(1);
            }

            if (changeColor)
                SDL_SetColorKey(pic, SDL_TRUE, SDL_MapRGB(pic->format, r, g, b));
            if (alpha != 255)
                SDL_SetSurfaceAlphaMod(pic, alpha);

            Texture newTexture;
            newTexture.underneathTexture = SDL_CreateTextureFromSurface(renderer, pic);
            newTexture.width = pic->w;
            newTexture.height = pic->h;
            newTexture.alphaMap = new int[pic->w * pic->h];
            for (int i = 0; i < pic->h; ++i) {
                for (int j = 0; j < pic->w; ++j)
                    newTexture.alphaMap[i * pic->w + j] = SBDL::getAlpha(pic, j, i);
            }

            SDL_SetTextureBlendMode(newTexture.underneathTexture, SDL_BLENDMODE_BLEND);
            SDL_FreeSurface(pic);

            return newTexture;
        }
    }

    /**
     * Comparator of SDL_Rect
     */
    bool operator==(const SDL_Rect &x, const SDL_Rect &y) {
        return x.x == y.x && x.y == y.y && x.h == y.h && x.w == y.w;
    }

    /**
     * A structure which give useful information about mouse state.
     * updateEvents must be call before using this structure.
     */
    struct Mouse {
        /**
         * x position of Mouse
         */
        int x;

        /**
         * y position of Mouse
         */
        int y;

        /**
         * left button state
         */
        bool left;

        /**
         * right button state
         */
        bool right;

        /**
         * middle button state
         */
        bool middle;

        bool scroll_up;

        bool scroll_down;

        bool scroll_left;

        bool scroll_right;

        /**
         * State of Mouse  < SDL_PRESSED or SDL_RELEASED >
         */
        Uint8 state;

        /**
         * Button clicks
         */
        Uint8 clicks;

        /**
         * For SDL backward compatibility
         */
        Uint8 button;

        /**
         * Check if Mouse clicked with given conditions.
         * @param button button to check <SDL_BUTTON_LEFT,SDL_BUTTON_RIGHT,SDL_BUTTON_MIDDLE>
         * @param clicks number of clicks
         * @param state sate of mouse <SDL_PRESSED,SDL_RELEASED>
         */
        bool clicked(Uint8 button = SDL_BUTTON_LEFT, Uint8 clicks = 1, Uint8 state = SDL_PRESSED) {
            return this->button == button && this->clicks == clicks && this->state == state;
        }
    } Mouse;

    /**
     * Check state of program.
     * @return state of SDL
     */
    bool isRunning() {
        return Core::running;
    }

    /**
     * Stop SBDL and SBDL::isRunning will return false later.
     */
    void stop() {
        Core::running = false;
    }

    /**
     * Indicate whether a key with specific scanCode was pressed.
     * @param scanCode specific code for each keyboard button (https://wiki.libsdl.org/SDL_Scancode)
     * @return true if specific keyboard button was pressed
     */
    bool keyPressed(SDL_Scancode scanCode) {
        return !Core::old_keystate[scanCode] && Core::keystate[scanCode];
    }

    /**
     * Indicate whether a key with specific scanCode was released.
     * @param scanCode specific code for each keyboard button (https://wiki.libsdl.org/SDL_Scancode)
     * @return true if specific keyboard button was released
     */
    bool keyReleased(SDL_Scancode scanCode) {
        return Core::old_keystate[scanCode] && !Core::keystate[scanCode];
    }

    /**
     * Indicate whether a key with specific scanCode is hold.
     * @param scanCode specific code for each keyboard button (https://wiki.libsdl.org/SDL_Scancode)
     * @return true if specific keyboard button is hold
     */
    bool keyHeld(SDL_Scancode scanCode) {
        return Core::old_keystate[scanCode] && Core::keystate[scanCode];
    }

    /**
     * Initialize SDL and show a simple empty window for drawing texture on it.
     * Before start using SDL functions and types, initialize the engine.
     * @param windowsTitle title of window
     * @param windowsWidth width of window
     * @param windowsHeight height of window
     * @param r red color of default background
     * @param g green color of default background
     * @param b blue color of default background
     */
    void InitEngine(const std::string &windowsTitle, int windowsWidth, int windowsHeight,
        Uint8 r = 255, Uint8 g = 255, Uint8 b = 255) {
        atexit(SDL_Quit); // set a SDL_Quit as exit function
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SBDL initialization", "SBDL initialize video engine error",
                nullptr);
            exit(1);
        }

        SDL_CreateWindowAndRenderer(windowsWidth, windowsHeight, SDL_WINDOW_SHOWN, &Core::window, &Core::renderer);
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother
        SDL_RenderSetLogicalSize(Core::renderer, windowsWidth, windowsHeight);
        SDL_SetRenderDrawColor(Core::renderer, r, g, b, 255);
        SDL_SetRenderDrawBlendMode(Core::renderer, SDL_BLENDMODE_BLEND);

        SDL_SetWindowTitle(Core::window, windowsTitle.c_str());
        // inilialize SDL_mixer, exit if fail
        if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SBDL initialization", "SBDL initialize audio engine error",
                nullptr);
            exit(1);
        }

        // Setup audio mode
        Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640);
        // Setup text system
        TTF_Init();
    }

    /**
     * Update state of keyboard buttons (release or push) and mouse.
     * Call this function in a loop after initializing the engine to get updated states.
     */
    void updateEvents() {
        // update keyboard state
        if (Core::keystate_size == -1) {
            Core::keystate = SDL_GetKeyboardState(&Core::keystate_size);
            Core::old_keystate = new Uint8[Core::keystate_size];
            for (int i = 0; i < Core::keystate_size; i++)
                Core::old_keystate[i] = 0;
        }
        else {
            for (int i = 0; i < Core::keystate_size; i++)
                Core::old_keystate[i] = Core::keystate[i];
            Core::keystate = SDL_GetKeyboardState(&Core::keystate_size);
        }
        SDL_PumpEvents();

        // reset event handler state for check it again
        Core::event = {};

        // returns true if there is an event in the queue, but will not remove it
        if (!SDL_PollEvent(nullptr)) {
            Mouse.left = Mouse.middle = Mouse.right = Mouse.scroll_up = Mouse.scroll_down = Mouse.scroll_left = Mouse.scroll_right = false;
            Mouse.button = 0;
            return;
        }
        while (SDL_PollEvent(&Core::event)) { // loop until there is a new event for handling
            if (Core::event.type == SDL_MOUSEBUTTONDOWN || Core::event.type == SDL_MOUSEBUTTONUP) {
                // Update state of Mouse structure if it was changed.
                switch (Core::event.button.button) {
                case 1:
                    Mouse.left = true;
                    Mouse.right = Mouse.middle = false;
                    break;
                case 2:
                    Mouse.middle = true;
                    Mouse.right = Mouse.left = false;
                    break;
                case 3:
                    Mouse.right = true;
                    Mouse.left = Mouse.middle = false;
                    break;
                default:
                    Mouse.left = Mouse.middle = Mouse.right = false;
                }

                Mouse.state = Core::event.button.state;
                Mouse.button = Core::event.button.button;
                Mouse.clicks = Core::event.button.clicks;
            }
            else if (Core::event.type == SDL_MOUSEWHEEL) {
                Mouse.scroll_up = Core::event.wheel.y > 0;
                Mouse.scroll_down = Core::event.wheel.y < 0;
                Mouse.scroll_left = Core::event.wheel.x < 0;
                Mouse.scroll_right = Core::event.wheel.x > 0;
            }
            else if (Core::event.type == SDL_MOUSEMOTION) {
                // Update position of mouse if it was changed.
                Mouse.x = Core::event.motion.x;
                Mouse.y = Core::event.motion.y;
            }
            else if (Core::event.type == SDL_QUIT) {
                Core::running = false;
            }
        }
    }

    /**
     * Get milliseconds since program was started.
     */
    unsigned int getTime() {
        return SDL_GetTicks();
    }

    /**
     * Clear the current rendering target.
     */
    void clearRenderScreen() {
        SDL_RenderClear(Core::renderer);
    }

    /**
     * Update the screen and apply all changes.
     */
    void updateRenderScreen() {
        SDL_RenderPresent(Core::renderer);
    }

    /**
     * Wait before continuing process.
     * @param frameRate set the dalay (milisecond).
     */
    void delay(Uint32 frameRate) {
        SDL_Delay(frameRate);
    }

    /**
     * Load font from file
     * @param path path of the font file to load
     * @param size size of font
     * @return font which is loaded
     */
    Font *loadFont(const std::string &path, int size) {
        return TTF_OpenFont(path.c_str(), size);
    }

    /**
     * Load texture from file.
     * @param path path of the image file to load
     * @param alpha transparency level
     * @return texture which is loaded
     */
    Texture loadTexture(const std::string &path, Uint8 alpha = 255) {
        return Core::loadTextureUnderneath(path, false, 0, 0, 0, alpha);
    }

    /**
     * Load texture from file and replace transparency of image with specific color.
     * @param path path of the image file to load
     * @param r red color
     * @param g green color
     * @param b blue color
     * @param alpha transparency level
     * @return texture which is loaded
     */
    Texture loadTexture(const std::string &path, Uint8 r, Uint8 g, Uint8 b, Uint8 alpha = 255) {
        return Core::loadTextureUnderneath(path, true, r, g, b, alpha);
    }

    /**
    * Play sound
    * multiple sound can play concurrently
    * @param sound sound which is loaded before
    * @param count frequency of sound (-1 to play all time)
    * @see loadSound
    */
    void playSound(Sound *sound, int count = 1) {
        if (count != 0)
            Mix_PlayChannel(-1, sound, (count > 0) ? count - 1 : -1);
    }

    /**
     * Play music
     * only one music file can play
     * @param music music which is loaded before
     * @param count frequency of msuic (-1 to play all time)
     * @see loadMusic
     */
    void playMusic(Music *music, int count = -1) {
        Mix_PlayMusic(music, count);
    }

    /**
     * Load sound from a file in disk (use .wav)
     * @param path path of the sound file to load
     * @return sound which is loaded
     */
    Sound *loadSound(const std::string &path) {
        Sound *sound;
        sound = Mix_LoadWAV(path.c_str());
        if (!sound) {
            const std::string message = "Unable to load: " + path;
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SBDL load sound error", message.c_str(), nullptr);
            exit(1);
        }
        return sound;
    }

    /**
     * Load music from a file in disk (use .ogg or .wav)
     * @param path path of the music file to load
     * @return music which is loaded
     */
    Music *loadMusic(const std::string &path) {
        Music *music;
        music = Mix_LoadMUS(path.c_str());
        if (!music) {
            const std::string message = "Unable to load: " + path;
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SBDL load music error", message.c_str(), nullptr);
            exit(1);
        }
        return music;
    }

    /**
     * Stop music
     */
    void stopMusic() {
        Mix_HaltMusic();
    }

    void rewindMusic() {
        Mix_RewindMusic();
    }

    /**
     * Stop all sounds
     */
    void stopAllSounds() {
        Mix_HaltChannel(-1);
    }

    /**
     * Free memory which is used for load sound from file
     * @param sound Sound which you want to destroy
     */
    void freeSound(Sound *sound) {
        Mix_FreeChunk(sound);
    }

    /**
    * Free memory which is used for load music from file
    * @param music Music which you want to destroy
    */
    void freeMusic(Music *music) {
        Mix_FreeMusic(music);
    }

    /**
     * Free memory which is used for texture
     * After call this function, texture is not usable anymore and any using has undefined behavior
     * @param texture Texture which you want to destroy
     */
    void freeTexture(Texture &texture) {
        SDL_DestroyTexture(texture.underneathTexture);
        texture.underneathTexture = nullptr;
        texture.width = 0;
        texture.height = 0;
    }

    void showTexture(const Texture &texture, int x, int y, int width, int height) {
        SDL_Rect destRect = {x, y, width, height};
        SDL_RenderCopyEx(Core::renderer, texture.underneathTexture, nullptr, &destRect, 0, nullptr, SDL_FLIP_NONE);
    }

    /**
     * Texture showed in render screen in position destRect with angle and flip
     * @param texture the source texture
     * @param angle an angle in degrees that indicates the rotation
              that will be applied to texture, rotating it
              in a clockwise direction around center of texture
     * @param destRect custom rect to draw texture
     * @param flip flipping actions performed on the texture (SDL_FLIP_NONE or SDL_FLIP_HORIZONTAL or SDL_FLIP_VERTICAL)
     */
    void showTexture(const Texture &texture, double angle, const SDL_Rect &destRect,
        SDL_RendererFlip flip = SDL_FLIP_NONE) {
        SDL_RenderCopyEx(Core::renderer, texture.underneathTexture, nullptr, &destRect, angle, nullptr, flip);
    }

    /**
     * Texture showed in render screen in position texture.rect with angle and flip
     * @param texture the source texture
     * @param x position x
     * @param y position y
     * @param angle an angle in degrees that indicates the rotation that will be applied to texture, rotating it
              in a clockwise direction around center of texture
     * @param flip flipping actions performed on the texture (SDL_FLIP_NONE or SDL_FLIP_HORIZONTAL or SDL_FLIP_VERTICAL)
     */
    void showTexture(const Texture &texture, int x, int y, double angle, SDL_RendererFlip flip = SDL_FLIP_NONE) {
        SDL_Rect rect;
        rect.x = x;
        rect.y = y;
        rect.w = texture.width;
        rect.h = texture.height;
        showTexture(texture, angle, rect, flip);
    }

    /**
     * Texture showed in render screen in position destRect
     * @param texture the source texture
     * @param destRect custom rect to draw texture
     */
    void showTexture(const Texture &texture, const SDL_Rect &destRect) {
        SDL_RenderCopy(Core::renderer, texture.underneathTexture, nullptr, &destRect);
    }

    /**
     * Texture showed in render screen in position texture.rect
     * @param texture the source texture
     * @param x position x
     * @param y position y
     */
    void showTexture(const Texture &texture, int x, int y) {
        SDL_Rect rect;
        rect.x = x;
        rect.y = y;
        rect.w = texture.width;
        rect.h = texture.height;
        showTexture(texture, rect);
    }

    /**
     * Create a texture from a font for a special string with specific color whcih can be drawed in render window
     * @param font font which is loaded
     * @param text text that convert to texture
     * @param r red color
     * @param g green color
     * @param b blue color
     * @param highQuality enable high quality rendering (slower)
     * @return texture which created with that font and text
     */
    Texture createFontTexture(Font *font, const std::string &text, Uint8 r, Uint8 g, Uint8 b, bool highQuality = false) {
        SDL_Color color;
        color.r = r;
        color.g = g;
        color.b = b;
        SDL_Surface *temp;
        if (highQuality)
            temp = TTF_RenderText_Blended(font, text.c_str(), color);
        else
            temp = TTF_RenderText_Solid(font, text.c_str(), color);

        Texture newTexture;
        newTexture.underneathTexture = SDL_CreateTextureFromSurface(Core::renderer, temp);
        newTexture.width = temp->w;
        newTexture.height = temp->h;

        SDL_FreeSurface(temp);
        return newTexture;
    }

    /**
     * Draw rectangle on renderer screen.
     * @param rect rectangle position
     * @param r red color
     * @param g green color
     * @param b blue color
     * @param alpha transparency
     */
    void drawRectangle(const SDL_Rect &rect, Uint8 r, Uint8 g, Uint8 b, Uint8 alpha = 255) {
        Uint8 defaults[4];
        SDL_GetRenderDrawColor(Core::renderer, &defaults[0], &defaults[1], &defaults[2], &defaults[3]);
        SDL_SetRenderDrawColor(Core::renderer, r, g, b, alpha);
        SDL_RenderFillRect(Core::renderer, &rect);
        SDL_SetRenderDrawColor(Core::renderer, defaults[0], defaults[1], defaults[2], defaults[3]);
    }

    /**
     * Check intersection of two SDL_Rect
     * @param firstRect first rectangle
     * @param secondRect second rectangle
     * @return true if has intersection
     */
    bool hasIntersectionRect(const SDL_Rect &firstRect, const SDL_Rect &secondRect) {
        return SDL_HasIntersection(&firstRect, &secondRect) == SDL_TRUE;
    }

    /**
     * Bilinear resize single channel image
     * pixels is an array of size w * h
     * Target dimension is w2 * h2
     * w2 * h2 cannot be zero
     * @param pixels image pixels
     * @param w Image width
     * @param h Image height
     * @param w2 New width
     * @param h2 New height
     * @return New array with size w2 * h2
     */
    int *resizeBilinear(int *pixels, int w, int h, int w2, int h2) {
        int *temp = new int[w2 * h2];
        int A, B, C, D, index, y_index, xr, yr, alpha;
        long x, y = 0, x_diff, y_diff, one_min_x_diff, one_min_y_diff;
        int x_ratio = (int)(((w - 1) << 16) / w2);
        int y_ratio = (int)(((h - 1) << 16) / h2);
        int offset = 0;
        for (int i = 0; i < h2; i++) {
            yr = (int)(y >> 16);
            y_diff = y - (yr << 16);
            one_min_y_diff = 65536 - y_diff;
            y_index = yr * w;
            x = 0;
            for (int j = 0; j < w2; j++) {
                xr = (int)(x >> 16);
                x_diff = x - (xr << 16);
                one_min_x_diff = 65536 - x_diff;
                index = y_index + xr;
    
                // range is 0 to 255, thus bitwise AND with 0xff
                A = pixels[index] & 0xff;
                B = pixels[index + 1] & 0xff;
                C = pixels[index + w] & 0xff;
                D = pixels[index + w + 1] & 0xff;
    
                // Y = A(1-w)(1-h) + B(w)(1-h) + C(h)(1-w) + D(w)(h)
                alpha = (int)((
                        A * one_min_x_diff * one_min_y_diff +
                        B * x_diff * one_min_y_diff +
                        C * y_diff * one_min_x_diff +
                        D * x_diff * y_diff
                ) >> 32);
    
                temp[offset++] = alpha;
    
                x += x_ratio;
            }
            y += y_ratio;
        }
        return temp;
    }

    /**
     * Create bounding box of a rotated rectangle
     * @param rect source rectangle
     * @param angle angle of source rectangle
     * @return bounding box of source rectangle
     */
    SDL_Rect getRotatedBoundingBox(SDL_Rect rect, double angle) {
        int c_x = rect.x + rect.w / 2;
        int c_y = rect.y + rect.h / 2;
        rect.x -= c_x;
        rect.y -= c_y;
        double angle_sin = sin(angle * M_PI / 180);
        double angle_cos = cos(angle * M_PI / 180);
        int x1, x2, x3, x4, y1, y2, y3, y4;
        int x1_2, x2_2, x3_2, x4_2, y1_2, y2_2, y3_2, y4_2;
        x1 = x4 = rect.x;
        x2 = x3 = rect.x + rect.w;
        y1 = y2 = rect.y;
        y3 = y4 = rect.y + rect.h;
        x1_2 = ceil(y1 * angle_sin + x1 * angle_cos);
        y1_2 = ceil(y1 * angle_cos - x1 * angle_sin);
        x2_2 = ceil(y2 * angle_sin + x2 * angle_cos);
        y2_2 = ceil(y2 * angle_cos - x2 * angle_sin);
        x3_2 = ceil(y3 * angle_sin + x3 * angle_cos);
        y3_2 = ceil(y3 * angle_cos - x3 * angle_sin);
        x4_2 = ceil(y4 * angle_sin + x4 * angle_cos);
        y4_2 = ceil(y4 * angle_cos - x4 * angle_sin);
        x1 = x4 = std::min(x1_2, std::min(x2_2, std::min(x3_2, x4_2)));
        x2 = x3 = std::max(x1_2, std::max(x2_2, std::max(x3_2, x4_2)));
        y1 = y2 = std::min(y1_2, std::min(y2_2, std::min(y3_2, y4_2)));
        y3 = y4 = std::max(y1_2, std::max(y2_2, std::max(y3_2, y4_2)));
        SDL_Rect box;
        box.x = x1 + c_x;
        box.y = y1 + c_y;
        box.w = x2 - x1 + 1;
        box.h = y3 - y2 + 1;
        return box;
    }

    /**
     * Rotate an alpha map
     * @param alphaMap source alpha map
     * @param width width of source alpha map
     * @param height height of source alpha map
     * @param angle angle to rotate
     * @return rotated alpha map
     */
    int* getRotatedAlphaMap(int *alphaMap, int width, int height, double angle) {
        angle *= -1;
        double angle_sin = sin(angle * M_PI / 180);
        double angle_cos = cos(angle * M_PI / 180);
        SDL_Rect boundingBox = {0, 0, width, height};
        boundingBox = getRotatedBoundingBox(boundingBox, angle);
        int c_x = width / 2;
        int c_y = height / 2;
        int* result = new int[boundingBox.w * boundingBox.h]();
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                if (alphaMap[i * width + j]) {
                    int x = round((i - c_y) * angle_sin + (j - c_x) * angle_cos);
                    int y = round((i - c_y) * angle_cos - (j - c_x) * angle_sin);
                    result[(y + c_y + (boundingBox.h - height) / 2) * boundingBox.w + x + c_x + (boundingBox.w - width) / 2] = alphaMap[i * width + j];
                }
            }
        }
        return result;
    }

    /**
     * Calculate SDL_Rect of two rectangles intersection
     * @param rect1 first rectangle
     * @param rect2 second rectangle
     * @return SDL_Rect of two rectangles intersection
     */
    SDL_Rect getIntersectionRect(const SDL_Rect& rect1, const SDL_Rect& rect2)
    {
        int x1 = std::max(rect1.x, rect2.x);
        int y1 = std::max(rect1.y, rect2.y);
        int x2 = std::min(rect1.x + rect1.w, rect2.x + rect2.w);
        int y2 = std::min(rect1.y + rect1.h, rect2.y + rect2.h);
        int width = x2 - x1;
        int height = y2 - y1;
        if(width > 0 && height > 0) {
            SDL_Rect intersect = {x1, y1, width, height};
            return intersect;
        }
        SDL_Rect intersect = {0, 0, 0, 0};
        return intersect;
    }

    /**
     * Check collision between two textures, based on alpha map
     * @param texture1 first texture
     * @param rect1 current rectangle of the first texture
     * @param angle1 angle of the first texture
     * @param texture2 second texture
     * @param rect2 current rectangle of the second texture
     * @param angle2 angle of the second texture
     * @return true if given textures has collision
     */
    bool hasCollision(Texture texture1, SDL_Rect rect1, double angle1, Texture texture2, SDL_Rect rect2, double angle2)
    {
        SDL_Rect box1 = getRotatedBoundingBox(rect1, angle1);
        SDL_Rect box2 = getRotatedBoundingBox(rect2, angle2);
        SDL_Rect intersection = getIntersectionRect(box1, box2);
        if (intersection.w == 0)
            return false;
        int *resized_map1 = resizeBilinear(texture1.alphaMap, texture1.width, texture1.height, rect1.w, rect1.h);
        int *resized_map2 = resizeBilinear(texture2.alphaMap, texture2.width, texture2.height, rect2.w, rect2.h);
        int *map1 = getRotatedAlphaMap(resized_map1, rect1.w, rect1.h, angle1);
        int *map2 = getRotatedAlphaMap(resized_map2, rect2.w, rect2.h, angle2);
        delete[] resized_map1;
        delete[] resized_map2;
        for (int i = intersection.y; i < intersection.y + intersection.h; ++i) {
            for (int j = intersection.x; j < intersection.x + intersection.w; ++j) {
                if (map1[(i - box1.y) * box1.w + j - box1.x] && map2[(i - box2.y) * box2.w + j - box2.x]) {
                    delete[] map1;
                    delete[] map2;
                    return true;
                }
            }
        }
        delete[] map1;
        delete[] map2;
        return false;
    }

    /**
     * Check if a point is inside a Rect
     * @param x
     * @param y
     * @param rect the rectangle to check with
     * @return true if point is inside the rectangle
     */
    bool pointInRect(int x, int y, const SDL_Rect &rect) {
        SDL_Point point;
        point.x = x;
        point.y = y;
        return SDL_PointInRect(&point, &rect) == SDL_TRUE;
    }

    /**
     * Check if mouse is inside a rect
     * @param rect check when mouse is inside this rectangle
     * @return true if mouse is inside the rectangle
     */
    bool mouseInRect(const SDL_Rect &rect) {
        return pointInRect(Mouse.x, Mouse.y, rect);
    }

    /**
     * Check if Mouse is clicked on the given SDL_Rect.
     * @param button SDL_Rect to check
     */
    bool rectClicked(const SDL_Rect &rect) {
        return pointInRect(Mouse.x, Mouse.y, rect) && Mouse.clicked(SDL_BUTTON_LEFT, 1, SDL_RELEASED);
    }

    /**
     * Check if Mouse is clicked on the given SDL_Rect.
     * @param button SDL_Rect to check
     */
    bool rectPressed(const SDL_Rect &rect) {
        return pointInRect(Mouse.x, Mouse.y, rect) && Mouse.clicked();
    }
}

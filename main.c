#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define WINDOW_WIDTH 856
#define WINDOW_HEIGHT 480

#define MC_WIDTH 214 
#define MC_HEIGHT 120

void MC_init(void);
void MC_destroy(void);
void MC_update(void);
void MC_render(void);
void MC_handleKeyboard(int down, int key);
void MC_handleMouse(int down, int left, int right);
void MC_handleMousePos(int x, int y);
extern uint32_t *MC_framebuffer;

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

static bool windowIsFocused = false;
static bool windowShouldClose = false;
static void mainloop(void);
static void cleanup(void);
static void handleInput(void);

int main(int argc, char **argv) {
  window = SDL_CreateWindow("Minecraft 4k",
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, MC_WIDTH, MC_HEIGHT);

  MC_init(); 

  #ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(mainloop, 0, 1);
  #else
  while (1) mainloop();
  #endif
}

void mainloop(void) {
  if (windowShouldClose) {
    cleanup();
    #ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
    #else
    exit(0);
    #endif
  }

  MC_update();
  MC_render();
  SDL_Delay(6);

  { void *pixels; int pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);
    for (int i = 0, sp = 0, dp = 0; i < MC_HEIGHT; i++, sp += MC_WIDTH, dp += pitch)
      memcpy(pixels + dp, MC_framebuffer + sp, MC_WIDTH * sizeof(uint32_t));
    SDL_UnlockTexture(texture);
  }

  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);

  handleInput();
}

void handleInput(void) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_QUIT)
      windowShouldClose = true;

    if (e.type == SDL_KEYDOWN) {
      MC_handleKeyboard(true, e.key.keysym.sym);
      if (e.key.keysym.sym == SDLK_ESCAPE) {
        SDL_SetWindowGrab(window, SDL_FALSE);
        SDL_ShowCursor(SDL_ENABLE);
        windowIsFocused = false;
      }
    } else if (e.type == SDL_KEYUP)
      MC_handleKeyboard(false, e.key.keysym.sym);

    bool right_click = e.button.button == SDL_BUTTON_RIGHT;
    bool left_click = e.button.button == SDL_BUTTON_LEFT;
    bool mouse_down = e.type == SDL_MOUSEBUTTONDOWN;

    if (e.type == SDL_MOUSEBUTTONDOWN) {
      SDL_SetWindowGrab(window, SDL_TRUE);
      SDL_ShowCursor(SDL_DISABLE);
      windowIsFocused = true;
    }

    MC_handleMouse(mouse_down, left_click, right_click);
  }
  int x = 0, y = 0;
  if (windowIsFocused)
    SDL_GetMouseState(&x, &y);
  MC_handleMousePos(x, y);
}

void cleanup(void) {
  MC_destroy();

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

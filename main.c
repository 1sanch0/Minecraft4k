#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define WINDOW_WIDTH 856
#define WINDOW_HEIGHT 480

#define MC_WIDTH 214 
#define MC_HEIGHT 120

extern uint32_t *MC_framebuffer;

void MC_init(void);
void MC_destroy(void);
void MC_run(float dt);
void MC_handleKeyboard(int down, int key);
void MC_handleMouse(int down, int left, int right);
void MC_handleMousePos(int x, int y);

int main(int argc, char **argv) {
  (void)argc; (void)argv;

  SDL_Window *window = SDL_CreateWindow("Minecraft 4k",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);

  SDL_Texture *texture = SDL_CreateTexture(renderer,
                                           SDL_PIXELFORMAT_RGB888,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           MC_WIDTH, MC_HEIGHT);

  MC_init(); 

  uint64_t last_ticks = SDL_GetTicks64();

  SDL_Event e;
  bool windowIsFocused = false;
  bool windowShouldClose = false;
  while (!windowShouldClose) {
    uint64_t ticks = SDL_GetTicks64();
    // SDL_Delay(10);
    MC_run(ticks - last_ticks);
    last_ticks = ticks;

    { void *pixels; int pitch;
      SDL_LockTexture(texture, NULL, &pixels, &pitch);
      for (int i = 0, sp = 0, dp = 0; i < MC_HEIGHT; i++, sp += MC_WIDTH, dp += pitch)
        memcpy(pixels + dp, MC_framebuffer + sp, MC_WIDTH * sizeof(uint32_t));
      SDL_UnlockTexture(texture);
    }

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    


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

  MC_destroy();

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  
  return 0;
}

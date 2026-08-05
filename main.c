#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define WINDOW_WIDTH 856
#define WINDOW_HEIGHT 480

#define MC_WIDTH 214 
#define MC_HEIGHT 120

uint32_t MC_framebuffer[MC_WIDTH * MC_HEIGHT];
void MC_sleepMillis(uint32_t ms) { SDL_Delay(ms); }
uint32_t MC_currentTimeMillis(void) { return SDL_GetTicks(); }
void MC_init(void);
void MC_run(void);
void MC_handleEvent(int id, int key, int x, int y, int modifiers);

int main(int argc, char **argv) {
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
  
  SDL_Event e;
  bool windowShouldClose = false;
  while (!windowShouldClose) {
    MC_run();

    { void *pixels; int pitch;
      SDL_LockTexture(texture, NULL, &pixels, &pitch);
      for (int i = 0, sp = 0, dp = 0; i < MC_HEIGHT; i++, sp += MC_WIDTH, dp += pitch)
        memcpy(pixels + dp, MC_framebuffer + sp, MC_WIDTH * sizeof(uint32_t));
      SDL_UnlockTexture(texture);
    }

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    




    int id = 0, key = 0, x = 0, y = 0, modifiers = 0;
    SDL_GetMouseState(&x, &y);
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        windowShouldClose = true;
      }
      if (e.type == SDL_KEYDOWN) {
        id = 401;
        key = e.key.keysym.sym; // this shit makes it crash sometimes lmao
      }
      else if (e.type == SDL_KEYUP) {
        id = 402;
        key = e.key.keysym.sym;
      }

      if (e.type == SDL_MOUSEBUTTONDOWN)
        id = 501;
      else if (e.type == SDL_MOUSEBUTTONUP)
        id = 502;

      if (e.button.button == SDL_BUTTON_RIGHT)
        modifiers |= 4;
    }
    MC_handleEvent(id, key, x, y, modifiers);
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  
  return 0;
}

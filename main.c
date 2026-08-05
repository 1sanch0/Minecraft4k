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
void MC_handleEvent(void);

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

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) {
        windowShouldClose = true;
      }
      if (e.type == SDL_KEYDOWN) {
        puts("keydown");
      } else if (e.type == SDL_KEYUP) {
        puts("keyup");
      }
    }
    MC_handleEvent();
  }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  
  return 0;
}

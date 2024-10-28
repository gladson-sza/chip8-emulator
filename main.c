#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "SDL.h"
#include "SDL_error.h"
#include "SDL_events.h"
#include "SDL_log.h"
#include "SDL_render.h"
#include "SDL_timer.h"
#include "SDL_video.h"

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
} sdl_t;

typedef struct {
  uint32_t window_width;
  uint32_t window_height;
  uint32_t fg_color;     // Foreground color RGBA8888
  uint32_t bg_color;     // Background color RGBA8888
  uint32_t scale_factor; // Scale CHIP8 pixels
} config_t;

typedef enum { QUIT, RUNNING, PAUSED } emulator_state_t;

typedef struct {
  emulator_state_t state;
} chip8_t;

bool init_sdl(sdl_t *sdl, const config_t config) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
    SDL_Log("Could not initialize SDL subsystems! %s\n", SDL_GetError());
    return false;
  }

  sdl->window = SDL_CreateWindow("CHIP8 Emulator", SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED,
                                 config.window_width * config.scale_factor,
                                 config.window_height * config.scale_factor, 0);

  if (!sdl->window) {
    SDL_Log("Could not create SDL window %s\n", SDL_GetError());
    return false;
  }

  sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);

  if (!sdl->renderer) {
    SDL_Log("Could not create SDL renderer %s\n", SDL_GetError());
  }

  return true;
}

bool set_args_config(config_t *config, const int argc, char **argv) {
  (void)argc;
  (void)argv;

  // Set Default CHIP8 Resolution
  config->window_width = 64;
  config->window_height = 32;
  config->fg_color = 0xFFFFFFF; // White
  config->bg_color = 0x0000000; // Black
  config->scale_factor = 20;

  return true;
}

bool init_chip8(chip8_t *chip8) {
  chip8->state = RUNNING;
  return true;
}

void cleanup_sdl(const sdl_t sdl) {
  SDL_DestroyRenderer(sdl.renderer);
  SDL_DestroyWindow(sdl.window);
  SDL_Quit();
}

void clear_screen(const sdl_t sdl, const config_t config) {
  const uint8_t r = config.bg_color >> 24 & 0xFF;
  const uint8_t g = config.bg_color >> 18 & 0xFF;
  const uint8_t b = config.bg_color >> 8 & 0xFF;
  const uint8_t a = config.bg_color >> 0 & 0xFF;

  SDL_SetRenderDrawColor(sdl.renderer, r, g, b, a);
  SDL_RenderClear(sdl.renderer);
}

void update_screen(const sdl_t sdl) { SDL_RenderPresent(sdl.renderer); }

void handle_input(chip8_t *chip8) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      chip8->state = QUIT;
      return;
    case SDL_KEYDOWN:
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
        chip8->state = QUIT;
        return;
      default:
        break;
      }
      break;
    case SDL_KEYUP:
      break;
    default:
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  // Set configs
  config_t config = {0};
  if (!set_args_config(&config, argc, argv))
    exit(EXIT_FAILURE);

  // Init SDL
  sdl_t sdl = {0};
  if (!init_sdl(&sdl, config))
    exit(EXIT_FAILURE);

  // Init CHIP 8
  chip8_t chip8 = {0};
  if (!init_chip8(&chip8))
    exit(EXIT_FAILURE);

  // Initial screen clear
  clear_screen(sdl, config);

  // Emulator Loop
  while (chip8.state != QUIT) {
    handle_input(&chip8);
    SDL_Delay(16);
    update_screen(sdl);
  }

  // Cleanup
  cleanup_sdl(sdl);

  exit(EXIT_SUCCESS);
}

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;

    SDL_Texture *background;
    SDL_Texture *needle;

    SDL_Texture *indicator;
    SDL_Texture *highbeam;
    SDL_Texture *oil;
    SDL_Texture *generator;

    TTF_Font *font;

    bool indicator_on;
    bool highbeam_on;
    bool oil_on;
    bool generator_on;

    int width;
    int height;

} Display;

bool display_init(
    Display *display,
    const char *title,
    int width,
    int height,
    const char *background_path,
    const char *needle_path,
    const char *font_path
);

void display_render(
    const Display *display,
    bool display_powered,
    float speed_kmh,
    float fuel_percent
);

void display_set_indicators(
    Display *display,
    bool indicator,
    bool highbeam,
    bool oil,
    bool generator
);

void display_destroy(
    Display *display
);

#endif
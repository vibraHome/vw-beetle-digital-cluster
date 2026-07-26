#include "display.h"

#include <stdio.h>
#include <string.h>
#include <SDL2/SDL_image.h>

static void display_render_speed(
    const Display *display,
    float speed_kmh
);

static void display_render_needle(
    const Display *display,
    float speed_kmh
);

static double speed_to_angle(float speed_kmh)
{
    const float minimum_speed = 0.0f;
    const float maximum_speed = 240.0f;

    const double minimum_angle = -139.0;
    const double maximum_angle = 139.0;

    if (speed_kmh < minimum_speed) {
        speed_kmh = minimum_speed;
    }

    if (speed_kmh > maximum_speed) {
        speed_kmh = maximum_speed;
    }

    const float normalized =
        (speed_kmh - minimum_speed) /
        (maximum_speed - minimum_speed);

    return minimum_angle +
           normalized * (maximum_angle - minimum_angle);
}

bool display_init(
    Display *display,
    const char *title,
    int width,
    int height,
    const char *background_path,
    const char *needle_path,
    const char *font_path
) {
    if (display == NULL ||
        title == NULL ||
        background_path == NULL ||
        needle_path == NULL ||
        font_path == NULL) {
        fprintf(stderr, "Ungueltige Argumente fuer display_init().\n");
        return false;
    }

    memset(display, 0, sizeof(*display));

    display->width = width;
    display->height = height;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init fehlgeschlagen: %s\n", SDL_GetError());
        return false;
    }

    const int image_flags = IMG_INIT_PNG;

    if ((IMG_Init(image_flags) & image_flags) != image_flags) {
        fprintf(stderr, "IMG_Init fehlgeschlagen: %s\n", IMG_GetError());
        SDL_Quit();
        return false;
    }

    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init fehlgeschlagen: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    display->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (display->window == NULL) {
        fprintf(stderr, "Fenster konnte nicht erstellt werden: %s\n",
                SDL_GetError());
        display_destroy(display);
        return false;
    }

    display->renderer = SDL_CreateRenderer(
        display->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (display->renderer == NULL) {
        fprintf(stderr, "Renderer konnte nicht erstellt werden: %s\n",
                SDL_GetError());
        display_destroy(display);
        return false;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    display->background = IMG_LoadTexture(
        display->renderer,
        background_path
    );

    if (display->background == NULL) {
        fprintf(stderr,
                "Hintergrundbild konnte nicht geladen werden: %s\n",
                IMG_GetError());
        display_destroy(display);
        return false;
    }

    display->needle = IMG_LoadTexture(
        display->renderer,
        needle_path
    );

    if (display->needle == NULL) {
        fprintf(stderr,
                "Zeigerbild konnte nicht geladen werden: %s\n",
                IMG_GetError());
        display_destroy(display);
        return false;
    }

    SDL_SetTextureBlendMode(
        display->needle,
        SDL_BLENDMODE_BLEND
    );
    SDL_SetTextureBlendMode(
    display->indicator,
    SDL_BLENDMODE_BLEND
);

SDL_SetTextureBlendMode(
    display->highbeam,
    SDL_BLENDMODE_BLEND
);

SDL_SetTextureBlendMode(
    display->oil,
    SDL_BLENDMODE_BLEND
);

    display->font = TTF_OpenFont(font_path, 38);

    if (display->font == NULL) {
        fprintf(stderr,
                "Schrift konnte nicht geladen werden: %s\n",
                TTF_GetError());
        display_destroy(display);
        return false;
    }

    display->indicator = IMG_LoadTexture(
    display->renderer,
    "assets/indicator.png"
);

if (display->indicator == NULL) {
    fprintf(stderr,
            "Blinkersymbol konnte nicht geladen werden: %s\n",
            IMG_GetError());
    display_destroy(display);
    return false;
}

display->highbeam = IMG_LoadTexture(
    display->renderer,
    "assets/highbeam.png"
);

if (display->highbeam == NULL) {
    fprintf(stderr,
            "Fernlichtsymbol konnte nicht geladen werden: %s\n",
            IMG_GetError());
    display_destroy(display);
    return false;
}

display->oil = IMG_LoadTexture(
    display->renderer,
    "assets/oil.png"
);

if (display->oil == NULL) {
    fprintf(stderr,
            "Oeldrucksymbol konnte nicht geladen werden: %s\n",
            IMG_GetError());
    display_destroy(display);
    return false;
}

    SDL_RenderSetLogicalSize(
        display->renderer,
        width,
        height
    );

    return true;
}

void display_render(
    const Display *display,
    float speed_kmh
) {
    if (display == NULL ||
        display->renderer == NULL ||
        display->background == NULL) {
        return;
    }

    SDL_SetRenderDrawColor(
        display->renderer,
        0,
        0,
        0,
        255
    );

    SDL_RenderClear(display->renderer);

    SDL_Rect background_destination = {
        .x = 0,
        .y = 0,
        .w = display->width,
        .h = display->height
    };

    SDL_RenderCopy(
        display->renderer,
        display->background,
        NULL,
        &background_destination
    );

    display_render_needle(display, speed_kmh);
    display_render_speed(display, speed_kmh);
    display_render_indicators(display);

    SDL_RenderPresent(display->renderer);
}

static void display_render_needle(
    const Display *display,
    float speed_kmh
) {
    if (display->needle == NULL) {
        return;
    }

    const int needle_width = 110;
    const int needle_height = 320;

    const int center_x = display->width / 2;
    const int center_y = display->height / 2;

    SDL_Rect destination = {
        .x = center_x - needle_width / 2,
        .y = center_y - needle_height + 20,
        .w = needle_width,
        .h = needle_height
    };

    SDL_Point pivot = {
        .x = needle_width / 2,
        .y = needle_height - 20
    };

    const double angle = speed_to_angle(speed_kmh);

    SDL_RenderCopyEx(
        display->renderer,
        display->needle,
        NULL,
        &destination,
        angle,
        &pivot,
        SDL_FLIP_NONE
    );
}

static void display_render_speed(
    const Display *display,
    float speed_kmh
) {
    char text[32];

    snprintf(
        text,
        sizeof(text),
        "%03.0f",
        speed_kmh
    );

    SDL_Color text_color = {
        .r = 235,
        .g = 235,
        .b = 235,
        .a = 255
    };

    SDL_Surface *surface = TTF_RenderUTF8_Blended(
        display->font,
        text,
        text_color
    );

    if (surface == NULL) {
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(
        display->renderer,
        surface
    );

    if (texture == NULL) {
        SDL_FreeSurface(surface);
        return;
    }

    const int display_box_center_x = 400;
    const int display_box_center_y = 538;

    SDL_Rect destination = {
        .x = display_box_center_x - surface->w / 2,
        .y = display_box_center_y - surface->h / 2,
        .w = surface->w,
        .h = surface->h
    };

    SDL_FreeSurface(surface);

    SDL_RenderCopy(
        display->renderer,
        texture,
        NULL,
        &destination
    );

    SDL_DestroyTexture(texture);
}

static void display_render_indicators(Display *display)
{
    SDL_Rect indicator_rect = {
        70,
        230,
        42,
        42
    };

    SDL_Rect highbeam_rect = {
        139,
        230,
        42,
        42
    };

    SDL_Rect oil_rect = {
        208,
        230,
        42,
        42
    };

    if (display->indicator != NULL)
    {
        SDL_RenderCopy(
            display->renderer,
            display->indicator,
            NULL,
            &indicator_rect
        );
    }

    if (display->highbeam != NULL)
    {
        SDL_RenderCopy(
            display->renderer,
            display->highbeam,
            NULL,
            &highbeam_rect
        );
    }

    if (display->oil != NULL)
    {
        SDL_RenderCopy(
            display->renderer,
            display->oil,
            NULL,
            &oil_rect
        );
    }
}


void display_destroy(Display *display)
{
    if (display == NULL) {
        return;
    }

    if (display->font != NULL) {
        TTF_CloseFont(display->font);
        display->font = NULL;
    }

    if (display->oil != NULL) {
    SDL_DestroyTexture(display->oil);
    display->oil = NULL;
    }

    if (display->highbeam != NULL) {
    SDL_DestroyTexture(display->highbeam);
    display->highbeam = NULL;
    }

    if (display->indicator != NULL) {
    SDL_DestroyTexture(display->indicator);
    display->indicator = NULL;
    }

    if (display->needle != NULL) {
        SDL_DestroyTexture(display->needle);
        display->needle = NULL;
    }

    if (display->background != NULL) {
        SDL_DestroyTexture(display->background);
        display->background = NULL;
    }

    if (display->renderer != NULL) {
        SDL_DestroyRenderer(display->renderer);
        display->renderer = NULL;
    }

    if (display->window != NULL) {
        SDL_DestroyWindow(display->window);
        display->window = NULL;
    }

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
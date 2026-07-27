#include "display.h"

#include <stdio.h>
#include <string.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

static void display_render_speed(
    const Display *display,
    float speed_kmh
);

static void display_render_needle(
    const Display *display,
    float speed_kmh
);

static void display_render_indicators(
    const Display *display
);

static void display_render_fuel(
    const Display *display,
    float fuel_percent
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
        fprintf(
            stderr,
            "Fenster konnte nicht erstellt werden: %s\n",
            SDL_GetError()
        );

        display_destroy(display);
        return false;
    }

    display->renderer = SDL_CreateRenderer(
        display->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (display->renderer == NULL) {
        fprintf(
            stderr,
            "Renderer konnte nicht erstellt werden: %s\n",
            SDL_GetError()
        );

        display_destroy(display);
        return false;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    SDL_RenderSetLogicalSize(
        display->renderer,
        width,
        height
    );

    display->background = IMG_LoadTexture(
        display->renderer,
        background_path
    );

    if (display->background == NULL) {
        fprintf(
            stderr,
            "Hintergrundbild konnte nicht geladen werden: %s\n",
            IMG_GetError()
        );

        display_destroy(display);
        return false;
    }

    display->needle = IMG_LoadTexture(
        display->renderer,
        needle_path
    );

    if (display->needle == NULL) {
        fprintf(
            stderr,
            "Zeigerbild konnte nicht geladen werden: %s\n",
            IMG_GetError()
        );

        display_destroy(display);
        return false;
    }

    SDL_SetTextureBlendMode(
        display->needle,
        SDL_BLENDMODE_BLEND
    );

    display->font = TTF_OpenFont(
        font_path,
        38
    );

    if (display->font == NULL) {
        fprintf(
            stderr,
            "Schrift konnte nicht geladen werden: %s\n",
            TTF_GetError()
        );

        display_destroy(display);
        return false;
    }

    display->indicator = IMG_LoadTexture(
        display->renderer,
        "assets/indicator.png"
    );

    if (display->indicator == NULL) {
        fprintf(
            stderr,
            "Blinkersymbol konnte nicht geladen werden: %s\n",
            IMG_GetError()
        );

        display_destroy(display);
        return false;
    }

    display->highbeam = IMG_LoadTexture(
        display->renderer,
        "assets/highbeam.png"
    );

    if (display->highbeam == NULL) {
        fprintf(
            stderr,
            "Fernlichtsymbol konnte nicht geladen werden: %s\n",
            IMG_GetError()
        );

        display_destroy(display);
        return false;
    }

    display->oil = IMG_LoadTexture(
        display->renderer,
        "assets/oil.png"
    );

    if (display->oil == NULL) {
        fprintf(
            stderr,
            "Oeldrucksymbol konnte nicht geladen werden: %s\n",
            IMG_GetError()
        );

        display_destroy(display);
        return false;
    }

    display->generator = IMG_LoadTexture(
    display->renderer,
    "assets/generator.png"
);

if (display->generator == NULL) {
    fprintf(
        stderr,
        "Generator-Ladekontrollsymbol konnte nicht geladen werden: %s\n",
        IMG_GetError()
    );

    display_destroy(display);
    return false;
}

display->coolant_warning = IMG_LoadTexture(
    display->renderer,
    "assets/coolant_warning.png"
);

if (display->coolant_warning == NULL) {
    fprintf(
        stderr,
        "Kuehlmittelwarnsymbol konnte nicht geladen werden: %s\n",
        IMG_GetError()
    );

    display_destroy(display);
    return false;
}
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

    SDL_SetTextureBlendMode(
        display->generator,
        SDL_BLENDMODE_BLEND
    );

    SDL_SetTextureBlendMode(
    display->coolant_warning,
    SDL_BLENDMODE_BLEND
    );

    return true;
}

void display_render(
    const Display *display,
    bool display_powered,
    float speed_kmh,
    float fuel_percent
) {
    if (display == NULL ||
        display->renderer == NULL ||
        display->background == NULL) {
        return;
    }

    if (!display_powered) {
        SDL_SetRenderDrawColor(
            display->renderer,
            0,
            0,
            0,
            255
        );

        SDL_RenderClear(display->renderer);
        SDL_RenderPresent(display->renderer);
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

    display_render_needle(
        display,
        speed_kmh
    );

    display_render_speed(
        display,
        speed_kmh
    );

    display_render_fuel(
        display,
        fuel_percent
    );

    display_render_indicators(display);

    SDL_RenderPresent(display->renderer);
}

static void display_render_needle(
    const Display *display,
    float speed_kmh
) {
    if (display == NULL ||
        display->renderer == NULL ||
        display->needle == NULL) {
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
    if (display == NULL ||
        display->renderer == NULL ||
        display->font == NULL) {
        return;
    }

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
        fprintf(
            stderr,
            "Geschwindigkeitstext konnte nicht erzeugt werden: %s\n",
            TTF_GetError()
        );

        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(
        display->renderer,
        surface
    );

    if (texture == NULL) {
        fprintf(
            stderr,
            "Textur fuer Geschwindigkeit konnte nicht erzeugt werden: %s\n",
            SDL_GetError()
        );

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

static void display_render_fuel(
    const Display *display,
    float fuel_percent
) {
    if (display == NULL ||
        display->renderer == NULL ||
        display->font == NULL) {
        return;
    }

    /* Tankstand auf 0 bis 100 Prozent begrenzen */
    if (fuel_percent < 0.0f) {
        fuel_percent = 0.0f;
    }

    if (fuel_percent > 100.0f) {
        fuel_percent = 100.0f;
    }

    bool reserve_blink =
    ((SDL_GetTicks() / 500) % 2) == 0;

    /*
     * Position der Tankanzeige.
     */
    const int gauge_start_x = 300;
    const int gauge_end_x = 500;
    const int gauge_y = 280;

    const int gauge_width = gauge_end_x - gauge_start_x;

    /* Position des Tankzeigers berechnen */
    const int pointer_x =
        gauge_start_x +
        (int)((fuel_percent / 100.0f) * gauge_width);

    SDL_SetRenderDrawColor(
        display->renderer,
        235,
        235,
        235,
        255
    );

    /* Hauptlinie */
    SDL_RenderDrawLine(
        display->renderer,
        gauge_start_x,
        gauge_y,
        gauge_end_x,
        gauge_y
    );

    /* Teilstriche */
    for (int index = 0; index <= 4; index++) {
        const int tick_x =
            gauge_start_x +
            (index * gauge_width / 4);

        const int tick_height =
            (index == 0 || index == 4) ? 14 : 8;

        SDL_RenderDrawLine(
            display->renderer,
            tick_x,
            gauge_y - tick_height / 2,
            tick_x,
            gauge_y + tick_height / 2
        );
    }

    /*
     * Dreieckiger Zeiger:
     *
     *      /\
     *     /__\
     */
    /* Ausgefüllter dreieckiger Tankzeiger */
    const int pointer_top_y = gauge_y - 30;
    const int pointer_tip_y = gauge_y - 10;
    const int pointer_half_width = 10;
    const int pointer_height = pointer_tip_y - pointer_top_y;

    bool draw_pointer =
    (fuel_percent >= 10.0f) || reserve_blink;

if (draw_pointer)
{
    // Farbe setzen

    if (fuel_percent < 15.0f)
        SDL_SetRenderDrawColor(display->renderer, 220, 30, 30, 255);
    else
        SDL_SetRenderDrawColor(display->renderer, 235, 235, 235, 255);

    for (int y = pointer_top_y; y <= pointer_tip_y; y++) {
    const int distance_from_tip = pointer_tip_y - y;

    const int half_width =
        (distance_from_tip * pointer_half_width) /
        pointer_height;

    SDL_RenderDrawLine(
        display->renderer,
        pointer_x - half_width,
        y,
        pointer_x + half_width,
        y
    );
    }
}

    SDL_Color normal_text_color = {
    .r = 235,
    .g = 235,
    .b = 235,
    .a = 255
};


SDL_Color empty_text_color;   // <-- genau EINMAL deklarieren

if (fuel_percent < 15.0f) {
    empty_text_color.r = 220;
    empty_text_color.g = 30;
    empty_text_color.b = 30;
    empty_text_color.a = 255;
} else {
    empty_text_color = normal_text_color;
}

    SDL_Surface *empty_surface = TTF_RenderUTF8_Blended(
    display->font,
    "E",
    empty_text_color
);

    SDL_Surface *full_surface = TTF_RenderUTF8_Blended(
    display->font,
    "F",
    normal_text_color
);

    if (empty_surface == NULL || full_surface == NULL) {
        fprintf(
            stderr,
            "Tankanzeigentext konnte nicht erzeugt werden: %s\n",
            TTF_GetError()
        );

        if (empty_surface != NULL) {
            SDL_FreeSurface(empty_surface);
        }

        if (full_surface != NULL) {
            SDL_FreeSurface(full_surface);
        }

        return;
    }

    SDL_Texture *empty_texture =
        SDL_CreateTextureFromSurface(
            display->renderer,
            empty_surface
        );

    SDL_Texture *full_texture =
        SDL_CreateTextureFromSurface(
            display->renderer,
            full_surface
        );

    if (empty_texture == NULL || full_texture == NULL) {
        fprintf(
            stderr,
            "Tankanzeigentextur konnte nicht erzeugt werden: %s\n",
            SDL_GetError()
        );

        if (empty_texture != NULL) {
            SDL_DestroyTexture(empty_texture);
        }

        if (full_texture != NULL) {
            SDL_DestroyTexture(full_texture);
        }

        SDL_FreeSurface(empty_surface);
        SDL_FreeSurface(full_surface);
        return;
    }

    SDL_Rect empty_destination = {
        .x = gauge_start_x - empty_surface->w - 15,
        .y = gauge_y - empty_surface->h / 2,
        .w = empty_surface->w,
        .h = empty_surface->h
    };

    SDL_Rect full_destination = {
        .x = gauge_end_x + 15,
        .y = gauge_y - full_surface->h / 2,
        .w = full_surface->w,
        .h = full_surface->h
    };

    if (fuel_percent >= 10.0f || reserve_blink)
    {
    SDL_RenderCopy(
        display->renderer,
        empty_texture,
        NULL,
        &empty_destination
    );
    }

    SDL_RenderCopy(
        display->renderer,
        full_texture,
        NULL,
        &full_destination
    );

    SDL_DestroyTexture(empty_texture);
    SDL_DestroyTexture(full_texture);

    SDL_FreeSurface(empty_surface);
    SDL_FreeSurface(full_surface);
}

static void display_render_indicators(
    const Display *display
) {
    if (display == NULL ||
        display->renderer == NULL) {
        return;
    }

    SDL_Rect indicator_rect = {
    .x = 265,
    .y = 635,
    .w = 80,
    .h = 80
};

SDL_Rect highbeam_rect = {
    .x = 338,
    .y = 643,
    .w = 120,
    .h = 120
};

SDL_Rect oil_rect = {
    .x = 462,
    .y = 635,
    .w = 80,
    .h = 80
};

SDL_Rect generator_rect = {
    .x = 530,
    .y = 375,
    .w = 60,
    .h = 60
};

SDL_Rect coolant_warning_rect = {
    .x = 200,
    .y = 370,
    .w = 75,
    .h = 75
};

    if (display->indicator != NULL &&
    display->indicator_on) {
        SDL_RenderCopy(
            display->renderer,
            display->indicator,
            NULL,
            &indicator_rect
        );
    }

    if (display->highbeam != NULL &&
    display->highbeam_on) {
        SDL_RenderCopy(
            display->renderer,
            display->highbeam,
            NULL,
            &highbeam_rect
        );
    }

    if (display->oil != NULL &&
    display->oil_on) {
        SDL_RenderCopy(
            display->renderer,
            display->oil,
            NULL,
            &oil_rect
        );
    }

    if (display->generator != NULL &&
    display->generator_on) {
        SDL_RenderCopy(
            display->renderer,
            display->generator,
            NULL,
            &generator_rect
        );
    }

    if (display->coolant_warning != NULL &&
    display->coolant_warning_on) 
{
    SDL_RenderCopy(
        display->renderer,
        display->coolant_warning,
        NULL,
        &coolant_warning_rect
    );
}
}

void display_set_indicators(
    Display *display,
    bool indicator,
    bool highbeam,
    bool oil,
    bool generator,
    bool coolant_warning
)
{
    if (display == NULL) {
        return;
    }

    display->indicator_on = indicator;
    display->highbeam_on = highbeam;
    display->oil_on = oil;
    display->generator_on = generator;
    display->coolant_warning_on = coolant_warning;
}

void display_destroy(Display *display)
{
    if (display == NULL) {
        return;
    }

     if (display->coolant_warning != NULL) {
        SDL_DestroyTexture(display->coolant_warning);
        display->coolant_warning = NULL;
    }
    
    if (display->generator != NULL) {
        SDL_DestroyTexture(display->generator);
        display->generator = NULL;
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

    if (display->font != NULL) {
        TTF_CloseFont(display->font);
        display->font = NULL;
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
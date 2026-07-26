#include <stdbool.h>
#include <SDL2/SDL.h>

#include "display.h"
#include "simulator.h"
#include "cluster_state.h"

enum {
    SCREEN_WIDTH = 800,
    SCREEN_HEIGHT = 800
};

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    Display display;
    Simulator simulator;
    ClusterController cluster;

    ClusterInput cluster_input = {0};
    ClusterOutput cluster_output = {0};

    if (!display_init(
            &display,
            "VW Beetle Digital Cluster",
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            "assets/background.png",
            "assets/needle.png",
            "assets/fonts/arialbd.ttf")) {
        return 1;
    }

    simulator_init(&simulator);
    cluster_init(&cluster);

    bool running = true;
    bool ignition_on = true;

    bool indicator = false;
    bool highbeam = false;
    bool oil = false;
    bool generator = false;

   Uint64 previous_counter = SDL_GetPerformanceCounter();

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            if (event.type == SDL_KEYDOWN &&
                event.key.repeat == 0) {

                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = false;
                        break;

                    case SDLK_i:
                        ignition_on = !ignition_on;
                        break;

                    case SDLK_1:
                        indicator = !indicator;
                        break;

                    case SDLK_2:
                        highbeam = !highbeam;
                        break;

                    case SDLK_3:
                        oil = !oil;
                        break;

                    case SDLK_4:
                        generator = !generator;
                        break;

                    case SDLK_5:
                        simulator.fuel_percent -= 5.0f;

                    if  (simulator.fuel_percent < 0.0f) {
                        simulator.fuel_percent = 0.0f;
                    }
                        break;

                    case SDLK_6:
                        fuel_percent += 5.0f;

                        if  (simulator.fuel_percent > 100.0f) {
                            simulator.fuel_percent = 100.0f;
                    }
                        break;

                    default:
                        break;
                }
            }

            simulator_handle_event(&simulator, &event);
        }

        const Uint64 current_counter =
            SDL_GetPerformanceCounter();

        float delta_seconds =
            (float)(current_counter - previous_counter) /
            (float)SDL_GetPerformanceFrequency();

        previous_counter = current_counter;

        if (delta_seconds > 0.1f) {
            delta_seconds = 0.1f;
        }

        simulator_update(
            &simulator,
            delta_seconds
        );

        /* Eingabedaten für den Cluster */

        cluster_input.ignition_on = ignition_on;
        cluster_input.vehicle_speed = simulator.speed_kmh;
        cluster_input.fuel_percent =
        simulator.fuel_percent;

        cluster_input.indicator = indicator;
        cluster_input.highbeam = highbeam;
        cluster_input.oil = oil;
        cluster_input.generator = generator;

        cluster_update(
            &cluster,
            &cluster_input,
            delta_seconds
        );

        cluster_build_output(
            &cluster,
            &cluster_input,
            &cluster_output
        );

        display_set_indicators(
            &display,
            cluster_output.indicator,
            cluster_output.highbeam,
            cluster_output.oil,
            cluster_output.generator
        );

        display_render(
            &display,
            cluster_output.display_powered,
            cluster_output.displayed_speed,
            cluster_output.fuel_percent
        );

        SDL_Delay(1);
    }

    display_destroy(&display);

    return 0;
}
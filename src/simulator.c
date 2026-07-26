#include "simulator.h"

void simulator_init(Simulator *simulator)
{
    if (simulator == NULL) {
        return;
    }

    simulator->speed_kmh = 0.0f;
    simulator->target_speed_kmh = 0.0f;
    simulator->fuel_percent = 75.0f;

    simulator->acceleration_kmh_per_second = 35.0f;
    simulator->braking_kmh_per_second = 50.0f;
    simulator->maximum_speed_kmh = 240.0f;

    simulator->accelerating = false;
    simulator->braking = false;
}

void simulator_handle_event(
    Simulator *simulator,
    const SDL_Event *event
) {
    if (simulator == NULL || event == NULL) {
        return;
    }

    if (event->type == SDL_KEYDOWN && event->key.repeat == 0) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
                simulator->accelerating = true;
                break;

            case SDLK_DOWN:
                simulator->braking = true;
                break;

            case SDLK_r:
                simulator->speed_kmh = 0.0f;
                simulator->target_speed_kmh = 0.0f;
                break;

            case SDLK_5:
            simulator->fuel_percent -= 5.0f;

            if (simulator->fuel_percent < 0.0f) {
                simulator->fuel_percent = 0.0f;
            }
            break;

            case SDLK_6:
            simulator->fuel_percent += 5.0f;

            if (simulator->fuel_percent > 100.0f) {
                simulator->fuel_percent = 100.0f;
            }
            break;

            default:
                break;
        }
    }

    if (event->type == SDL_KEYUP) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
                simulator->accelerating = false;
                break;

            case SDLK_DOWN:
                simulator->braking = false;
                break;

            default:
                break;
        }
    }
}

void simulator_update(
    Simulator *simulator,
    float delta_seconds
) {
    if (simulator == NULL || delta_seconds <= 0.0f) {
        return;
    }

    /*
     * Mit den Pfeiltasten wird zunächst die Zielgeschwindigkeit
     * verändert.
     */
    if (simulator->accelerating) {
        simulator->target_speed_kmh +=
            simulator->acceleration_kmh_per_second *
            delta_seconds;
    }

    if (simulator->braking) {
        simulator->target_speed_kmh -=
            simulator->braking_kmh_per_second *
            delta_seconds;
    }

    /*
     * Zielgeschwindigkeit auf den gültigen Bereich begrenzen.
     */
    if (simulator->target_speed_kmh < 0.0f) {
        simulator->target_speed_kmh = 0.0f;
    }

    if (simulator->target_speed_kmh >
        simulator->maximum_speed_kmh) {
        simulator->target_speed_kmh =
            simulator->maximum_speed_kmh;
    }

    /*
     * Die angezeigte Geschwindigkeit bewegt sich weich
     * zur Zielgeschwindigkeit.
     */
    const float smoothing_factor = 5.0f;

    simulator->speed_kmh +=
        (simulator->target_speed_kmh -
         simulator->speed_kmh) *
        smoothing_factor *
        delta_seconds;

    /*
     * Sehr kleine Abweichungen beseitigen.
     */
    float difference =
        simulator->target_speed_kmh -
        simulator->speed_kmh;

    if (difference > -0.01f && difference < 0.01f) {
        simulator->speed_kmh =
            simulator->target_speed_kmh;
    }
}
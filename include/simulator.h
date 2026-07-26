#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdbool.h>
#include <SDL2/SDL.h>

typedef struct {
    float speed_kmh;
    float fuel_percent;
    float target_speed_kmh;
    float acceleration_kmh_per_second;
    float braking_kmh_per_second;
    float maximum_speed_kmh;
    bool accelerating;
    bool braking;
    bool indicator_on;
    bool highbeam_on;
    bool oil_on;
} Simulator;

void simulator_init(Simulator *simulator);
void simulator_handle_event(Simulator *simulator, const SDL_Event *event);
void simulator_update(Simulator *simulator, float delta_seconds);

#endif
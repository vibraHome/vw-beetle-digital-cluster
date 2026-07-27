#ifndef CLUSTER_STATE_H
#define CLUSTER_STATE_H

#include <stdbool.h>
#include <SDL2/SDL.h>

typedef enum
{
    CLUSTER_STATE_POWER_OFF = 0,
    CLUSTER_STATE_LAMP_TEST,
    CLUSTER_STATE_NEEDLE_SWEEP,
    CLUSTER_STATE_RUNNING,
    CLUSTER_STATE_SHUTDOWN
} ClusterState;

typedef struct
{
    bool ignition_on;

    float vehicle_speed;
    float fuel_percent;

    bool indicator;
    bool highbeam;
    bool oil;
    bool generator;
    bool coolant_warning;
} ClusterInput;

typedef struct
{
    ClusterState state;
    Uint32 state_start_time;

    float indicator_timer;
    bool indicator_visible;
} ClusterController;

typedef struct
{
    float displayed_speed;
    float fuel_percent;

    bool display_powered;
    bool lamp_test;

    bool indicator;
    bool highbeam;
    bool oil;
    bool generator;
    bool coolant_warning;
} ClusterOutput;

void cluster_init(
    ClusterController *controller
);

void cluster_update(
    ClusterController *controller,
    const ClusterInput *input,
    float delta_seconds
);

void cluster_build_output(
    const ClusterController *controller,
    const ClusterInput *input,
    ClusterOutput *output
);

#endif
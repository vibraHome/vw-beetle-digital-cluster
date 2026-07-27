#include "cluster_state.h"

#define LAMP_TEST_DURATION_MS 2000U
#define NEEDLE_SWEEP_DURATION_MS 2400U
#define NEEDLE_SWEEP_MAX_SPEED 240.0f

#define INDICATOR_INTERVAL_SECONDS 0.5f

static void cluster_change_state(
    ClusterController *controller,
    ClusterState new_state
) {
    controller->state = new_state;
    controller->state_start_time = SDL_GetTicks();
}

static float cluster_calculate_sweep_speed(
    const ClusterController *controller
) {
    const Uint32 elapsed_time =
        SDL_GetTicks() - controller->state_start_time;

    float progress =
        (float)elapsed_time /
        (float)NEEDLE_SWEEP_DURATION_MS;

    if (progress < 0.0f) {
        progress = 0.0f;
    }

    if (progress > 1.0f) {
        progress = 1.0f;
    }

    if (progress < 0.5f) {
        const float forward_progress =
            progress * 2.0f;

        return forward_progress *
               NEEDLE_SWEEP_MAX_SPEED;
    }

    const float backward_progress =
        (progress - 0.5f) * 2.0f;

    return
        (1.0f - backward_progress) *
        NEEDLE_SWEEP_MAX_SPEED;
}

static void cluster_update_indicator(
    ClusterController *controller,
    bool indicator_requested,
    float delta_seconds
) {
    if (!indicator_requested) {
        controller->indicator_timer = 0.0f;
        controller->indicator_visible = false;
        return;
    }

    controller->indicator_timer += delta_seconds;

    if (controller->indicator_timer >=
        INDICATOR_INTERVAL_SECONDS) {

        controller->indicator_timer -=
            INDICATOR_INTERVAL_SECONDS;

        controller->indicator_visible =
            !controller->indicator_visible;
    }
}

void cluster_init(ClusterController *controller)
{
    if (controller == NULL) {
        return;
    }

    controller->state = CLUSTER_STATE_POWER_OFF;
    controller->state_start_time = SDL_GetTicks();

    controller->indicator_timer = 0.0f;
    controller->indicator_visible = false;
}

void cluster_update(
    ClusterController *controller,
    const ClusterInput *input,
    float delta_seconds
) {
    if (controller == NULL || input == NULL) {
        return;
    }

    const Uint32 current_time = SDL_GetTicks();

    const Uint32 elapsed_time =
        current_time - controller->state_start_time;

    switch (controller->state) {
        case CLUSTER_STATE_POWER_OFF:
            if (input->ignition_on) {
                cluster_change_state(
                    controller,
                    CLUSTER_STATE_LAMP_TEST
                );
            }
            break;

        case CLUSTER_STATE_LAMP_TEST:
            if (!input->ignition_on) {
                cluster_change_state(
                    controller,
                    CLUSTER_STATE_POWER_OFF
                );
            } else if (elapsed_time >=
                       LAMP_TEST_DURATION_MS) {

                cluster_change_state(
                    controller,
                    CLUSTER_STATE_NEEDLE_SWEEP
                );
            }
            break;

        case CLUSTER_STATE_NEEDLE_SWEEP:
            if (!input->ignition_on) {
                cluster_change_state(
                    controller,
                    CLUSTER_STATE_POWER_OFF
                );
            } else if (elapsed_time >=
                       NEEDLE_SWEEP_DURATION_MS) {

                cluster_change_state(
                    controller,
                    CLUSTER_STATE_RUNNING
                );
            }
            break;

        case CLUSTER_STATE_RUNNING:
            if (!input->ignition_on) {
                cluster_change_state(
                    controller,
                    CLUSTER_STATE_SHUTDOWN
                );
            }
            break;

        case CLUSTER_STATE_SHUTDOWN:
            cluster_change_state(
                controller,
                CLUSTER_STATE_POWER_OFF
            );
            break;

        default:
            cluster_change_state(
                controller,
                CLUSTER_STATE_POWER_OFF
            );
            break;
    }

    if (controller->state ==
        CLUSTER_STATE_RUNNING) {

        cluster_update_indicator(
            controller,
            input->indicator,
            delta_seconds
        );
    } else {
        controller->indicator_timer = 0.0f;
        controller->indicator_visible = false;
    }
}

void cluster_build_output(
    const ClusterController *controller,
    const ClusterInput *input,
    ClusterOutput *output
) {
    if (output == NULL) {
        return;
    }

    output->displayed_speed = 0.0f;
    output->fuel_percent = 0.0f;

    output->display_powered = false;
    output->lamp_test = false;

    output->indicator = false;
    output->highbeam = false;
    output->oil = false;
    output->generator = false;
    output->coolant_warning = false;

    if (controller == NULL || input == NULL) {
        return;
    }
    output->fuel_percent = input->fuel_percent;
    
    switch (controller->state) {
        case CLUSTER_STATE_POWER_OFF:
            break;

        case CLUSTER_STATE_LAMP_TEST:
            output->display_powered = true;
            output->lamp_test = true;

            output->indicator = true;
            output->highbeam = true;
            output->oil = true;
            output->generator = true;
            output->coolant_warning = true;
            break;

        case CLUSTER_STATE_NEEDLE_SWEEP:
            output->display_powered = true;

            output->displayed_speed =
                cluster_calculate_sweep_speed(
                    controller
                );

            output->highbeam = input->highbeam;
            output->oil = input->oil;
            output->generator = input->generator;
            output->coolant_warning =
                input->coolant_warning;
            break;

        case CLUSTER_STATE_RUNNING:
            output->display_powered = true;

            output->displayed_speed =
                input->vehicle_speed;

            output->indicator =
                controller->indicator_visible;

            output->highbeam = input->highbeam;
            output->oil = input->oil;
            output->generator = input->generator;
                        output->coolant_warning =
                input->coolant_warning;
            break;

        case CLUSTER_STATE_SHUTDOWN:
            output->display_powered = true;
            output->displayed_speed = 0.0f;
            break;

        default:
            break;
    }
}
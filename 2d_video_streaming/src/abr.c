#include "abr.h"

int abr_mpc_choose_representation(const MpdInfo *mpd_info,
                                  const double *throughput_history_bps,
                                  size_t throughput_sample_count,
                                  double buffer_seconds,
                                  size_t next_segment_index,
                                  size_t previous_representation_index,
                                  const MpcAbrConfig *config,
                                  size_t *representation_index)
{
    (void)mpd_info;
    (void)throughput_history_bps;
    (void)throughput_sample_count;
    (void)buffer_seconds;
    (void)next_segment_index;
    (void)previous_representation_index;
    (void)config;
    (void)representation_index;

    /* TODO: evaluate MPC trajectories and choose a representation. */
    return RET_FAIL;
}

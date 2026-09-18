#ifndef ABR_H
#define ABR_H

#include <stddef.h>

#include "define.h"
#include "request_handler.h"

typedef struct {
    size_t lookahead_segments;
    double target_buffer_seconds;
    double rebuffer_penalty;
    double smoothness_penalty;
} MpcAbrConfig;

/*
 * TODO: implement MPC quality selection.
 *
 * throughput_history_bps is ordered from oldest to newest and contains only
 * successful media-segment downloads.  previous_representation_index is
 * SIZE_MAX before the first segment.  Return an index into
 * mpd_info->representations through representation_index.
 */
int abr_mpc_choose_representation(const MpdInfo *mpd_info,
                                  const double *throughput_history_bps,
                                  size_t throughput_sample_count,
                                  double buffer_seconds,
                                  size_t next_segment_index,
                                  size_t previous_representation_index,
                                  const MpcAbrConfig *config,
                                  size_t *representation_index);

#endif

#ifndef BW_PRED_H
#define BW_PRED_H

#include <stddef.h>

#include "define.h"

/*
 * TODO: predict future throughput from previous download samples.
 * throughput_samples_bps is ordered from oldest to newest.  Use at most
 * prediction_window most-recent samples and return RET_FAIL when none exist.
 */
int bw_predict(const double *throughput_samples_bps,
               size_t sample_count,
               size_t prediction_window,
               double *predicted_throughput_bps);

#endif

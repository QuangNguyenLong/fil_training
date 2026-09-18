#define _POSIX_C_SOURCE 200809L

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "abr.h"
#include "request_handler.h"

#define MAX_THROUGHPUT_SAMPLES 64

typedef struct {
    const char *mpd_url;
    size_t segment_count;
    size_t prediction_window;
    int list_only;
    int insecure_tls;
    MpcAbrConfig mpc;
} ClientConfig;

static void print_usage(const char *program)
{
    printf("Usage: %s --mpd-url URL [options]\n", program);
    printf("  -m, --mpd-url URL             MPD URL (required)\n");
    printf("  -n, --segments COUNT          Media segments to request (default: 5)\n");
    printf("  -w, --prediction-window N     Recent throughput samples for MPC (default: 5)\n");
    printf("      --lookahead N              MPC planning horizon (default: 5)\n");
    printf("      --target-buffer SECONDS    MPC target buffer (default: 10)\n");
    printf("      --rebuffer-penalty VALUE   MPC rebuffer penalty (default: 4.3)\n");
    printf("      --smoothness-penalty VALUE MPC quality-switch penalty (default: 1.0)\n");
    printf("      --list-representations     Parse and print the MPD, then exit\n");
    printf("      --insecure-tls             Accept a self-signed certificate (local testing only)\n");
}

static int parse_positive_size(const char *text, size_t maximum, size_t *result)
{
    char *end;
    unsigned long long value = strtoull(text, &end, 10);

    if (text[0] == '\0' || *end != '\0' || value == 0 || value > SIZE_MAX || value > maximum) {
        return RET_FAIL;
    }
    *result = (size_t)value;
    return RET_SUCCESS;
}

static int parse_nonnegative_double(const char *text, double *result)
{
    char *end;
    double value = strtod(text, &end);

    if (text[0] == '\0' || *end != '\0' || value < 0.0) {
        return RET_FAIL;
    }
    *result = value;
    return RET_SUCCESS;
}

static double elapsed_seconds(const struct timespec *start, const struct timespec *finish)
{
    return (double)(finish->tv_sec - start->tv_sec) +
           (double)(finish->tv_nsec - start->tv_nsec) / 1000000000.0;
}

static void print_representations(const MpdInfo *mpd_info)
{
    size_t i;

    printf("MPD contains %zu video representations:\n", mpd_info->representation_count);
    for (i = 0; i < mpd_info->representation_count; i++) {
        const Representation *representation = &mpd_info->representations[i];
        printf("  [%zu] id=%s bitrate=%ld bps", i, representation->id, representation->bitrate_bps);
        if (representation->width > 0 && representation->height > 0) {
            printf(" %dx%d", representation->width, representation->height);
        }
        printf(" segment=%.3f s\n", representation->segment_duration_seconds);
    }
}

int main(int argc, char **argv)
{
    static const struct option options[] = {
        {"mpd-url", required_argument, NULL, 'm'},
        {"segments", required_argument, NULL, 'n'},
        {"prediction-window", required_argument, NULL, 'w'},
        {"lookahead", required_argument, NULL, 1},
        {"target-buffer", required_argument, NULL, 2},
        {"rebuffer-penalty", required_argument, NULL, 3},
        {"smoothness-penalty", required_argument, NULL, 4},
        {"list-representations", no_argument, NULL, 5},
        {"insecure-tls", no_argument, NULL, 6},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };
    ClientConfig config = {
        NULL, 5, 5, 0, 0,
        {5, 10.0, 4.3, 1.0}
    };
    MpdInfo mpd_info;
    double throughput_history[MAX_THROUGHPUT_SAMPLES];
    size_t throughput_count = 0;
    size_t previous_representation = SIZE_MAX;
    double buffer_seconds = 0.0;
    int option;
    size_t segment_index;

    while ((option = getopt_long(argc, argv, "m:n:w:h", options, NULL)) != -1) {
        switch (option) {
        case 'm': config.mpd_url = optarg; break;
        case 'n':
            if (parse_positive_size(optarg, SIZE_MAX, &config.segment_count) == RET_FAIL) goto invalid_option;
            break;
        case 'w':
            if (parse_positive_size(optarg, MAX_THROUGHPUT_SAMPLES, &config.prediction_window) == RET_FAIL) goto invalid_option;
            break;
        case 1:
            if (parse_positive_size(optarg, SIZE_MAX, &config.mpc.lookahead_segments) == RET_FAIL) goto invalid_option;
            break;
        case 2:
            if (parse_nonnegative_double(optarg, &config.mpc.target_buffer_seconds) == RET_FAIL) goto invalid_option;
            break;
        case 3:
            if (parse_nonnegative_double(optarg, &config.mpc.rebuffer_penalty) == RET_FAIL) goto invalid_option;
            break;
        case 4:
            if (parse_nonnegative_double(optarg, &config.mpc.smoothness_penalty) == RET_FAIL) goto invalid_option;
            break;
        case 5: config.list_only = 1; break;
        case 6: config.insecure_tls = 1; break;
        case 'h': print_usage(argv[0]); return EXIT_SUCCESS;
        default: goto invalid_option;
        }
    }
    if (config.mpd_url == NULL || optind != argc) goto invalid_option;

    request_set_tls_verification(!config.insecure_tls);
    if (request_fetch_and_parse_mpd(config.mpd_url, &mpd_info) == RET_FAIL) {
        fprintf(stderr, "Could not fetch or parse MPD: %s\n", config.mpd_url);
        return EXIT_FAILURE;
    }
    print_representations(&mpd_info);
    if (config.list_only) return EXIT_SUCCESS;

    for (segment_index = 0; segment_index < config.segment_count; segment_index++) {
        const double *recent_history = throughput_history;
        size_t recent_count = throughput_count;
        size_t selected_representation;
        const Representation *representation;
        char url[MAX_URL_LENGTH];
        unsigned char *data;
        size_t data_size;
        struct timespec start;
        struct timespec finish;
        double download_seconds;
        long segment_number;

        if (recent_count > config.prediction_window) {
            recent_history += recent_count - config.prediction_window;
            recent_count = config.prediction_window;
        }
        if (abr_mpc_choose_representation(&mpd_info, recent_history, recent_count,
                                          buffer_seconds, segment_index,
                                          previous_representation, &config.mpc,
                                          &selected_representation) == RET_FAIL ||
            selected_representation >= mpd_info.representation_count) {
            fprintf(stderr, "MPC ABR did not select a valid representation. Implement abr_mpc_choose_representation().\n");
            return EXIT_FAILURE;
        }
        representation = &mpd_info.representations[selected_representation];
        if (selected_representation != previous_representation &&
            representation->initialization_template[0] != '\0') {
            if (request_build_initialization_url(config.mpd_url, representation, url, sizeof(url)) == RET_FAIL ||
                request_download_segment(url, &data, &data_size) == RET_FAIL) {
                fprintf(stderr, "Could not download initialization segment for representation %s\n", representation->id);
                return EXIT_FAILURE;
            }
            free(data);
        }
        segment_number = representation->start_number + (long)segment_index;
        if (request_build_segment_url(config.mpd_url, representation, segment_number, url, sizeof(url)) == RET_FAIL) {
            fprintf(stderr, "Could not create URL for segment %ld\n", segment_number);
            return EXIT_FAILURE;
        }
        clock_gettime(CLOCK_MONOTONIC, &start);
        if (request_download_segment(url, &data, &data_size) == RET_FAIL) {
            fprintf(stderr, "Could not download segment %ld\n", segment_number);
            return EXIT_FAILURE;
        }
        clock_gettime(CLOCK_MONOTONIC, &finish);
        download_seconds = elapsed_seconds(&start, &finish);
        if (download_seconds <= 0.0) download_seconds = 0.000001;
        if (throughput_count == MAX_THROUGHPUT_SAMPLES) {
            memmove(throughput_history, throughput_history + 1,
                    (MAX_THROUGHPUT_SAMPLES - 1) * sizeof(throughput_history[0]));
            throughput_count--;
        }
        throughput_history[throughput_count++] = (double)data_size * 8.0 / download_seconds;
        buffer_seconds -= download_seconds;
        if (buffer_seconds < 0.0) buffer_seconds = 0.0;
        buffer_seconds += representation->segment_duration_seconds;
        printf("segment=%ld rep=%s bytes=%zu download=%.3f s buffer=%.2f s\n",
               segment_number, representation->id, data_size, download_seconds, buffer_seconds);
        free(data);
        previous_representation = selected_representation;
    }
    return EXIT_SUCCESS;

invalid_option:
    print_usage(argv[0]);
    return EXIT_FAILURE;
}

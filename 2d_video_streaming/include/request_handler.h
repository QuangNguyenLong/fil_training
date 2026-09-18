#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <stddef.h>

#include "define.h"

#define MAX_REPRESENTATIONS 16
#define MAX_REPRESENTATION_ID_LENGTH 64
#define MAX_URL_LENGTH 1024

/*
 * The server-side preprocessing tool generates a static MPD with a video
 * AdaptationSet and SegmentTemplate using $RepresentationID$ and $Number%05d$.
 */
typedef struct {
    char id[MAX_REPRESENTATION_ID_LENGTH];
    long bitrate_bps;
    int width;
    int height;
    double segment_duration_seconds;
    long start_number;
    char initialization_template[MAX_URL_LENGTH];
    char media_template[MAX_URL_LENGTH];
} Representation;

typedef struct {
    Representation representations[MAX_REPRESENTATIONS];
    size_t representation_count;
} MpdInfo;

/* Set to 0 only for a local server with a self-signed development certificate. */
void request_set_tls_verification(int verify_peer);

/* fetch mpd_url, parse the MPD, and fill mpd_info. */
int request_fetch_and_parse_mpd(const char *mpd_url, MpdInfo *mpd_info);

/* replace MPD template variables and write the URL to segment_url. */
int request_build_segment_url(const char *mpd_url,
                              const Representation *representation,
                              long segment_number,
                              char *segment_url,
                              size_t segment_url_size);

/* replace initialization template variables and write the URL. */
int request_build_initialization_url(const char *mpd_url,
                                     const Representation *representation,
                                     char *initialization_url,
                                     size_t initialization_url_size);

/* download one media segment.  The caller frees *data on success. */
int request_download_segment(const char *segment_url,
                             unsigned char **data,
                             size_t *data_size);

#endif

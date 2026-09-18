#include "request_handler.h"

#include <ctype.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef struct {
    unsigned char *data;
    size_t size;
} DownloadBuffer;

typedef struct {
    char initialization[MAX_URL_LENGTH];
    char media[MAX_URL_LENGTH];
    long timescale;
    long duration;
    long start_number;
} SegmentTemplate;

static long verify_tls_peer = 1L;

void request_set_tls_verification(int verify_peer)
{
    verify_tls_peer = verify_peer ? 1L : 0L;
}

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *user_data)
{
    DownloadBuffer *buffer = user_data;
    size_t received = size * nmemb;
    unsigned char *new_data;

    if (size != 0 && received / size != nmemb) {
        return 0;
    }
    new_data = realloc(buffer->data, buffer->size + received + 1);
    if (new_data == NULL) {
        return 0;
    }
    buffer->data = new_data;
    memcpy(buffer->data + buffer->size, contents, received);
    buffer->size += received;
    buffer->data[buffer->size] = '\0';
    return received;
}

static int get_url(const char *url, unsigned char **data, size_t *data_size)
{
    CURL *curl;
    CURLcode curl_status;
    long http_status = 0;
    DownloadBuffer buffer = {NULL, 0};

    if (url == NULL || data == NULL || data_size == NULL || url[0] == '\0') {
        return RET_FAIL;
    }
    *data = NULL;
    *data_size = 0;
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        return RET_FAIL;
    }
    curl = curl_easy_init();
    if (curl == NULL) {
        return RET_FAIL;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "fil-training-dash-client/1.0");
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, verify_tls_peer);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, verify_tls_peer ? 2L : 0L);
    curl_status = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status);
    curl_easy_cleanup(curl);

    if (curl_status != CURLE_OK || http_status < 200 || http_status >= 300) {
        free(buffer.data);
        return RET_FAIL;
    }
    *data = buffer.data;
    *data_size = buffer.size;
    return RET_SUCCESS;
}

static const char *find_tag(const char *cursor, const char *limit, const char *name)
{
    const char *match;
    size_t name_length = strlen(name);

    while ((match = strstr(cursor, name)) != NULL && match < limit) {
        const char *after = match + name_length;
        if (match > cursor && match[-1] == '<' && after < limit &&
            (isspace((unsigned char)*after) || *after == '>' || *after == '/')) {
            return match - 1;
        }
        cursor = match + name_length;
    }
    return NULL;
}

static const char *find_tag_end(const char *tag, const char *limit)
{
    const char *end = strchr(tag, '>');
    return end != NULL && end < limit ? end : NULL;
}

static int read_attribute(const char *tag, const char *tag_limit, const char *name,
                          char *value, size_t value_size)
{
    const char *cursor = tag;
    const char *match;
    size_t name_length = strlen(name);

    if (value == NULL || value_size == 0) {
        return RET_FAIL;
    }
    value[0] = '\0';
    while ((match = strstr(cursor, name)) != NULL && match < tag_limit) {
        const char *p = match + name_length;
        const char *start;
        char quote;

        if (match > tag && !isspace((unsigned char)match[-1])) {
            cursor = p;
            continue;
        }
        while (p < tag_limit && isspace((unsigned char)*p)) {
            p++;
        }
        if (p >= tag_limit || *p++ != '=') {
            cursor = p;
            continue;
        }
        while (p < tag_limit && isspace((unsigned char)*p)) {
            p++;
        }
        if (p >= tag_limit || (*p != '\'' && *p != '"')) {
            return RET_FAIL;
        }
        quote = *p++;
        start = p;
        while (p < tag_limit && *p != quote) {
            p++;
        }
        if (p == tag_limit || (size_t)(p - start) >= value_size) {
            return RET_FAIL;
        }
        memcpy(value, start, (size_t)(p - start));
        value[p - start] = '\0';
        return RET_SUCCESS;
    }
    return RET_FAIL;
}

static void read_segment_template(const char *scope, const char *scope_limit,
                                  SegmentTemplate *segment_template)
{
    const char *tag = find_tag(scope, scope_limit, "SegmentTemplate");
    const char *tag_limit;
    char value[64];

    if (tag == NULL || (tag_limit = find_tag_end(tag, scope_limit)) == NULL) {
        return;
    }
    (void)read_attribute(tag, tag_limit, "initialization", segment_template->initialization,
                         sizeof(segment_template->initialization));
    (void)read_attribute(tag, tag_limit, "media", segment_template->media,
                         sizeof(segment_template->media));
    if (read_attribute(tag, tag_limit, "timescale", value, sizeof(value)) == RET_SUCCESS) {
        segment_template->timescale = strtol(value, NULL, 10);
    }
    if (read_attribute(tag, tag_limit, "duration", value, sizeof(value)) == RET_SUCCESS) {
        segment_template->duration = strtol(value, NULL, 10);
    }
    if (read_attribute(tag, tag_limit, "startNumber", value, sizeof(value)) == RET_SUCCESS) {
        segment_template->start_number = strtol(value, NULL, 10);
    }
}

static int is_video_adaptation(const char *tag, const char *tag_limit)
{
    char value[64];

    if (read_attribute(tag, tag_limit, "contentType", value, sizeof(value)) == RET_SUCCESS) {
        return strcasecmp(value, "video") == 0;
    }
    if (read_attribute(tag, tag_limit, "mimeType", value, sizeof(value)) == RET_SUCCESS) {
        return strncmp(value, "video/", 6) == 0;
    }
    return 0;
}

static int parse_mpd(const char *xml, MpdInfo *mpd_info)
{
    const char *document_end;
    const char *adaptation;
    char value[64];

    if (xml == NULL || mpd_info == NULL) {
        return RET_FAIL;
    }
    memset(mpd_info, 0, sizeof(*mpd_info));
    document_end = xml + strlen(xml);
    adaptation = xml;
    while ((adaptation = find_tag(adaptation, document_end, "AdaptationSet")) != NULL) {
        const char *adaptation_end = strstr(adaptation, "</AdaptationSet>");
        const char *adaptation_tag_end = find_tag_end(adaptation, document_end);
        const char *first_representation;
        const char *representation;
        SegmentTemplate inherited = {{0}, {0}, 1, 0, 1};

        if (adaptation_end == NULL || adaptation_tag_end == NULL) {
            return RET_FAIL;
        }
        if (!is_video_adaptation(adaptation, adaptation_tag_end)) {
            adaptation = adaptation_end + strlen("</AdaptationSet>");
            continue;
        }
        first_representation = find_tag(adaptation, adaptation_end, "Representation");
        read_segment_template(adaptation,
                              first_representation != NULL ? first_representation : adaptation_end,
                              &inherited);
        representation = adaptation;
        while ((representation = find_tag(representation, adaptation_end, "Representation")) != NULL) {
            const char *representation_tag_end = find_tag_end(representation, adaptation_end);
            const char *representation_end = strstr(representation, "</Representation>");
            SegmentTemplate local = inherited;
            Representation *out;

            if (representation_tag_end == NULL ||
                mpd_info->representation_count == MAX_REPRESENTATIONS) {
                return RET_FAIL;
            }
            if (representation_end == NULL || representation_end > adaptation_end) {
                representation_end = representation_tag_end;
            }
            read_segment_template(representation, representation_end, &local);
            out = &mpd_info->representations[mpd_info->representation_count];
            memset(out, 0, sizeof(*out));
            if (read_attribute(representation, representation_tag_end, "id", out->id,
                               sizeof(out->id)) == RET_FAIL ||
                read_attribute(representation, representation_tag_end, "bandwidth", value,
                               sizeof(value)) == RET_FAIL) {
                return RET_FAIL;
            }
            out->bitrate_bps = strtol(value, NULL, 10);
            if (out->bitrate_bps <= 0 || local.media[0] == '\0' ||
                local.timescale <= 0 || local.duration <= 0) {
                return RET_FAIL;
            }
            if (read_attribute(representation, representation_tag_end, "width", value, sizeof(value)) == RET_SUCCESS) {
                out->width = (int)strtol(value, NULL, 10);
            }
            if (read_attribute(representation, representation_tag_end, "height", value, sizeof(value)) == RET_SUCCESS) {
                out->height = (int)strtol(value, NULL, 10);
            }
            out->start_number = local.start_number > 0 ? local.start_number : 1;
            if (local.timescale > 0 && local.duration > 0) {
                out->segment_duration_seconds = (double)local.duration / (double)local.timescale;
            }
            snprintf(out->initialization_template, sizeof(out->initialization_template), "%s", local.initialization);
            snprintf(out->media_template, sizeof(out->media_template), "%s", local.media);
            mpd_info->representation_count++;
            representation = representation_end;
        }
        adaptation = adaptation_end + strlen("</AdaptationSet>");
    }
    return mpd_info->representation_count > 0 ? RET_SUCCESS : RET_FAIL;
}

int request_fetch_and_parse_mpd(const char *mpd_url, MpdInfo *mpd_info)
{
    unsigned char *data;
    size_t data_size;
    int status;

    if (get_url(mpd_url, &data, &data_size) == RET_FAIL || data_size == 0) {
        free(data);
        return RET_FAIL;
    }
    status = parse_mpd((const char *)data, mpd_info);
    free(data);
    return status;
}

static int is_absolute_url(const char *url)
{
    return strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0;
}

static int resolve_url(const char *mpd_url, const char *reference, char *result, size_t result_size)
{
    const char *last_slash;
    const char *scheme;
    const char *path;
    int written;

    if (mpd_url == NULL || reference == NULL || result == NULL || result_size == 0) {
        return RET_FAIL;
    }
    if (is_absolute_url(reference)) {
        written = snprintf(result, result_size, "%s", reference);
    } else if (reference[0] == '/') {
        scheme = strstr(mpd_url, "://");
        path = scheme == NULL ? NULL : strchr(scheme + 3, '/');
        if (scheme == NULL) {
            return RET_FAIL;
        }
        if (path == NULL) {
            written = snprintf(result, result_size, "%s%s", mpd_url, reference);
        } else {
            written = snprintf(result, result_size, "%.*s%s", (int)(path - mpd_url), mpd_url, reference);
        }
    } else {
        last_slash = strrchr(mpd_url, '/');
        if (last_slash == NULL) {
            return RET_FAIL;
        }
        written = snprintf(result, result_size, "%.*s/%s", (int)(last_slash - mpd_url), mpd_url, reference);
    }
    return written >= 0 && (size_t)written < result_size ? RET_SUCCESS : RET_FAIL;
}

static int append_text(char *output, size_t output_size, size_t *used,
                       const char *text, size_t length)
{
    if (*used + length >= output_size) {
        return RET_FAIL;
    }
    memcpy(output + *used, text, length);
    *used += length;
    output[*used] = '\0';
    return RET_SUCCESS;
}

static int expand_template(const Representation *representation, long segment_number,
                           const char *template, char *expanded, size_t expanded_size)
{
    const char *p = template;
    size_t used = 0;
    char replacement[128];

    expanded[0] = '\0';
    while (*p != '\0') {
        const char *end;
        if (*p != '$') {
            if (append_text(expanded, expanded_size, &used, p, 1) == RET_FAIL) return RET_FAIL;
            p++;
            continue;
        }
        if (p[1] == '$') {
            if (append_text(expanded, expanded_size, &used, "$", 1) == RET_FAIL) return RET_FAIL;
            p += 2;
            continue;
        }
        end = strchr(p + 1, '$');
        if (end == NULL) return RET_FAIL;
        if ((size_t)(end - p - 1) == strlen("RepresentationID") &&
            strncmp(p + 1, "RepresentationID", strlen("RepresentationID")) == 0) {
            snprintf(replacement, sizeof(replacement), "%s", representation->id);
        } else if (strncmp(p + 1, "Number", strlen("Number")) == 0) {
            const char *format = p + 1 + strlen("Number");
            if (format == end) {
                snprintf(replacement, sizeof(replacement), "%ld", segment_number);
            } else if (*format == '%' && end - format >= 2 && end - format < 16) {
                const char *q = format + 1;
                long width;
                while (q < end - 1 && isdigit((unsigned char)*q)) q++;
                if (q != end - 1 || *q != 'd') return RET_FAIL;
                width = strtol(format + 1, NULL, 10);
                if (width < 0 || width > 100) return RET_FAIL;
                if (format[1] == '0') snprintf(replacement, sizeof(replacement), "%0*ld", (int)width, segment_number);
                else snprintf(replacement, sizeof(replacement), "%*ld", (int)width, segment_number);
            } else {
                return RET_FAIL;
            }
        } else {
            return RET_FAIL;
        }
        if (append_text(expanded, expanded_size, &used, replacement, strlen(replacement)) == RET_FAIL) return RET_FAIL;
        p = end + 1;
    }
    return RET_SUCCESS;
}

int request_build_segment_url(const char *mpd_url, const Representation *representation,
                              long segment_number, char *segment_url, size_t segment_url_size)
{
    char relative_url[MAX_URL_LENGTH];

    if (representation == NULL || segment_number < 0 || representation->media_template[0] == '\0') {
        return RET_FAIL;
    }
    if (expand_template(representation, segment_number, representation->media_template,
                        relative_url, sizeof(relative_url)) == RET_FAIL) {
        return RET_FAIL;
    }
    return resolve_url(mpd_url, relative_url, segment_url, segment_url_size);
}

int request_build_initialization_url(const char *mpd_url, const Representation *representation,
                                     char *initialization_url, size_t initialization_url_size)
{
    char relative_url[MAX_URL_LENGTH];

    if (representation == NULL || representation->initialization_template[0] == '\0') {
        return RET_FAIL;
    }
    if (expand_template(representation, 0, representation->initialization_template,
                        relative_url, sizeof(relative_url)) == RET_FAIL) {
        return RET_FAIL;
    }
    return resolve_url(mpd_url, relative_url, initialization_url, initialization_url_size);
}

int request_download_segment(const char *segment_url, unsigned char **data, size_t *data_size)
{
    return get_url(segment_url, data, data_size);
}

#!/usr/bin/env bash
# Download (or accept) one video, encode an H.264 bitrate ladder, and package DASH.

set -euo pipefail

INPUT_URL=""
INPUT_FILE=""
OUTPUT_DIR=""
SEGMENT_DURATION=2

usage() {
    cat <<'EOF'
Usage: bash prepare_dash.sh (--input-url URL | --input-file FILE) --output-dir DIR [options]

Options:
  --input-url URL           Download the source video with curl.
  --input-file FILE         Use an already-downloaded source video.
  --output-dir DIR          New directory for manifest.mpd and .m4s files.
  --segment-duration N      Segment duration in whole seconds (default: 2).
  -h, --help                Show this help.

The output directory must not already exist.  Three H.264 video
representations are produced: 426x240, 640x360, and 1280x720.
EOF
}

fail() {
    printf 'error: %s\n' "$*" >&2
    exit 1
}

require_command() {
    command -v "$1" >/dev/null 2>&1 || fail "required command not found: $1"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --input-url) INPUT_URL=${2:-}; shift 2 ;;
        --input-file) INPUT_FILE=${2:-}; shift 2 ;;
        --output-dir) OUTPUT_DIR=${2:-}; shift 2 ;;
        --segment-duration) SEGMENT_DURATION=${2:-}; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) fail "unknown option: $1" ;;
    esac
done

[[ -n "$OUTPUT_DIR" ]] || fail "--output-dir is required"
[[ -n "$INPUT_URL" || -n "$INPUT_FILE" ]] || fail "an input URL or input file is required"
[[ -z "$INPUT_URL" || -z "$INPUT_FILE" ]] || fail "choose only one input source"
[[ "$SEGMENT_DURATION" =~ ^[1-9][0-9]*$ ]] || fail "--segment-duration must be a positive whole number"
[[ ! -e "$OUTPUT_DIR" ]] || fail "output directory already exists: $OUTPUT_DIR"

require_command ffmpeg
require_command ffprobe
if [[ -n "$INPUT_URL" ]]; then
    require_command curl
fi

if ! ffmpeg -hide_banner -encoders 2>/dev/null | grep -q 'libx264'; then
    fail "ffmpeg was built without the libx264 encoder"
fi

output_parent=$(dirname "$OUTPUT_DIR")
mkdir -p "$output_parent"
stage_dir=$(mktemp -d "$output_parent/.dash-stage.XXXXXX")
download_file=""

cleanup() {
    rm -rf "$stage_dir"
    if [[ -n "$download_file" ]]; then
        rm -f "$download_file"
    fi
}
trap cleanup EXIT

if [[ -n "$INPUT_URL" ]]; then
    download_file=$(mktemp "${TMPDIR:-/tmp}/dash-source.XXXXXX.mp4")
    printf 'Downloading source video...\n'
    curl --fail --location --retry 3 --connect-timeout 15 --output "$download_file" "$INPUT_URL"
    input_video=$download_file
else
    [[ -f "$INPUT_FILE" ]] || fail "input file does not exist: $INPUT_FILE"
    input_video=$INPUT_FILE
fi

if ! ffprobe -v error -select_streams v:0 -show_entries stream=codec_type \
    -of default=nokey=1:noprint_wrappers=1 "$input_video" | grep -qx 'video'; then
    fail "input has no video stream"
fi

frame_rate=$(ffprobe -v error -select_streams v:0 -show_entries stream=r_frame_rate \
    -of default=nokey=1:noprint_wrappers=1 "$input_video")
frames_per_second=$(awk -F/ '
    NF == 2 && $2 > 0 { value = $1 / $2 }
    NF == 1 && $1 > 0 { value = $1 }
    END { if (value > 0) printf "%d", value + 0.5; else print 30 }
' <<<"$frame_rate")
gop_size=$((frames_per_second * SEGMENT_DURATION))

has_audio=0
if ffprobe -v error -select_streams a:0 -show_entries stream=index \
    -of default=nokey=1:noprint_wrappers=1 "$input_video" | grep -q .; then
    has_audio=1
fi

ffmpeg_args=(
    -hide_banner -y -i "$input_video"
    -map 0:v:0 -map 0:v:0 -map 0:v:0
    -filter:v:0 'scale=w=426:h=240:force_original_aspect_ratio=decrease,pad=426:240:(ow-iw)/2:(oh-ih)/2'
    -filter:v:1 'scale=w=640:h=360:force_original_aspect_ratio=decrease,pad=640:360:(ow-iw)/2:(oh-ih)/2'
    -filter:v:2 'scale=w=1280:h=720:force_original_aspect_ratio=decrease,pad=1280:720:(ow-iw)/2:(oh-ih)/2'
    -c:v libx264 -pix_fmt yuv420p -preset medium
    -b:v:0 400k -maxrate:v:0 428k -bufsize:v:0 600k
    -b:v:1 800k -maxrate:v:1 856k -bufsize:v:1 1200k
    -b:v:2 2500k -maxrate:v:2 2675k -bufsize:v:2 3750k
    -g "$gop_size" -keyint_min "$gop_size" -sc_threshold 0
    -force_key_frames "expr:gte(t,n_forced*${SEGMENT_DURATION})"
)

if [[ "$has_audio" -eq 1 ]]; then
    ffmpeg_args+=( -map 0:a:0 -c:a aac -b:a 128k -ac 2 )
    adaptation_sets='id=0,streams=v id=1,streams=a'
else
    adaptation_sets='id=0,streams=v'
fi

init_name='init-$RepresentationID$.m4s'
media_name='chunk-$RepresentationID$-$Number%05d$.m4s'
ffmpeg_args+=(
    -use_template 1 -use_timeline 0 -seg_duration "$SEGMENT_DURATION"
    -adaptation_sets "$adaptation_sets"
    -init_seg_name "$init_name" -media_seg_name "$media_name"
    -f dash "$stage_dir/manifest.mpd"
)

printf 'Encoding three video representations and packaging DASH...\n'
ffmpeg "${ffmpeg_args[@]}"

[[ -s "$stage_dir/manifest.mpd" ]] || fail "ffmpeg did not create manifest.mpd"
grep -Fq '<SegmentTemplate' "$stage_dir/manifest.mpd" || fail "MPD has no SegmentTemplate"
grep -Fq 'chunk-$RepresentationID$-$Number%05d$.m4s' "$stage_dir/manifest.mpd" || \
    fail "MPD media template differs from the client exercise contract"

mv "$stage_dir" "$OUTPUT_DIR"
trap - EXIT
if [[ -n "$download_file" ]]; then
    rm -f "$download_file"
fi
printf 'Created DASH output: %s/manifest.mpd\n' "$OUTPUT_DIR"

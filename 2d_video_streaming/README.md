# Educational 2D DASH client

This project is a small C client for learning the main parts of adaptive video
streaming: request handling, bandwidth estimation, and MPC-based ABR.  It
downloads an MPD, obtains media segments, measures achieved throughput, and
passes streaming state to the ABR function.

If the idea of a *module* is new, first read [the C-project module
tutorial](../c_cpp/c/README.md#iii-module-your-code).  In this project, a
module is one focused header/source pair: its header states the interface and
its source file contains the logic for one responsibility.

---

## I. Directory structure

```
2d_video_streaming/
├── include/
│   ├── define.h            # Shared RET_SUCCESS / RET_FAIL values
│   ├── request_handler.h   # MPD and HTTP data structures and API
│   ├── bw_pred.h           # Bandwidth-prediction student API
│   ├── abr.h               # MPC ABR student API and configuration
│   └── metric.h            # Reserved for optional evaluation metrics
├── src/
│   ├── request_handler.c   # Instructor-provided MPD/HTTP implementation
│   ├── bw_pred.c           # Student bandwidth-prediction implementation
│   ├── abr.c               # Student MPC implementation
│   └── metric.c            # Optional future work
├── main.c                  # Instructor-provided CLI and streaming loop
├── CMakeLists.txt          # Build configuration
└── README.md
```

---

## II. Module responsibilities

| Module | Role | Who implements the logic? |
| --- | --- | --- |
| Request handler | Fetches/parses the MPD, expands segment templates, and downloads initialization/media segments. | Instructor-provided in [src/request_handler.c](src/request_handler.c). |
| Bandwidth prediction | Converts prior throughput observations into a future throughput estimate. | Student: [src/bw_pred.c](src/bw_pred.c), using [include/bw_pred.h](include/bw_pred.h). |
| ABR | Uses model predictive control (MPC) to select the next representation. | Student: [src/abr.c](src/abr.c), using [include/abr.h](include/abr.h). |
| CLI/streaming loop | Reads experiment parameters, records throughput history and buffer state, and invokes MPC. | Instructor-provided in [main.c](main.c). |
| Metrics | A place for QoE, rebuffering, bitrate, or switch-count evaluation later. | Optional student extension. |

The MPC function receives the representation ladder, recent throughput history,
current buffer level, next segment position, previous quality, and the
look-ahead/QoE parameters.  Students should implement only the body of
`abr_mpc_choose_representation()`; do not change its interface unless the
experiment itself changes.

---

## III. Build

Install a C compiler, CMake, Make, and libcurl development files:

```sh
sudo apt install build-essential cmake libcurl4-openssl-dev
```

Build from this directory:

```sh
cmake -S . -B build
cmake --build build
```

The executable is `build/dash_abr_client`.  Display all experiment settings
with:

```sh
./build/dash_abr_client --help
```

---

## IV. Test each module

### 1. Request handler and MPD parser

First prepare and serve content by following the
[server guide](../2d_DASH_server/README.md).  Then run parser-only mode; it
does not call the unfinished ABR function:

```sh
./build/dash_abr_client \
  --mpd-url https://localhost:8443/manifest.mpd \
  --insecure-tls \
  --list-representations
```

Expected output lists the three video representations, including each ID,
bitrate, resolution, and segment duration.  `--insecure-tls` is only for the
self-signed local certificate created in the server guide.

### 2. Bandwidth prediction

Implement `bw_predict()` in [src/bw_pred.c](src/bw_pred.c).  Test it with a
small deterministic history, for example `500000, 700000, 600000` bps.  A good
module test checks that it returns `RET_SUCCESS`, uses no more than the chosen
prediction window, returns a positive result, and returns `RET_FAIL` for an
empty history or invalid output pointer.  Compile the module alone to catch
syntax and header errors:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Iinclude -c src/bw_pred.c -o /tmp/bw_pred.o
```

### 3. MPC ABR

Implement `abr_mpc_choose_representation()` in [src/abr.c](src/abr.c).  Unit
test it with a hand-written `MpdInfo` containing at least three bitrates and a
short throughput history.  Check that the returned index is smaller than
`representation_count`, that low buffer prefers a safe quality, and that an
empty history still selects a valid startup quality.  Compile it alone with:

```sh
cc -std=c11 -Wall -Wextra -Wpedantic -Iinclude -c src/abr.c -o /tmp/abr.o
```

---

## V. Run the complete experiment

In terminal 1, serve `dash_output` with `nghttpd` as shown in the server guide.
After implementing both student functions, use terminal 2:

```sh
cd 2d_video_streaming
./build/dash_abr_client \
  --mpd-url https://localhost:8443/manifest.mpd \
  --insecure-tls \
  --segments 10 \
  --prediction-window 5 \
  --lookahead 5 \
  --target-buffer 10 \
  --rebuffer-penalty 4.3 \
  --smoothness-penalty 1.0
```

The client prints each selected representation, bytes received, download time,
and current buffer.  It records successful media-segment throughput samples
and supplies the most recent `--prediction-window` samples to MPC.

Without a completed `abr_mpc_choose_representation()` implementation, the
program stops after parsing the MPD and reports that MPC did not choose a valid
representation.  That failure is expected for the starter code.

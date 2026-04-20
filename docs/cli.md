# `beeping-core` CLI

The `beeping-core` binary is a thin wrapper around the C++ library. It lets
you decode a WAV file locally without writing any code — useful for debugging,
verifying an encoded payload round-trips, or closing the loop in a Beeping
tutorial (e.g. the [First Beep quest](https://beeping.io/quest)).

## Install

Build the library from source (see [the main README](../README.md)) —
the binary is built alongside the library and ends up in
`build/<BuildType>/cli/beeping-core` (or `beeping-core.exe` on Windows).

Install system-wide with:

```bash
cmake --install build/Release --prefix /usr/local
```

## Commands

### `decode`

Decode a WAV file and print the payload to stdout.

```text
beeping-core decode [OPTIONS] <audio.wav>

Options:
  -m, --mode N   2 = audible, 3 = inaudible, 5 = all (default: 5)
  -h, --help     Show this help
```

**Exit codes:**

| Code | Meaning |
|---|---|
| `0` | Decoded successfully — payload on stdout |
| `1` | No beep detected in the audio |
| `2` | Invalid input (missing file, unreadable WAV, bad args) |

**Supported WAV formats:**

- PCM integer, 16-bit (format 1)
- IEEE float, 32-bit (format 3)
- Mono or stereo (stereo is downmixed by channel averaging)
- Any sample rate supported by BeepingCore (commonly 44100, 48000, 96000 Hz)

**Example:**

```bash
beeping-core decode message.wav
# hello
echo "exit=$?"
# exit=0
```

### `version`

Print the BeepingCore library version and exit.

```bash
beeping-core version
# BeepingCoreLib version 1.0.0 [20220426]
```

### `help`

Print the top-level help.

```bash
beeping-core help
```

## Round-trip with `beepbox` cloud

A full end-to-end round-trip (requires a Beeping API key):

```bash
# 1. Encode via the cloud beepbox API
curl -X POST https://api.beeping.io/v1/encode \
  -H "Authorization: Bearer bk_XXXXXXXX" \
  -H "Content-Type: application/json" \
  -d '{"key": "a1b2c", "mode": "inaudible"}' \
  -o output.wav

# 2. Decode locally with beeping-core
beeping-core decode -m 3 output.wav
# a1b2c
```

## Troubleshooting

**`cannot open file: X`** — check the path and file permissions.

**`only PCM integer (1) or IEEE float (3) supported`** — the WAV file uses
an unsupported format. Re-encode to 16-bit PCM or 32-bit float WAV with
a tool like `ffmpeg -i input.wav -c:a pcm_s16le out.wav`.

**Exit code 1 (no beep)** — the audio likely doesn't contain a Beeping
signal in the selected mode. Try `-m 5` (tries both audible and inaudible)
or verify the recording captured the full beep sequence.

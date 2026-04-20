#include "decode_cmd.h"

#include <BeepingCoreLib_api.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "wav_reader.h"

namespace beeping::cli {

namespace {

constexpr int kDefaultMode = BEEPING_MODE_ALL;  // tries audible + inaudible
constexpr int kBufferSize = 1024;

void print_decode_help() {
  std::fprintf(stderr,
               "Usage: beeping-core decode [OPTIONS] <audio.wav>\n"
               "\n"
               "Decode a WAV file produced by beeping-core or beepbox.\n"
               "\n"
               "Options:\n"
               "  -m, --mode N   Decode mode: 2=audible, 3=inaudible, 5=all "
               "(default: 5)\n"
               "  -h, --help     Show this help\n"
               "\n"
               "Exit codes:\n"
               "  0  decoded successfully (payload on stdout)\n"
               "  1  no beep decoded in the audio\n"
               "  2  invalid input / unreadable WAV\n");
}

}  // namespace

int decode_cmd(int argc, char** argv) {
  int mode = kDefaultMode;
  std::string path;

  for (int i = 0; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      print_decode_help();
      return 0;
    }
    if (arg == "-m" || arg == "--mode") {
      if (i + 1 >= argc) {
        std::fprintf(stderr, "Error: --mode requires an argument\n");
        return 2;
      }
      try {
        mode = std::stoi(argv[++i]);
      } catch (...) {
        std::fprintf(stderr, "Error: --mode must be an integer\n");
        return 2;
      }
      if (mode != BEEPING_MODE_AUDIBLE && mode != BEEPING_MODE_INAUDIBLE &&
          mode != BEEPING_MODE_ALL) {
        std::fprintf(stderr,
                     "Error: --mode must be 2 (audible), 3 (inaudible) or 5 "
                     "(all)\n");
        return 2;
      }
    } else if (!arg.empty() && arg[0] != '-') {
      if (!path.empty()) {
        std::fprintf(stderr, "Error: multiple input files not supported\n");
        return 2;
      }
      path = arg;
    } else {
      std::fprintf(stderr, "Error: unknown option '%s'\n", arg.c_str());
      return 2;
    }
  }

  if (path.empty()) {
    print_decode_help();
    return 2;
  }

  WavData wav;
  if (std::string err = read_wav_file(path, wav); !err.empty()) {
    std::fprintf(stderr, "Error reading WAV: %s\n", err.c_str());
    return 2;
  }
  if (wav.samples.empty()) {
    std::fprintf(stderr, "Error: WAV contains no audio samples\n");
    return 2;
  }

  void* handle = BEEPING_Create();
  if (!handle) {
    std::fprintf(stderr, "Error: BEEPING_Create failed\n");
    return 2;
  }

  if (BEEPING_Configure(mode, wav.sample_rate, kBufferSize, handle) != 0) {
    std::fprintf(stderr,
                 "Error: BEEPING_Configure failed for mode=%d sr=%.0f\n", mode,
                 wav.sample_rate);
    BEEPING_Destroy(handle);
    return 2;
  }

  bool word_decoded = false;
  for (std::size_t i = 0; i < wav.samples.size(); i += kBufferSize) {
    int chunk = static_cast<int>(
        std::min<std::size_t>(kBufferSize, wav.samples.size() - i));
    int result = BEEPING_DecodeAudioBuffer(wav.samples.data() + i, chunk,
                                           handle);
    if (result == -3) {
      word_decoded = true;
      break;
    }
  }

  // If not decoded yet, flush with silence to push remaining frames through
  // the decoder's internal buffer (up to ~5s of audio at 44.1kHz).
  if (!word_decoded) {
    std::vector<float> silence(kBufferSize, 0.0f);
    for (int flush = 0; flush < 200; ++flush) {
      int result =
          BEEPING_DecodeAudioBuffer(silence.data(), kBufferSize, handle);
      if (result == -3) {
        word_decoded = true;
        break;
      }
    }
  }

  int exit_code = 1;
  if (word_decoded) {
    char decoded[32] = {0};
    int len = BEEPING_GetDecodedData(decoded, handle);
    if (len > 0 && len < static_cast<int>(sizeof(decoded))) {
      decoded[len] = '\0';
      std::fwrite(decoded, 1, static_cast<std::size_t>(len), stdout);
      std::fputc('\n', stdout);
      exit_code = 0;
    } else if (len < 0) {
      std::fprintf(
          stderr,
          "Warning: decoded data failed integrity checks (length=%d)\n",
          -len);
      exit_code = 1;
    }
  }

  BEEPING_Destroy(handle);
  return exit_code;
}

}  // namespace beeping::cli

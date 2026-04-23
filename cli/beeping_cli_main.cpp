// beeping-core CLI — entry point.
//
// Usage:
//   beeping-core decode <audio.wav>   Decode a WAV to stdout
//   beeping-core version              Print library + server versions
//   beeping-core help                 Show this help

#include <BeepingCoreLib_api.h>

#include <cstdio>
#include <cstring>
#include <string>

#include "decode_cmd.h"

namespace {

int print_help() {
  std::printf(
      "beeping-core — CLI for the Beeping data-over-sound library.\n"
      "\n"
      "Usage:\n"
      "  beeping-core decode [OPTIONS] <audio.wav>\n"
      "  beeping-core version\n"
      "  beeping-core help\n"
      "\n"
      "Commands:\n"
      "  decode     Decode a WAV file and print the payload to stdout.\n"
      "  version    Print the BeepingCore library version.\n"
      "  help       Show this help.\n"
      "\n"
      "Run 'beeping-core decode --help' for decode-specific options.\n");
  return 0;
}

int print_version() {
  std::printf("%s\n", BEEPING_GetVersion());
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    return print_help();
  }

  const std::string cmd = argv[1];
  if (cmd == "help" || cmd == "--help" || cmd == "-h") {
    return print_help();
  }
  if (cmd == "version" || cmd == "--version" || cmd == "-v") {
    return print_version();
  }
  if (cmd == "decode") {
    // Forward remaining args (starting at argv[2]) to decode_cmd.
    return beeping::cli::decode_cmd(argc - 2, argv + 2);
  }

  std::fprintf(stderr,
               "Error: unknown command '%s'. Try 'beeping-core help'.\n",
               cmd.c_str());
  return 2;
}

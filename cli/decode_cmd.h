#ifndef BEEPING_CLI_DECODE_CMD_H
#define BEEPING_CLI_DECODE_CMD_H

namespace beeping::cli {

// Runs the `decode` subcommand.
// Exit codes:
//   0 = decoded successfully (payload printed to stdout)
//   1 = audio processed but nothing decoded (no valid beep found)
//   2 = invalid input (bad args, missing file, unreadable WAV, etc.)
int decode_cmd(int argc, char** argv);

}  // namespace beeping::cli

#endif  // BEEPING_CLI_DECODE_CMD_H

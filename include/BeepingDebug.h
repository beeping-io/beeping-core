#ifndef __BEEPINGDEBUG__
#define __BEEPINGDEBUG__

// BeepingDebug.h — Structured logging for beeping-core via spdlog.
//
// All logging goes through spdlog with JSON-formatted output to rotating
// files in logs/. Log level is configurable via BEEPING_LOG_LEVEL env var.
//
// Macros: BTRACE, BDEBUG, BINFO, BWARN, BERROR — same interface as before,
// now backed by spdlog. Auto-initializes on first use.

#include <spdlog/spdlog.h>

namespace BEEPING {

// Initialize the beeping logger (safe to call multiple times).
// Creates a rotating file logger in logs/ with JSON pattern.
// Reads BEEPING_LOG_LEVEL env var (trace/debug/info/warn/error, default: warn).
void initBeepingLogger();

// Shutdown (call from last BEEPING_Destroy)
void shutdownBeepingLogger();

// Ensure logger is initialized before use
void ensureBeepingLogger();

}  // namespace BEEPING

// Strip path to filename only (cross-platform — handles both / and \).
// Runtime strrchr is fine for debug logging; not on hot paths.
namespace BEEPING {
inline const char* beeping_filename(const char* path) noexcept {
  const char* slash = nullptr;
  for (const char* p = path; *p; ++p) {
    if (*p == '/' || *p == '\\') {
      slash = p;
    }
  }
  return slash ? slash + 1 : path;
}
}  // namespace BEEPING

#define BEEPING_FILENAME_ BEEPING::beeping_filename(__FILE__)

#define BTRACE(fmt, ...)                                \
  do {                                                  \
    BEEPING::ensureBeepingLogger();                     \
    spdlog::trace("[{}:{}] " fmt, BEEPING_FILENAME_,    \
                  __LINE__ __VA_OPT__(, ) __VA_ARGS__); \
  } while (0)

#define BDEBUG(fmt, ...)                                \
  do {                                                  \
    BEEPING::ensureBeepingLogger();                     \
    spdlog::debug("[{}:{}] " fmt, BEEPING_FILENAME_,    \
                  __LINE__ __VA_OPT__(, ) __VA_ARGS__); \
  } while (0)

#define BINFO(fmt, ...)                                \
  do {                                                 \
    BEEPING::ensureBeepingLogger();                    \
    spdlog::info("[{}:{}] " fmt, BEEPING_FILENAME_,    \
                 __LINE__ __VA_OPT__(, ) __VA_ARGS__); \
  } while (0)

#define BWARN(fmt, ...)                                \
  do {                                                 \
    BEEPING::ensureBeepingLogger();                    \
    spdlog::warn("[{}:{}] " fmt, BEEPING_FILENAME_,    \
                 __LINE__ __VA_OPT__(, ) __VA_ARGS__); \
  } while (0)

#define BERROR(fmt, ...)                                \
  do {                                                  \
    BEEPING::ensureBeepingLogger();                     \
    spdlog::error("[{}:{}] " fmt, BEEPING_FILENAME_,    \
                  __LINE__ __VA_OPT__(, ) __VA_ARGS__); \
  } while (0)

#endif  // __BEEPINGDEBUG__

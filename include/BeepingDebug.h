#ifndef __BEEPINGDEBUG__
#define __BEEPINGDEBUG__

// BeepingDebug.h — Verbose debug logging for beeping-core.
//
// Activated by compiling with -DBEEPING_DEBUG_LOG (CMake option
// BEEPING_ENABLE_DEBUG_LOG). In release builds everything compiles to
// nothing — zero overhead.
//
// Logs are written to files in a `logs/` directory with automatic
// rotation (max 10 files, max 7 days). One file per session (process
// start), thread-safe.

#ifdef BEEPING_DEBUG_LOG

#include <cstdio>

namespace BEEPING {

enum class LogLevel { Trace = 0, Debug = 1, Info = 2, Warn = 3, Error = 4 };

class BeepingLogger {
 public:
  static BeepingLogger& instance();

  // Configuration (call before first log, or use defaults)
  void setLogDir(const char* dir);  // default: "logs/"
  void setMaxFiles(int n);          // default: 10
  void setMaxAgeDays(int days);     // default: 7
  void setLevel(LogLevel min);      // default: Trace

  void log(LogLevel lvl, const char* file, int line, const char* func,
           const char* fmt, ...);

  // Called on first BEEPING_Create — opens file, purges old logs
  void openSession();

  // Called on last BEEPING_Destroy — flush + close
  void closeSession();

  ~BeepingLogger();

 private:
  BeepingLogger();
  BeepingLogger(const BeepingLogger&) = delete;
  BeepingLogger& operator=(const BeepingLogger&) = delete;

  void purgeOldLogs();
  void ensureOpen();

  char logDir_[256];
  char currentFile_[512];
  int maxFiles_;
  int maxAgeDays_;
  LogLevel minLevel_;
  FILE* file_;
  bool opened_;
  int sessionCount_;  // number of active handles
};

}  // namespace BEEPING

// Strip path to filename only
#define BEEPING_FILENAME_                                                  \
  (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 \
                                    : __FILE__)

#define BTRACE(fmt, ...)                                               \
  BEEPING::BeepingLogger::instance().log(                              \
      BEEPING::LogLevel::Trace, BEEPING_FILENAME_, __LINE__, __func__, \
      fmt __VA_OPT__(, ) __VA_ARGS__)

#define BDEBUG(fmt, ...)                                               \
  BEEPING::BeepingLogger::instance().log(                              \
      BEEPING::LogLevel::Debug, BEEPING_FILENAME_, __LINE__, __func__, \
      fmt __VA_OPT__(, ) __VA_ARGS__)

#define BINFO(fmt, ...)                                               \
  BEEPING::BeepingLogger::instance().log(                             \
      BEEPING::LogLevel::Info, BEEPING_FILENAME_, __LINE__, __func__, \
      fmt __VA_OPT__(, ) __VA_ARGS__)

#define BWARN(fmt, ...)                                               \
  BEEPING::BeepingLogger::instance().log(                             \
      BEEPING::LogLevel::Warn, BEEPING_FILENAME_, __LINE__, __func__, \
      fmt __VA_OPT__(, ) __VA_ARGS__)

#define BERROR(fmt, ...)                                               \
  BEEPING::BeepingLogger::instance().log(                              \
      BEEPING::LogLevel::Error, BEEPING_FILENAME_, __LINE__, __func__, \
      fmt __VA_OPT__(, ) __VA_ARGS__)

#else  // !BEEPING_DEBUG_LOG

#define BTRACE(fmt, ...) ((void)0)
#define BDEBUG(fmt, ...) ((void)0)
#define BINFO(fmt, ...) ((void)0)
#define BWARN(fmt, ...) ((void)0)
#define BERROR(fmt, ...) ((void)0)

#endif  // BEEPING_DEBUG_LOG

#endif  // __BEEPINGDEBUG__

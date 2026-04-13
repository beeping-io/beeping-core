#ifdef BEEPING_DEBUG_LOG

#include <BeepingDebug.h>

#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <mutex>
#include <string>
#include <vector>

// Platform-specific directory listing
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define beeping_mkdir(d) _mkdir(d)
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#define beeping_mkdir(d) mkdir(d, 0755)
#endif

namespace BEEPING {

static std::mutex g_log_mutex;

static const char* levelTag(LogLevel lvl) {
  switch (lvl) {
    case LogLevel::Trace:
      return "TRC";
    case LogLevel::Debug:
      return "DBG";
    case LogLevel::Info:
      return "INF";
    case LogLevel::Warn:
      return "WRN";
    case LogLevel::Error:
      return "ERR";
  }
  return "???";
}

// ISO-8601 timestamp with milliseconds
static void formatTimestamp(char* buf, size_t bufSize) {
  auto now = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) %
            1000;
  auto tt = std::chrono::system_clock::to_time_t(now);
  struct tm tm{};
#ifdef _WIN32
  localtime_s(&tm, &tt);
#else
  localtime_r(&tt, &tm);
#endif
  int n =
      std::snprintf(buf, bufSize, "%04d-%02d-%02dT%02d:%02d:%02d.%03d",
                    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
                    tm.tm_min, tm.tm_sec, static_cast<int>(ms.count()));
  (void)n;
}

// Generate session filename
static void formatSessionFilename(char* buf, size_t bufSize,
                                  const char* logDir) {
  auto now = std::chrono::system_clock::now();
  auto tt = std::chrono::system_clock::to_time_t(now);
  struct tm tm{};
#ifdef _WIN32
  localtime_s(&tm, &tt);
  int pid = _getpid();
#else
  localtime_r(&tt, &tm);
  int pid = static_cast<int>(getpid());
#endif
  std::snprintf(buf, bufSize,
                "%sbeeping-debug-%04d%02d%02dT%02d%02d%02d-pid%d.log", logDir,
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
                tm.tm_min, tm.tm_sec, pid);
}

BeepingLogger::BeepingLogger()
    : maxFiles_(10),
      maxAgeDays_(7),
      minLevel_(LogLevel::Trace),
      file_(nullptr),
      opened_(false),
      sessionCount_(0) {
  std::strncpy(logDir_, "logs/", sizeof(logDir_) - 1);
  logDir_[sizeof(logDir_) - 1] = '\0';
  currentFile_[0] = '\0';
}

BeepingLogger::~BeepingLogger() {
  if (file_) {
    std::fclose(file_);
    file_ = nullptr;
  }
}

BeepingLogger& BeepingLogger::instance() {
  static BeepingLogger inst;
  return inst;
}

void BeepingLogger::setLogDir(const char* dir) {
  std::lock_guard<std::mutex> lk(g_log_mutex);
  std::strncpy(logDir_, dir, sizeof(logDir_) - 1);
  logDir_[sizeof(logDir_) - 1] = '\0';
  // Ensure trailing slash
  size_t len = std::strlen(logDir_);
  if (len > 0 && logDir_[len - 1] != '/') {
    if (len < sizeof(logDir_) - 1) {
      logDir_[len] = '/';
      logDir_[len + 1] = '\0';
    }
  }
}

void BeepingLogger::setMaxFiles(int n) {
  std::lock_guard<std::mutex> lk(g_log_mutex);
  maxFiles_ = n;
}

void BeepingLogger::setMaxAgeDays(int days) {
  std::lock_guard<std::mutex> lk(g_log_mutex);
  maxAgeDays_ = days;
}

void BeepingLogger::setLevel(LogLevel min) {
  std::lock_guard<std::mutex> lk(g_log_mutex);
  minLevel_ = min;
}

void BeepingLogger::openSession() {
  std::lock_guard<std::mutex> lk(g_log_mutex);
  sessionCount_++;
  if (opened_) return;

  // Create directory
  beeping_mkdir(logDir_);

  // Purge old logs before opening new one
  purgeOldLogs();

  // Open new log file
  formatSessionFilename(currentFile_, sizeof(currentFile_), logDir_);
  file_ = std::fopen(currentFile_, "w");
  opened_ = (file_ != nullptr);

  if (opened_) {
    char ts[64];
    formatTimestamp(ts, sizeof(ts));
    std::fprintf(file_,
                 "=== BeepingCore debug session started at %s ===\n"
                 "=== Log file: %s ===\n\n",
                 ts, currentFile_);
    std::fflush(file_);
  }
}

void BeepingLogger::closeSession() {
  std::lock_guard<std::mutex> lk(g_log_mutex);
  sessionCount_--;
  if (sessionCount_ <= 0 && file_) {
    char ts[64];
    formatTimestamp(ts, sizeof(ts));
    std::fprintf(file_, "\n=== BeepingCore debug session ended at %s ===\n",
                 ts);
    std::fclose(file_);
    file_ = nullptr;
    opened_ = false;
    sessionCount_ = 0;
  }
}

void BeepingLogger::ensureOpen() {
  // NOTE: caller already holds g_log_mutex — do NOT call openSession()
  // which would deadlock on the same mutex.  Instead duplicate the minimal
  // open logic inline.
  if (!opened_) {
    beeping_mkdir(logDir_);
    purgeOldLogs();
    formatSessionFilename(currentFile_, sizeof(currentFile_), logDir_);
    file_ = std::fopen(currentFile_, "w");
    opened_ = (file_ != nullptr);
    if (opened_) {
      char ts[64];
      formatTimestamp(ts, sizeof(ts));
      std::fprintf(file_,
                   "=== BeepingCore debug session started (auto) at %s ===\n"
                   "=== Log file: %s ===\n\n",
                   ts, currentFile_);
      std::fflush(file_);
    }
  }
}

void BeepingLogger::purgeOldLogs() {
  // Collect log files in logDir_
  std::vector<std::string> logFiles;

#ifndef _WIN32
  DIR* dir = opendir(logDir_);
  if (!dir) return;

  auto now = std::time(nullptr);
  double maxAgeSeconds = maxAgeDays_ * 86400.0;

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    if (std::strncmp(entry->d_name, "beeping-debug-", 14) != 0) continue;
    std::string fullPath = std::string(logDir_) + entry->d_name;

    struct stat st{};
    if (stat(fullPath.c_str(), &st) == 0) {
      double age = std::difftime(now, st.st_mtime);
      if (age > maxAgeSeconds) {
        std::remove(fullPath.c_str());
      } else {
        logFiles.push_back(fullPath);
      }
    }
  }
  closedir(dir);
#endif

  // Enforce maxFiles_ (keep newest)
  if (static_cast<int>(logFiles.size()) >= maxFiles_) {
    std::sort(logFiles.begin(), logFiles.end());
    int toRemove =
        static_cast<int>(logFiles.size()) - (maxFiles_ - 1);  // -1 for new
    for (int i = 0; i < toRemove; i++) {
      std::remove(logFiles[static_cast<size_t>(i)].c_str());
    }
  }
}

void BeepingLogger::log(LogLevel lvl, const char* file, int line,
                        const char* func, const char* fmt, ...) {
  if (lvl < minLevel_) return;

  std::lock_guard<std::mutex> lk(g_log_mutex);
  ensureOpen();
  if (!file_) return;

  char ts[64];
  formatTimestamp(ts, sizeof(ts));

  std::fprintf(file_, "[%s] %s %s:%d %s | ", levelTag(lvl), ts, file, line,
               func);

  va_list args;
  va_start(args, fmt);
  std::vfprintf(file_, fmt, args);
  va_end(args);

  std::fputc('\n', file_);
  std::fflush(file_);
}

}  // namespace BEEPING

#else

// Empty translation unit produces ranlib warnings on macOS.
// Provide a dummy symbol to suppress them.
namespace BEEPING {
void beeping_debug_placeholder_() {}
}  // namespace BEEPING

#endif  // BEEPING_DEBUG_LOG

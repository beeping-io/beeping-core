#include <BeepingDebug.h>
#include <spdlog/spdlog.h>

#ifdef __ANDROID__
#include <spdlog/sinks/android_sink.h>
#else
#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#endif

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <string>

#ifdef _WIN32
#include <direct.h>
#define beeping_mkdir(d) _mkdir(d)
#else
#include <sys/stat.h>
#define beeping_mkdir(d) mkdir(d, 0755)
#endif

namespace BEEPING {

static constexpr const char* kDefaultLogPath = "logs/beeping.log";
static std::mutex g_init_mutex;
static std::atomic<bool> g_initialized{false};
static std::atomic<int> g_session_count{0};
static std::string g_log_path = kDefaultLogPath;

void ensureBeepingLogger() {
  if (g_initialized.load(std::memory_order_acquire)) return;
  initBeepingLogger();
}

int setLogPath(const char* absolutePath) {
#ifdef __ANDROID__
  (void)absolutePath;
  return 0;
#else
  std::lock_guard<std::mutex> lk(g_init_mutex);
  if (g_initialized.load(std::memory_order_relaxed)) return -1;
  if (absolutePath == nullptr) {
    g_log_path = kDefaultLogPath;
    return 0;
  }
  if (*absolutePath == '\0') return -2;
  g_log_path = absolutePath;
  return 0;
#endif
}

void initBeepingLogger() {
  std::lock_guard<std::mutex> lk(g_init_mutex);
  if (g_initialized.load(std::memory_order_relaxed)) {
    g_session_count.fetch_add(1);
    return;
  }

  std::shared_ptr<spdlog::sinks::sink> sink;
  std::string sink_kind;

#ifdef __ANDROID__
  // Logcat sink — no filesystem dependency, immune to a read-only cwd.
  sink = std::make_shared<spdlog::sinks::android_sink_mt>("BeepingCore");
  sink_kind = "android(logcat)";
#else
  // Best-effort: if the path is the legacy default, try to create the
  // companion `logs/` directory. Failure is fine; the open below will
  // surface any real problem.
  if (g_log_path == kDefaultLogPath) {
    beeping_mkdir("logs");
  }

  try {
    sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        g_log_path, 5 * 1024 * 1024, 10);
    sink_kind = "file:" + g_log_path;
  } catch (const spdlog::spdlog_ex& ex) {
    // Unwritable target — drop logs silently instead of crashing the host
    // process. One-time warning to stderr so the misconfiguration is
    // discoverable in dev.
    std::fprintf(
        stderr,
        "[beeping-core] log path %s unwritable (%s); logs disabled. Call "
        "BEEPING_SetLogPath() before BEEPING_Create() to redirect.\n",
        g_log_path.c_str(), ex.what());
    sink = std::make_shared<spdlog::sinks::null_sink_mt>();
    sink_kind = "null(unwritable)";
  }
#endif

  sink->set_pattern(
      R"({"ts":"%Y-%m-%dT%H:%M:%S.%e","level":"%l","msg":"%v"})");

  auto logger = std::make_shared<spdlog::logger>("beeping", sink);

  // Read log level from env var BEEPING_LOG_LEVEL
  const char* env_level = std::getenv("BEEPING_LOG_LEVEL");
  std::string level_str = env_level ? env_level : "warn";

  if (level_str == "trace")
    logger->set_level(spdlog::level::trace);
  else if (level_str == "debug")
    logger->set_level(spdlog::level::debug);
  else if (level_str == "info")
    logger->set_level(spdlog::level::info);
  else if (level_str == "warn")
    logger->set_level(spdlog::level::warn);
  else if (level_str == "error")
    logger->set_level(spdlog::level::err);
  else
    logger->set_level(spdlog::level::warn);

  logger->flush_on(spdlog::level::debug);

  spdlog::set_default_logger(logger);
  g_initialized.store(true, std::memory_order_release);
  g_session_count.store(1);

  spdlog::info("BeepingCore logger initialized (sink={} level={})", sink_kind,
               level_str);
}

void shutdownBeepingLogger() {
  std::lock_guard<std::mutex> lk(g_init_mutex);
  int remaining = g_session_count.fetch_sub(1) - 1;
  if (remaining <= 0 && g_initialized.load(std::memory_order_relaxed)) {
    spdlog::info("BeepingCore logger shutdown");
    spdlog::shutdown();
    g_initialized.store(false, std::memory_order_release);
    g_session_count.store(0);
  }
}

}  // namespace BEEPING

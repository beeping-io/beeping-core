#include <BeepingDebug.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

#include <atomic>
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

static std::mutex g_init_mutex;
static std::atomic<bool> g_initialized{false};
static std::atomic<int> g_session_count{0};

void ensureBeepingLogger() {
  if (g_initialized.load(std::memory_order_acquire)) return;
  initBeepingLogger();
}

void initBeepingLogger() {
  std::lock_guard<std::mutex> lk(g_init_mutex);
  if (g_initialized.load(std::memory_order_relaxed)) {
    g_session_count.fetch_add(1);
    return;
  }

  // Create logs directory
  beeping_mkdir("logs");

  // Rotating file sink: 5MB max, 10 files
  auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      "logs/beeping.log", 5 * 1024 * 1024, 10);

  // JSON pattern
  file_sink->set_pattern(
      R"({"ts":"%Y-%m-%dT%H:%M:%S.%e","level":"%l","msg":"%v"})");

  auto logger = std::make_shared<spdlog::logger>("beeping", file_sink);

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

  spdlog::info("BeepingCore logger initialized (level={})", level_str);
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

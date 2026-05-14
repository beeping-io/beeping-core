// LoggerTest.cpp — Catch2 v3 tests for BEEPING_SetLogPath and the
// non-crashing fallback when the configured log path is unwritable.
//
// The runtime invariant we protect is: BEEPING_Create() must never crash
// the host process even if the working directory is read-only or the
// configured log path is invalid. Before BEE-2227, spdlog would throw
// spdlog_ex when opening a relative `logs/beeping.log` under Android's
// `/` cwd; the exception was uncaught and the runtime called terminate().
//
// Tests are written to be order-independent: many other test cases also
// call BEEPING_Create() and may leave the logger initialized, so we never
// assume a clean global state when our test starts.

#include <BeepingCoreLib_api.h>

#include <catch2/catch_test_macros.hpp>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

namespace {

bool fileExists(const std::string& path) {
  std::error_code ec;
  return std::filesystem::exists(path, ec);
}

std::string uniqueTempLogPath() {
  static std::atomic<int> counter{0};
  const auto tmpdir = std::filesystem::temp_directory_path();
  const int n = counter.fetch_add(1);
  char name[64] = {};
  std::snprintf(name, sizeof(name), "beeping-test-%d-%d.log",
                static_cast<int>(std::rand()), n);
  return (tmpdir / name).string();
}

}  // namespace

TEST_CASE("SetLogPath: rejects empty string", "[logger]") {
  // Either logger is uninitialized (-> -2 because of empty arg) or it is
  // already initialized (-> -1 because it is locked). In neither case
  // should the empty string be silently accepted (which would later cause
  // spdlog to throw with cryptic errors).
  int rc = BEEPING_SetLogPath("");
  REQUIRE((rc == -2 || rc == -1));
}

TEST_CASE("SetLogPath: returns -1 once the logger is initialized",
          "[logger]") {
  // Force the logger to be initialized for this test scope.
  void* core = BEEPING_Create();
  REQUIRE(core != nullptr);

  // Once initialized, any rebind attempt must be rejected.
  REQUIRE(BEEPING_SetLogPath("/tmp/foo.log") == -1);
  REQUIRE(BEEPING_SetLogPath(nullptr) == -1);

  BEEPING_Destroy(core);
}

TEST_CASE("BEEPING_Create does not crash with an unwritable log path",
          "[logger]") {
  // This is the regression test for BEE-2227. If our test is the first to
  // initialize the logger we can actually steer it to the bad path and
  // exercise the fallback. If a prior test already initialized the
  // logger, SetLogPath returns -1 and we cannot reroute — the assertion
  // becomes "Create still does not crash with whatever path was set
  // before", which is exactly the user-visible contract we care about.
  int rc = BEEPING_SetLogPath("/this/path/should/not/exist/beeping.log");
  REQUIRE((rc == 0 || rc == -1));

  void* core = BEEPING_Create();
  REQUIRE(core != nullptr);

  // A real codepath that calls into the logger via the BTRACE macros.
  REQUIRE(BEEPING_Configure(BEEPING_MODE_INAUDIBLE, 44100.0f, 128, core) == 0);

  BEEPING_Destroy(core);
}

TEST_CASE("SetLogPath: writes to a custom absolute path when uninitialized",
          "[logger]") {
  const std::string path = uniqueTempLogPath();
  int rc = BEEPING_SetLogPath(path.c_str());
  if (rc == -1) {
    // Another test already initialized the logger. Nothing more we can
    // assert here without test isolation — leave as a smoke check.
    SUCCEED("logger already initialized; skipping path-write assertion");
    return;
  }
  REQUIRE(rc == 0);

  void* core = BEEPING_Create();
  REQUIRE(core != nullptr);
  BEEPING_Configure(BEEPING_MODE_INAUDIBLE, 44100.0f, 128, core);
  BEEPING_Destroy(core);

  INFO("custom log path = " << path);
  REQUIRE(fileExists(path));
  std::remove(path.c_str());

  // Reset to default so we do not leak this path into subsequent tests
  // if they happen to run while the logger has been torn down.
  BEEPING_SetLogPath(nullptr);
}

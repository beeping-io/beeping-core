#include "Scheduler.h"

#include <algorithm>
#include <cmath>

namespace BEEPING {

int computeBeepCount(float duration, float startTime, float interval) {
  if (duration < kMinBeepWindow) return 0;
  if (startTime < 0.0f) return 0;
  if (interval <= 0.0f) return 0;
  if ((startTime + kMinBeepWindow) > duration) return 0;

  const float remaining = duration - startTime - kMinBeepWindow;
  const int extra = static_cast<int>(std::floor((remaining + 1e-6f) / interval));
  return 1 + std::max(extra, 0);
}

std::vector<double> computeBeepSchedule(float duration, float startTime, float interval) {
  std::vector<double> times;
  const int count = computeBeepCount(duration, startTime, interval);
  times.reserve(static_cast<size_t>(count));
  double t = static_cast<double>(startTime);
  while (static_cast<int>(times.size()) < count &&
         (t + kMinBeepWindow) <= static_cast<double>(duration) + 1e-6) {
    times.push_back(t);
    t += static_cast<double>(interval);
  }
  return times;
}

static constexpr char kBase32Digits[] = "0123456789abcdefghijklmnopqrstuv";

std::string toBase32(int value) {
  if (value <= 0) return "0";
  std::string result;
  while (value > 0) {
    result.insert(result.begin(), kBase32Digits[value % 32]);
    value /= 32;
  }
  return result;
}

int fromBase32(const char* s, int len) {
  if (!s || len <= 0) return -1;
  int result = 0;
  for (int i = 0; i < len; ++i) {
    const char c = s[i];
    int digit = -1;
    for (int j = 0; j < 32; ++j) {
      if (c == kBase32Digits[j]) {
        digit = j;
        break;
      }
    }
    if (digit < 0) return -1;
    result = result * 32 + digit;
  }
  return result;
}

bool parseScheduledPayload(const char* payload, int payloadSize,
                           const char** outCode, int* outCodeSize,
                           int* outTimestampSec) {
  if (!payload || payloadSize < 5) return false;
  const int tsStart = payloadSize - 4;
  const int ts = fromBase32(payload + tsStart, 4);
  if (ts < 0) return false;
  if (outCode) *outCode = payload;
  if (outCodeSize) *outCodeSize = tsStart;
  if (outTimestampSec) *outTimestampSec = ts;
  return true;
}

}  // namespace BEEPING

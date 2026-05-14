/**
 * @file Scheduler.h
 * @brief Internal C++ helpers backing the time-scheduled encoding C API.
 *
 * Public consumers should use the C functions declared in BeepingCoreLib_api.h
 * (BEEPING_ComputeBeepSchedule, BEEPING_EncodeWithSchedule). This header is
 * an implementation detail exposed only because all BeepingCore C++ headers
 * live under `include/` for build-tree convenience.
 *
 * @copyright Copyright (C) Beeping, LLC. Apache-2.0.
 */

#ifndef BEEPING_SCHEDULER_H
#define BEEPING_SCHEDULER_H

#include <string>
#include <vector>

namespace BEEPING {

/// Minimum duration of one beep window in seconds. The encoder emits
/// approximately this much audio per encoded payload at 44.1 kHz.
inline constexpr float kMinBeepWindow = 2.3f;

/// Compute how many beeps fit in `duration` seconds at `interval` cadence
/// starting at `startTime`. Returns 0 when no beep fits (duration shorter
/// than the minimum window, startTime offsets past `duration - 2.3`, or
/// non-positive interval).
int computeBeepCount(float duration, float startTime, float interval);

/// Compute the timestamps (seconds, monotonically increasing) at which each
/// beep should fire. Empty when no beep fits.
std::vector<double> computeBeepSchedule(float duration, float startTime, float interval);

/// Convert a non-negative integer to base-32 using the [0-9a-v] alphabet.
/// Used internally by BEEPING_EncodeWithSchedule to embed the rounded
/// timestamp of each scheduled beep into its payload, so a decoder can
/// recover the beep's position within the schedule.
std::string toBase32(int value);

/// Parse a base-32 string back to a non-negative integer. Returns -1 if
/// the string contains any character outside the [0-9a-v] alphabet.
int fromBase32(const char* s, int len);

/// Split a scheduler-formatted payload (`code` + 4-char zero-padded base-32
/// timestamp) into its parts.
///
/// @param payload         Pointer to decoded payload chars.
/// @param payloadSize     Length of payload in bytes.
/// @param[out] outCode    Pointer where the code prefix start will be
///                        written. Points into `payload` — NOT a copy.
/// @param[out] outCodeSize Length of the code prefix (payloadSize - 4).
/// @param[out] outTimestampSec Decoded timestamp in seconds.
/// @return true on success, false if payloadSize < 5 (need at least 1 code
///         char + 4 timestamp chars), or if the trailing 4 chars are not
///         valid base-32.
bool parseScheduledPayload(const char* payload, int payloadSize,
                           const char** outCode, int* outCodeSize,
                           int* outTimestampSec);

}  // namespace BEEPING

#endif  // BEEPING_SCHEDULER_H

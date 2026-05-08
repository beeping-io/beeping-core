/** BEEPING_MODE enum values mirrored from BeepingCoreLib_api.h. */
export const BEEPING_MODE_AUDIBLE: 2;
export const BEEPING_MODE_INAUDIBLE: 3;
export const BEEPING_MODE_ALL: 5;

/**
 * Decode a WAV buffer and return its Beeping payload, or `null` if no
 * beep was detected. The WAV must be PCM int16 or IEEE float32, mono
 * or stereo (stereo is downmixed to mono).
 *
 * @param wavBuffer Raw WAV file bytes.
 * @param mode      BEEPING_MODE_* selector (defaults to BEEPING_MODE_ALL).
 */
export function decode(
  wavBuffer: ArrayBuffer,
  mode?: 2 | 3 | 5,
): Promise<string | null>;

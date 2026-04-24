// High-level ES-module wrapper around the Emscripten-compiled BeepingCore.
//
// The low-level WASM module is emitted next to this file as
// `beeping-core.mjs` + `beeping-core.wasm`. This wrapper parses a WAV
// payload on the JS side (mono or stereo, PCM int16 or IEEE float32) and
// streams the samples into the C decoder in 1024-sample chunks — the
// same chunking the CLI uses, so the behaviour matches the native
// binaries.
//
// Usage:
//   import { decode } from "./beeping-core.js";
//   const payload = await decode(await file.arrayBuffer());

import createBeepingCore from "./beeping-core.mjs";

export const BEEPING_MODE_AUDIBLE   = 2;
export const BEEPING_MODE_INAUDIBLE = 3;
export const BEEPING_MODE_ALL       = 5;

const kBufferSize = 1024;

let _modulePromise = null;
function loadModule() {
  if (!_modulePromise) _modulePromise = createBeepingCore();
  return _modulePromise;
}

function parseWav(arrayBuffer) {
  const view = new DataView(arrayBuffer);
  const tag = (o) =>
    String.fromCharCode(view.getUint8(o), view.getUint8(o+1), view.getUint8(o+2), view.getUint8(o+3));
  if (tag(0) !== "RIFF" || tag(8) !== "WAVE") {
    throw new Error("Not a RIFF/WAVE file");
  }

  let fmt = null;
  let data = null;
  let offset = 12;
  while (offset + 8 <= view.byteLength) {
    const id = tag(offset);
    const size = view.getUint32(offset + 4, true);
    const body = offset + 8;
    if (id === "fmt ") {
      fmt = {
        format:        view.getUint16(body, true),
        channels:      view.getUint16(body + 2, true),
        sampleRate:    view.getUint32(body + 4, true),
        bitsPerSample: view.getUint16(body + 14, true),
      };
    } else if (id === "data") {
      data = { offset: body, size };
    }
    offset = body + size + (size % 2);
  }
  if (!fmt || !data) throw new Error("Missing fmt or data chunk");

  const bytesPerSample = fmt.bitsPerSample / 8;
  const frames = Math.floor(data.size / bytesPerSample / fmt.channels);
  const samples = new Float32Array(frames);

  if (fmt.format === 1 && fmt.bitsPerSample === 16) {
    let pos = data.offset;
    for (let i = 0; i < frames; i++) {
      let sum = 0;
      for (let c = 0; c < fmt.channels; c++) {
        sum += view.getInt16(pos, true) / 32768;
        pos += bytesPerSample;
      }
      samples[i] = sum / fmt.channels;
    }
  } else if (fmt.format === 3 && fmt.bitsPerSample === 32) {
    let pos = data.offset;
    for (let i = 0; i < frames; i++) {
      let sum = 0;
      for (let c = 0; c < fmt.channels; c++) {
        sum += view.getFloat32(pos, true);
        pos += bytesPerSample;
      }
      samples[i] = sum / fmt.channels;
    }
  } else {
    throw new Error(
      `Unsupported WAV format=${fmt.format} bitsPerSample=${fmt.bitsPerSample} ` +
      `(expected PCM int16 or IEEE float32)`);
  }

  return { sampleRate: fmt.sampleRate, samples };
}

/**
 * Decode a beeping WAV buffer and return its payload, or `null` if no
 * beep was detected.
 *
 * @param {ArrayBuffer} wavBuffer  Raw WAV file bytes.
 * @param {number} [mode=5]        BEEPING_MODE_* selector.
 * @returns {Promise<string|null>}
 */
export async function decode(wavBuffer, mode = BEEPING_MODE_ALL) {
  const M = await loadModule();
  const wav = parseWav(wavBuffer);

  const create    = M.cwrap("BEEPING_Create",            "number", []);
  const destroy   = M.cwrap("BEEPING_Destroy",            null,     ["number"]);
  const configure = M.cwrap("BEEPING_Configure",          "number", ["number", "number", "number", "number"]);
  const feed      = M.cwrap("BEEPING_DecodeAudioBuffer",  "number", ["number", "number", "number"]);
  const getData   = M.cwrap("BEEPING_GetDecodedData",     "number", ["number", "number"]);

  const handle = create();
  if (!handle) throw new Error("BEEPING_Create failed");

  if (configure(mode, wav.sampleRate, kBufferSize, handle) !== 0) {
    destroy(handle);
    throw new Error(`BEEPING_Configure failed (mode=${mode} sr=${wav.sampleRate})`);
  }

  const bufPtr  = M._malloc(kBufferSize * 4);
  const outPtr  = M._malloc(32);
  try {
    let decoded = false;
    for (let i = 0; i < wav.samples.length && !decoded; i += kBufferSize) {
      const chunk = Math.min(kBufferSize, wav.samples.length - i);
      M.HEAPF32.set(wav.samples.subarray(i, i + chunk), bufPtr / 4);
      if (feed(bufPtr, chunk, handle) === -3) decoded = true;
    }

    // Flush trailing silence — same pattern the CLI uses (decode_cmd.cpp).
    if (!decoded) {
      M.HEAPF32.fill(0, bufPtr / 4, bufPtr / 4 + kBufferSize);
      for (let f = 0; f < 200 && !decoded; f++) {
        if (feed(bufPtr, kBufferSize, handle) === -3) decoded = true;
      }
    }
    if (!decoded) return null;

    const len = getData(outPtr, handle);
    if (len <= 0) return null;
    return M.UTF8ToString(outPtr, len);
  } finally {
    M._free(bufPtr);
    M._free(outPtr);
    destroy(handle);
  }
}

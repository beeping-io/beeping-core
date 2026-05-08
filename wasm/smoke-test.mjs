// Node.js smoke test — used by the WASM release job to gate the build.
// Usage:
//   node smoke-test.mjs <wav-file> <expected-payload>

import { readFile } from "node:fs/promises";
import { decode } from "./beeping-core.js";

const [wavPath, expected] = process.argv.slice(2);
if (!wavPath || !expected) {
  console.error("usage: node smoke-test.mjs <wav> <expected>");
  process.exit(2);
}

const buf = await readFile(wavPath);
const ab = buf.buffer.slice(buf.byteOffset, buf.byteOffset + buf.byteLength);
const payload = await decode(ab);

if (payload === expected) {
  console.log(`✅ decoded '${payload}' (expected '${expected}')`);
  process.exit(0);
} else {
  console.error(`❌ got ${JSON.stringify(payload)}, expected '${expected}'`);
  process.exit(1);
}

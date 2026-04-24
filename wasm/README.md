# beeping-core — WASM decoder

Pre-built ES-module bundle that decodes Beeping WAVs entirely in the
browser (or Node.js). No server, no `<audio>` element, no network call
after the module loads.

## Files

| File | Purpose |
|---|---|
| `beeping-core.wasm` | Compiled BeepingCore C++20 library (Emscripten, `-O3`, `/MT`-equivalent). |
| `beeping-core.mjs`  | Emscripten ES-module glue emitted alongside the `.wasm`. |
| `beeping-core.js`   | Thin high-level wrapper: parses WAV (PCM int16 / float32, mono/stereo) and streams samples into the C API. |
| `beeping-core.d.ts` | TypeScript types for `beeping-core.js`. |
| `demo.html`         | Drag-and-drop browser demo — open it over HTTPS or via a local static server. |
| `smoke-test.mjs`    | Node.js smoke test (gating job in CI). |

## Usage

```js
import { decode } from "./beeping-core.js";

const file = document.querySelector("input[type=file]").files[0];
const payload = await decode(await file.arrayBuffer());
console.log(payload); // e.g. "h3l7m0000"
```

`decode()` resolves to the payload string, or `null` if no beep was
detected. Pass a mode override as the second argument:

```js
import { decode, BEEPING_MODE_AUDIBLE } from "./beeping-core.js";
await decode(buf, BEEPING_MODE_AUDIBLE);
```

## Serving

`beeping-core.wasm` must be served with MIME `application/wasm`. Most
static hosts (Firebase Hosting, GitHub Pages, Cloudflare Pages, S3
with the right content-type) handle this by default. If you bundle it
into a Vite/Webpack app, import the `.mjs` directly — the bundler will
wire the `.wasm` path.

CORS: the module is same-origin by default. If you load it from a
different origin, the host must send `Access-Control-Allow-Origin: *`
(or the right allow-list) on both the `.mjs` and `.wasm`.

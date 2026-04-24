// Emscripten entry point for the WASM build of BeepingCore.
//
// Emscripten requires a `main` to link a "binary" module. The C API in
// include/BeepingCoreLib_api.h is kept alive from JavaScript via
// `-sEXPORTED_FUNCTIONS=['_BEEPING_*','_malloc','_free']` in release.yml
// (mirrored in CMakeLists.txt). No new bindings are needed — the
// existing C ABI is the web ABI.
#include <BeepingCoreLib_api.h>

int main() { return 0; }

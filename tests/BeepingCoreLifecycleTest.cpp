#include <BeepingCoreLib_api.h>

#include <cassert>

int main() {
  void* core = BEEPING_Create();
  assert(core != nullptr);

  BEEPING_Destroy(core);
  return 0;
}

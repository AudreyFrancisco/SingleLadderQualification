#include "scope_control.h"

int main()
{
  scope_control scope;
  scope.debug_en = true;
  scope.open_auto();
  scope.get_errors();
  // scope.open("/dev/pts/9");
  scope.enable_ch(1);
  scope.set_vscale_ch(1, 100e-3);
  scope.set_timescale(1e-3);
  scope.set_trigger_ext();
  scope.single_capture();
  scope.wait_for_trigger();
  scope.start_quick_meas();
  scope.get_quick_meas();
  // scope.stop_quick_meas();
  scope.close();
  printf("End.\n");
  return 0;
}
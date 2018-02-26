#include "scope_control.h"

int main()
{
  scope_control scope;
  scope.debug_en = true;
  scope.open_auto();
  scope.get_errors();
  // scope.open("/dev/pts/9");
  for (int i = 1; i <= 4; i++) {
    scope.enable_ch(i);
    scope.set_vscale_ch(i, 100e-3);
  }
  scope.set_timescale(1e-3);
  scope.set_trigger_ext();
  scope.set_trigger_slope_pos(true);
  scope.set_ext_trigger_level(-2.0);
  scope.single_capture();
  scope.wait_for_trigger();
  scope.set_math_measure();
  scope.set_math_diff(1, 2);
  scope.get_meas();
  printf("CH1 Peak-to-peak : %.6f Amplitude : %.6f Risetime : %.6f Falltime : %.6f\n", scope.peak,
         scope.amp, scope.rtim, scope.ftim);
  scope.set_math_diff(3, 4);
  scope.get_meas();
  printf("CH2 Peak-to-peak : %.6f Amplitude : %.6f Risetime : %.6f Falltime : %.6f\n", scope.peak,
         scope.amp, scope.rtim, scope.ftim);
  // scope.start_quick_meas();
  // scope.get_quick_meas();
  // scope.stop_quick_meas();
  scope.close();
  printf("End.\n");
  return 0;
}
#include "scope_control.h"

int main()
{
  scope_control scope;
  scope.debug_en = true;
  if (!scope.open_auto()) {
    exit(1);
  }
  scope.get_errors();
  // scope.open("/dev/pts/9");
  for (int i = 1; i <= 4; i++) {
    scope.enable_ch(i);
    scope.set_vscale_ch(i, 200e-3);
    scope.set_dc_coupling_ch(i, true);
  }
  scope.set_timescale(5e-9);
  scope.set_trigger_ext();
  scope.set_trigger_slope_rising(false);
  scope.set_trigger_position(1.1e-6);
  scope.set_ext_trigger_level(-0.5);
  scope.set_reflevel_rtime_ftime(20, 80);
  scope.single_capture();
  scope.wait_for_trigger(1000);
  for (int i = 1; i <= 4; i++) {
    scope.en_measure_ch(i);
    scope.get_meas();
  }
  printf("CH1 Peak-to-peak : %.6f Amplitude : %.6f Risetime : %.6f Falltime : %.6f\n",
         scope.ch1.peak, scope.ch1.amp, scope.ch1.rtim, scope.ch1.ftim);
  printf("CH2 Peak-to-peak : %.6f Amplitude : %.6f Risetime : %.6f Falltime : %.6f\n",
         scope.ch2.peak, scope.ch2.amp, scope.ch2.rtim, scope.ch2.ftim);
  printf("CH3 Peak-to-peak : %.6f Amplitude : %.6f Risetime : %.6f Falltime : %.6f\n",
         scope.ch3.peak, scope.ch3.amp, scope.ch3.rtim, scope.ch3.ftim);
  printf("CH4 Peak-to-peak : %.6f Amplitude : %.6f Risetime : %.6f Falltime : %.6f\n",
         scope.ch4.peak, scope.ch4.amp, scope.ch4.rtim, scope.ch4.ftim);
  // scope.start_quick_meas();
  // scope.get_quick_meas();
  // scope.stop_quick_meas();
  scope.close();
  printf("End.\n");
  return 0;
}
#ifndef ALPIDECONFIG_H
#define ALPIDECONFIG_H

#include "TAlpide.h"

namespace AlpideConfig {
  void MaskRow                  ();
  void ApplyStandardDACSettings (TAlpide *chip, float backBias);
}



#endif

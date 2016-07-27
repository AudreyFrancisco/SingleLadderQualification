#ifndef ALPIDECONFIG_H
#define ALPIDECONFIG_H

#include "TAlpide.h"

namespace AlpideConfig {
  void Init (TAlpide *chip);
  void ClearPixSelectBits (TAlpide *chip, bool clearPulseGating);
  void WritePixConfReg    (TAlpide *chip, Alpide::TPixReg reg, bool data);
  void WritePixRegAll     (TAlpide *chip, Alpide::TPixReg reg, bool data);
  void WritePixRegRow     (TAlpide *chip, Alpide::TPixReg reg, bool data, int row);
  void WritePixRegSingle  (TAlpide *chip, Alpide::TPixReg reg, bool data, int row, int col);
  void ApplyStandardDACSettings (TAlpide *chip, float backBias);
}



#endif

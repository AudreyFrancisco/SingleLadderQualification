#include "AlpideConfig.h"
#include <iostream>


void AlpideConfig::MaskRow () {
}


// Alpide 3 settings, to be confirmed
void AlpideConfig::ApplyStandardDACSettings (TAlpide *chip, float backBias) {
  if (backBias == 0) {
    chip->WriteRegister(Alpide::REG_VCASN,    60);
    chip->WriteRegister(Alpide::REG_VCASN2,   62);
    chip->WriteRegister(Alpide::REG_VRESETD, 147);
    chip->WriteRegister(Alpide::REG_IDB,      29);
  }
  else if (backBias == 3) {
    chip->WriteRegister(Alpide::REG_VCASN,   105);
    chip->WriteRegister(Alpide::REG_VCASN2,  117);
    chip->WriteRegister(Alpide::REG_VCLIP,    60);
    chip->WriteRegister(Alpide::REG_VRESETD, 147);
    chip->WriteRegister(Alpide::REG_IDB,      29);
  }
  else if (backBias == 6) {
    chip->WriteRegister(Alpide::REG_VCASN,   135);
    chip->WriteRegister(Alpide::REG_VCASN2,  147);
    chip->WriteRegister(Alpide::REG_VCLIP,   100);
    chip->WriteRegister(Alpide::REG_VRESETD, 170);
    chip->WriteRegister(Alpide::REG_IDB,      29);
  }
  else {
    std::cout << "Settings not defined for back bias " << backBias << " V. Please set manually." << std::endl;
  }

}

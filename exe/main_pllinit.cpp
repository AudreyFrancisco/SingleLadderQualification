#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "SetupHelpers.h"
#include "TAlpide.h"
#include "TConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "USBHelpers.h"
#include <unistd.h>

TBoardType                   fBoardType;
std::vector<TReadoutBoard *> fBoards;
std::vector<TAlpide *>       fChips;
TConfig *                    fConfig;
std::vector<THic *>          fHICs;
int                          main(int argc, char **argv)
{

  decodeCommandParameters(argc, argv);

  initSetup(fConfig, &fBoards, &fBoardType, &fChips, "", &fHICs, nullptr, false, true);
  for (auto rBoard : fBoards) {
    rBoard->SendOpCode(Alpide::OPCODE_GRST);
    rBoard->SendOpCode(Alpide::OPCODE_PRST);
    rBoard->SendOpCode(Alpide::OPCODE_RORST);
  }

  const int pos   = 84;
  const int n_rep = 1000;
  int       n_err = 0;
  auto      last  = std::chrono::high_resolution_clock::now();

  AlpideConfig::BaseConfig(fChips[pos]);
  for (int i = 0; i < n_rep; ++i) {
    AlpideConfig::BaseConfigPLL(fChips[pos]);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    uint16_t val[2];
    fChips[pos]->ReadRegister(0x14, val[0]);
    fChips[pos]->ReadRegister(0x16, val[1]);
    if ((val[0] & 0x1000) == 0) {
      auto                                      temp = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> dist = temp - last;
      printf("PLL registers: 0x%04x,0x%04x; time since last error: %g\n", val[0], val[1],
             dist.count());
      last = temp;
      ++n_err;
    }
  }

  printf("failures / repetitions: %i / %i\n", n_err, n_rep);

  return 0;
}

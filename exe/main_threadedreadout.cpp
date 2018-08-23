#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "SetupHelpers.h"
#include "TAlpide.h"
#include "TReadoutBoardRU.h"
#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <thread>
#include <unistd.h>

TBoardType                   fBoardType;
std::vector<TReadoutBoard *> fBoards;
std::vector<TAlpide *>       fChips;
TConfig *                    fConfig;

int configureFromu(TAlpide *chip)
{

  uint16_t data = (1 << 4) | (1 << 6);
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,
                      data); // fromu config 1: digital pulsing (put to 0x20 for analogue)
  chip->WriteRegister(
      Alpide::REG_FROMU_CONFIG2,
      chip->GetConfig()->GetParamValue("STROBEDURATION")); // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1,
                      chip->GetConfig()->GetParamValue("STROBEDELAYCHIP")); // fromu pulsing 1:
                                                                            // delay pulse - strobe
                                                                            // (not used here, since
                                                                            // using external
                                                                            // strobe)
  chip->WriteRegister(
      Alpide::REG_FROMU_PULSING2,
      chip->GetConfig()->GetParamValue("PULSEDURATION")); // fromu pulsing 2: pulse length
  return 0;
}

void InitChips(int numUnmaskedPixels)
{

  // This will mask all but the number of pixels you select between 0 and 1024 (a full column, just
  // for quick testing purposes)

  for (unsigned int i = 0; i < fChips.size(); i++) {
    auto ch     = fChips.at(i);
    auto chipId = fChips.at(i)->GetConfig()->GetChipId();

    std::cout << "Configure chip " << (int)chipId << " for readout\n";

    AlpideConfig::BaseConfig(ch);
    AlpideConfig::Init(ch);

    configureFromu(ch);
    AlpideConfig::ConfigureCMU(ch);

    usleep(1000);


    if (numUnmaskedPixels == 0)
      AlpideConfig::BaseConfigMask(ch);
    else {

      for (int i = 0; i < numUnmaskedPixels; i++) {
        AlpideConfig::WritePixRegSingle(ch, Alpide::PIXREG_MASK, false, 230, i);
        AlpideConfig::WritePixRegSingle(ch, Alpide::PIXREG_SELECT, true, 230, i);
      }
    }
  }
}

int main(int argc, char **argv)
{

  std::cout << "Decode Command parameters\n";
  decodeCommandParameters(argc, argv);

  std::cout << "Create BoardConfig+Board\n";
  initSetup(fConfig, &fBoards, &fBoardType, &fChips);

  InitChips(1000);
  TReadoutBoardRUv1 *theBoard = dynamic_cast<TReadoutBoardRUv1 *>(fBoards.at(0));
  theBoard->Initialize(1200);


  theBoard->StartRun();
  theBoard->SetTriggerConfig(true, true, 1000, 1000);

  int                  nBytes;
  std::vector<uint8_t> buffer(1 * 1024 * 1024);
  uint16_t             lane_timeout;
  theBoard->SendStartOfTriggered();
  for (int i = 0; i < 1000; i++) {
    theBoard->ResetAllCounters();
    if (i % 1 == 0) theBoard->Trigger(1);
    usleep(1000);
    theBoard->LatchAllCounters();
    lane_timeout = theBoard->datapathmon->Read(17 * 8 + 12 + 2);
    if (lane_timeout != 0)
      std::cout << "WARNING: LANE PACKAGER LANE TIMEOUT: " << (lane_timeout & 0xff)
                << " AND GBT PACKER LANE TIMEOUT: " << ((lane_timeout >> 8) & 0xff) << std::endl;

    std::cout << theBoard->ReadEventData(nBytes, buffer.data()) << " BYTES READ\n";
  }
  theBoard->SendEndOfTriggered();
  theBoard->CleanUp();
}

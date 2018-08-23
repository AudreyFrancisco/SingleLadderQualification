#include "AlpideConfig.h"
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


void InitChips(int numUnmaskedPixels)
{

  // This will mask all but the number of pixels you select between 0 and 1024 (a full column, just
  // for quick testing purposes)

  for (unsigned int i = 0; i < fChips.size(); i++) {
    auto ch     = fChips.at(i);
    auto chipId = fChips.at(i)->GetConfig()->GetChipId();

    std::cout << "Configure chip " << (int)chipId << " for readout\n";

    AlpideConfig::Init(ch);
    AlpideConfig::BaseConfig(ch);
    if (numUnmaskedPixels == 0)
      AlpideConfig::BaseConfigMask(ch);
    else {
      // AlpideConfig::ConfigureMaskStage(ch, 32, 0);
      for (int i = 0; i < numUnmaskedPixels; i++) {
        AlpideConfig::WritePixRegSingle(ch, Alpide::PIXREG_MASK, false, 0, i);
        AlpideConfig::WritePixRegSingle(ch, Alpide::PIXREG_SELECT, true, 0, i);
      }
    }
  }
}

void CleanUp()
{
  for (unsigned int i = 0; i < fBoards.size(); i++) {
    TBoardType boardType = fBoards.at(i)->GetConfig()->GetBoardType();
    if (boardType == boardRU) {
      TReadoutBoardRU *ABoard = dynamic_cast<TReadoutBoardRU *>(fBoards.at(i));
      ABoard->CleanUp();
    }
    if (boardType == boardRUv1) {
      TReadoutBoardRUv1 *theBoard = dynamic_cast<TReadoutBoardRUv1 *>(fBoards.at(i));
      theBoard->CleanUp();
    }
  }
}

void PortToFile()
{

  for (unsigned int i = 0; i < fBoards.size(); i++) {
    TBoardType boardType = fBoards.at(i)->GetConfig()->GetBoardType();
    if ((boardType == boardRU) || (boardType == boardRUv1)) {
      TReadoutBoardRUv1 *       theBoard = dynamic_cast<TReadoutBoardRUv1 *>(fBoards.at(i));
      static UsbDev::DataBuffer buffer;
      ofstream                  textout("test.txt", ios::out);
      while (theBoard->readFromPort(TReadoutBoardRUv1::EP_DATA0_IN,
                                    TReadoutBoardRUv1::EVENT_DATA_READ_CHUNK, buffer) != 0) {
      }
      textout.close();
    }
  }
}

void Trigger(int numTriggers)
{
  for (unsigned int i = 0; i < fBoards.size(); i++) {
    fBoards.at(i)->Trigger(numTriggers);
  }
}

int main(int argc, char **argv)
{

  std::cout << "Decode Command parameters\n";
  decodeCommandParameters(argc, argv);

  std::cout << "Create BoardConfig+Board\n";
  initSetup(fConfig, &fBoards, &fBoardType, &fChips);

  int numPixels   = 0;
  int numTriggers = 10;

  InitChips(numPixels); // argument is number of pixels to leave unmasked, "stress" readout

  // prepare for trigger/readout
  for (unsigned int i = 0; i < fBoards.size(); i++) {
    fBoards.at(i)->StartRun();
    fBoards.at(i)->SetTriggerConfig(false, true, 100, 100);
  }

  std::thread first(Trigger, numTriggers);
  usleep(10000);
  std::thread second(PortToFile);


  first.join();
  second.join();


  CleanUp();

  std::cout << "TEST COMPLETE! \n";

  return 0;
}

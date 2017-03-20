/* ---------------
 * Example of MOSAIC use
 *
 ----------------- */

#include <iostream>
#include <unistd.h>
#include "TReadoutBoardRU.h"
#include "TAlpide.h"
#include <exception>
#include "SetupHelpers.h"
#include "AlpideConfig.h"

int main(int argc, char **argv) {

    using namespace std;

    std::cout << "Decode Command parameters\n";
  decodeCommandParameters(argc, argv);

  std::cout << "Create BoardConfig+Board\n";
  TBoardConfigRU ru_config;
  TReadoutBoardRU ru_board(&ru_config);

  TReadoutBoardRU *theBoard = &ru_board;

  std::vector<int> chipIDs;
  std::vector<TAlpide *> fChips;

  for (int i = 0; i < 8; i++)
    chipIDs.push_back(i);

    std::cout << "Create config\n";
  TConfig *config = new TConfig(1, chipIDs);

  std::cout << "Create chips\n";
  for (int i = 0; i < config->GetNChips(); i++) {
    fChips.push_back(new TAlpide(config->GetChipConfigById(chipIDs.at(i))));
    fChips.at(i)->SetReadoutBoard(theBoard);
  }

  // Initialize readout unit
  theBoard->Initialize();
  uint16_t Value;

  theBoard->checkGitHash();

  // Access board register to test read/write
  std::cout << "Access R/W register on RU\n";
  theBoard->transceiver_array[0]->Write(0,0x42,true);
  uint16_t result = theBoard->transceiver_array[0]->Read(0,true);
  std::cout << "Transceiver 0, Address 0, written: 0x42, received: 0x" << std::hex << result  << "\n";


  std::cout << "Reset Chips (GRST)\n";
  theBoard->SendOpCode(Alpide::OPCODE_GRST);
  std::cout << "Register Tests" << "\n";
  for (int i = 0; i < fChips.size(); i++) {
    fChips.at(i)->WriteRegister(0x60d, 10);
    try {
      fChips.at(i)->ReadRegister(0x60d, Value);
      std::cout << "Chip ID " << chipIDs.at(i) << ", Value = 0x" << std::hex
                << (int)Value << std::dec << std::endl;
    }
    catch (exception &e) {
      std::cout << "Chip ID " << chipIDs.at(i)
                << ", not answering, exception: " << e.what() << std::endl;
    }
  }
  /*
  // Setup chips and transceivers for readout
  for (int i = 0; i < fChips.size(); i++) {
      auto ch = fChips.at(i);
      auto chipId = chipIDs.at(i);
      AlpideConfig::Init(ch);
      AlpideConfig::ConfigureCMU(ch, config->GetChipConfigById(chipId));
      AlpideConfig::BaseConfigPLL(ch);
  }

  for(int i = 0; i < chipIDs.size(); ++i) {
      auto tr = theBoard->transceiver_array[i]; // TODO: Mapping between transceiver and chipid
    tr->Initialize(TBoardConfigRU::ReadoutSpeed::RO_1200,0);
    tr->ActivateReadout();
    if(tr->IsAligned()) {
        std::cout << "Transceiver " << i << "is aligned \n";
    } else {
        std::cout << "Transceiver " << i << "is NOT aligned \n";
    }
    tr->ResetCounters();
  }

  // Start triggering
  theBoard->Trigger(10);

  // check counters
  for(int i = 0; i < chipIDs.size(); ++i) {
      auto tr = theBoard->transceiver_array[i]; // TODO: Mapping between transceiver and chipid
      auto counters = tr->ReadCounters();
      std::cout << "Transceiver " << i << ", Events: " << counters["Events NrEvents"] << "\n";
  }
  */
  return 0;
}

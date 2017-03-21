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

#include <fstream>

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

  // Setup chips and transceivers for readout
  for (int i = 0; i < fChips.size(); i++) {
      auto ch = fChips.at(i);
      auto chipId = chipIDs.at(i);
      AlpideConfig::Init(ch);
      AlpideConfig::BaseConfig(ch);

      AlpideConfig::WritePixRegAll(ch, Alpide::PIXREG_MASK, true);
  }

  for(int i = 0; i < chipIDs.size(); ++i) {
      auto tr = theBoard->transceiver_array[i]; // TODO: Mapping between transceiver and chipid
    tr->Initialize(TBoardConfigRU::ReadoutSpeed::RO_1200,0);
    bool alignedBefore = tr->IsAligned();
    tr->ActivateReadout();
    if(tr->IsAligned()) {
        std::cout << "Transceiver " << i << " is aligned (before: " << alignedBefore << " )\n";
    } else {
        std::cout << "Transceiver " << i << " is NOT aligned \n";
    }
    tr->ResetCounters();
  }

  // Start triggering
  theBoard->SetTriggerConfig(false, true, 10000, 0);
  theBoard->Trigger(10);

  // check counters
  for(int i = 0; i < chipIDs.size(); ++i) {
      auto tr = theBoard->transceiver_array[i]; // TODO: Mapping between transceiver and chipid
      auto counters = tr->ReadCounters();
      std::cout << "Transceiver " << i << ", Events: " << counters["Events NrEvents"] << "\n";

      //for(auto cnt : counters) {
      //    std::cout << cnt.first << ": " << cnt.second << "\n";
      //}
  }

  ///////////
  // READOUT
  //////////

  // Cycle through readout 8 Chips -> 0-3 on DP2/DP3
  for(int i = 0; i < 4; ++i) {
      theBoard->setDataportSource(i, i);
      theBoard->dctrl->Wait(1000); // wait 1000 cycles approx.
  }

  std::vector<uint8_t> buffer(1*1024*1024);
  for(int i = 0; i < 10;++i) {
      int bytesRead = 0;
      theBoard->ReadEventData(bytesRead, buffer.data());
      std::cout << "Pass " << i << ", Bytes Read: " << bytesRead;
      std::string filename = std::string("event_") + std::to_string(i) + ".dat";
      std::ofstream of(filename, ios::binary);
      of.write((char*) buffer.data(),bytesRead);
  }

  return 0;
}

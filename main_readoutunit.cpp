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

int main(int argc, char **argv) {

  decodeCommandParameters(argc, argv);

  TBoardConfigRU ru_config;
  TReadoutBoardRU ru_board(&ru_config);

  TReadoutBoardRU *theBoard = &ru_board;

  std::vector<int> chipIDs;
  std::vector<TAlpide *> fChips;

  for (int i = 0; i < 8; i++)
    chipIDs.push_back(i);

  TConfig *config = new TConfig(1, chipIDs);

  for (int i = 0; i < config->GetNChips(); i++) {
    fChips.push_back(new TAlpide(config->GetChipConfigById(chipIDs.at(i))));
    fChips.at(i)->SetReadoutBoard(theBoard);
  }

  uint16_t Value;

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

  return 0;
}

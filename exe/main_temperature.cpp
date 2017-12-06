// Template to prepare standard test routines
// ==========================================
//
// After successful call to initSetup() the elements of the setup are accessible in the two vectors
//   - fBoards: vector of readout boards (setups implemented here have only 1 readout board, i.e. fBoards.at(0)
//   - fChips:  vector of chips, depending on setup type 1, 9 or 14 elements
//
// In order to have a generic scan, which works for single chips as well as for staves and modules,
// all chip accesses should be done with a loop over all elements of the chip vector.
// (see e.g. the configureChip loop in main)
// Board accesses are to be done via fBoards.at(0);
// For an example how to access board-specific functions see the power off at the end of main.
//
// The functions that should be modified for the specific test are configureChip() and main()


#include <unistd.h>
#include <string.h>
#include "TAlpide.h"
#include "AlpideConfig.h"
#include "TReadoutBoard.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "USBHelpers.h"
#include "TConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "SetupHelpers.h"

TConfig* fConfig;
std::vector <TReadoutBoard *> fBoards;
TBoardType fBoardType;
std::vector <TAlpide *> fChips;

int fEnabled = 0;  // variable to count number of enabled chips; leave at 0

void readTemp() {
  // Allocate the memory for host the results
  uint16_t *theResult = (uint16_t *)malloc(sizeof(uint16_t) * (fChips.size()+1) ); //
  if( theResult == NULL ) {
    std::cerr << "Test_temperature : Error to allocate memory" << std::endl;
    return;
  }
  float theValue;
  uint16_t theChipId;

  std::cout <<  "\tChipId\tBias\tTemp."  << std::endl;
  // Set all chips for Temperature Measurement
  for (unsigned int i = 0; i < fChips.size(); i ++) {
    if (! fChips.at(i)->GetConfig()->IsEnabled()) continue;
    theChipId = fChips.at(i)->GetConfig()->GetChipId();
    theValue = fChips.at(i)->ReadTemperature();
    std::cout << i << ")\t" << theChipId << "\t" << fChips.at(i)->GetADCOffset() << "\t" << theValue << "\t" << fChips.at(i)->ReadAnalogueVoltage() << std::endl;

  }
  // Deallocate memory
  free(theResult);

  std::cout << std::endl << "Test_temperature : Test finished !" << std::endl;
  return;
}

char *makeTimeStamp(char *ABuffer)
{
  time_t       t = time(0);   // get time now
  struct tm *now = localtime( & t );
  sprintf(ABuffer, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
  return(ABuffer);
}

int main(int argc, char** argv) {

  decodeCommandParameters(argc, argv);

  initSetup(fConfig,  &fBoards,  &fBoardType, &fChips);

  char TimeStamp[20];
  if (fBoards.size()) {
    for (const auto& rBoard : fBoards) {
      rBoard->SendOpCode (Alpide::OPCODE_GRST);
      rBoard->SendOpCode (Alpide::OPCODE_PRST);
    }

    for (const auto& rChip : fChips) {
      if (! rChip->GetConfig()->IsEnabled()) continue;
      ++fEnabled;
    }

    for (const auto& rBoard : fBoards) {
      rBoard->SendOpCode (Alpide::OPCODE_RORST);
    }

    std::cout << std::endl << std::endl;

    std::cout << "Found " << fEnabled << " enabled chips." << std::endl;

    makeTimeStamp(TimeStamp);
    std::cout << "Temperature test : " << TimeStamp << std::endl;
    readTemp();
  }
  return 0;
}

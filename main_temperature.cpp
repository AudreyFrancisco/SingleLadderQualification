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


int fEnabled = 0;  // variable to count number of enabled chips; leave at 0

void readTemp() {

  TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC*> (fBoards.at(0));

  // Allocate the memory for host the results
  uint16_t *theResult = (uint16_t *)malloc(sizeof(uint16_t) * (fChips.size()+1) ); //
  if( theResult == NULL ) {
	  std::cerr << "Test_temperature : Error to allocate memory" << std::endl;
	  return;
  }

  uint16_t MonitoringDAC = 0x00 | (0x00 << 4) | (0x0 << 7) | (0x0 << 8);


  uint16_t Mode = 0x0; // 0:Manual 1:Calibrate 2:Auto 3:SupoerManual
  uint16_t SelectInput = 0x08 << 2; // 0:AVSS 1:DVSS 2:AVDD 3:DVDD 4:VBGthVolScal 5:DACMONV 6:DACMONI 7:Bandgap 8:Temperature
  uint16_t ComparatorCurrent = 0x02 << 6; // 0:180uA 1:190uA 2:296uA 3:410uA
  uint16_t DiscriminatorSign = 0x00 << 8;
  uint16_t RampSpeed = 0x01 << 9; // 0:500ms 1:1us 2:2us 3:4us
  uint16_t HalfSLBTrim = 0x00 << 11;

  uint16_t TemperatureSelect = Mode | SelectInput | ComparatorCurrent | DiscriminatorSign | RampSpeed | HalfSLBTrim | DiscriminatorSign;

  // Set all chips for Temperature Measurement
  for (int i = 0; i < fChips.size(); i ++) {
	  if (! fChips.at(i)->GetConfig()->IsEnabled()) continue;
	  fChips.at(i)->WriteRegister( Alpide::REG_ANALOGMON, MonitoringDAC);
	  fChips.at(i)->WriteRegister( Alpide::REG_ADC_CONTROL, TemperatureSelect);
  }

  // Send the ADC Measurement command to all chips
  fBoards.at(0)->SendOpCode (Alpide::OPCODE_ADCMEASURE);

  usleep(6000); // Wait for the measurement > of 5 milli sec

  // Read all ADC registers
  for (int i = 0; i < fChips.size(); i ++) {
	  if (! fChips.at(i)->GetConfig()->IsEnabled()) continue;
	  fChips.at(i)->ReadRegister( Alpide::REG_ADC_AVSS, theResult[i]);
  }
 
  // Calculate and dump the temperature values
  float theValue;
  uint16_t theChipId;
  for (int i = 0; i < fChips.size(); i ++) {
  	  if (! fChips.at(i)->GetConfig()->IsEnabled()) continue;
  	  theChipId = fChips.at(i)->GetConfig()->GetChipId() & 0xf;
  	  theValue =  ( ((float)theResult[i]) * 0.1281) + 6.8; // first approximation
  	  std::cout << i << ")\t" << theChipId << "\t" << theValue << " " << std::endl;
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

int main() {

	initSetup();
	char TimeStamp[20];

	TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));
  
	if (fBoards.size() == 1) {
		fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
		fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

		for (int i = 0; i < fChips.size(); i ++) {
			if (!fChips.at(i)->GetConfig()->IsEnabled()) continue;
			fEnabled ++;
//			configureChip (fChips.at(i));
		}

		std::cout << "Found " << fEnabled << " enabled chips." << std::endl;
		fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);

		// put your test here...
		if (fBoards.at(0)->GetConfig()->GetBoardType() == boardMOSAIC) {
		}
		else if (fBoards.at(0)->GetConfig()->GetBoardType() == boardDAQ) {
		}

		makeTimeStamp(TimeStamp);
		std::cout << "Temperature test : " << TimeStamp << std::endl;

		readTemp();

	}
	return 0;
}

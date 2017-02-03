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
// Mode =  0:Manual 1:Calibrate 2:Auto 3:SupoerManual
// SelectInput =   0:AVSS 1:DVSS 2:AVDD 3:DVDD 4:VBGthVolScal 5:DACMONV 6:DACMONI 7:Bandgap 8:Temperature
// ComparatorCurrent =  0:180uA 1:190uA 2:296uA 3:410uA
// DiscriminatorSign = 0 / 1;
// RampSpeed = 0:500ms 1:1us 2:2us 3:4us
// HalfSLBTrim = 0 / 1;
uint16_t setTheADCCtrlRegister(int ChipIndex, uint16_t Mode, uint16_t SelectInput, uint16_t ComparatorCurrent,
		uint16_t DiscriminatorSign, uint16_t RampSpeed, uint16_t HalfSLBTrim)
{
	uint16_t Data;
	Data = Mode | (SelectInput<<2) | (ComparatorCurrent<<6) | (DiscriminatorSign<<8) | (RampSpeed<<9) | (HalfSLBTrim<<11);
	fChips.at(ChipIndex)->WriteRegister( Alpide::REG_ADC_CONTROL, Data);
	return(Data);
}

// Iref =  0:0.25ua 1:0.75uA 2:1.00uA 3:1.25uA
uint16_t setTheDacMonitor(int ChipIndex, uint16_t VoltageDAC, uint16_t CurrentDAC, bool CurrentOverwrite, bool VoltageOverwrite, uint16_t Iref)
{
	uint16_t Data;
	Data = VoltageDAC | (CurrentDAC<<4) | (CurrentOverwrite ? 1<<7 : 0) | (VoltageOverwrite ? 1<<8 : 0) | (Iref<<9);
	fChips.at(ChipIndex)->WriteRegister( Alpide::REG_ANALOGMON, Data);
	return(Data);
}



int CalibrateADC(int ChipIndex, bool &bDiscrSign, bool &bHalfBit)
{
	uint16_t theVal2,theVal1;
	bool isAVoltDAC, isACurrDAC, isATemperature, isAVoltageBuffered;
	int theSelInput;
	uint16_t Bias;

	// Calibration Phase 1
	setTheADCCtrlRegister(ChipIndex, 1, 0, 2, false, 1, false);
	fBoards.at(0)->SendOpCode ( Alpide::OPCODE_ADCMEASURE, fChips.at(ChipIndex)->GetConfig()->GetChipId());
	usleep(4000); // > of 5 milli sec
	fChips.at(ChipIndex)->ReadRegister( Alpide::REG_ADC_CALIB, theVal1);
	setTheADCCtrlRegister(ChipIndex, 1, 0, 2, true, 1, false);
	fBoards.at(0)->SendOpCode ( Alpide::OPCODE_ADCMEASURE, fChips.at(ChipIndex)->GetConfig()->GetChipId());
	usleep(4000); // > of 5 milli sec
	fChips.at(ChipIndex)->ReadRegister( Alpide::REG_ADC_CALIB, theVal2);
	bDiscrSign =  (theVal1 > theVal2) ? false : true;

	// Calibration Phase 2
	setTheADCCtrlRegister(ChipIndex, 1, 7, 2, bDiscrSign, 1, false);
	fBoards.at(0)->SendOpCode ( Alpide::OPCODE_ADCMEASURE, fChips.at(ChipIndex)->GetConfig()->GetChipId());
	usleep(4000); // > of 5 milli sec
	fChips.at(ChipIndex)->ReadRegister( Alpide::REG_ADC_CALIB, theVal1);
	setTheADCCtrlRegister(ChipIndex, 1, 7, 2, bDiscrSign, 1, true);
	fBoards.at(0)->SendOpCode ( Alpide::OPCODE_ADCMEASURE, fChips.at(ChipIndex)->GetConfig()->GetChipId());
	usleep(4000); // > of 5 milli sec
	fChips.at(ChipIndex)->ReadRegister( Alpide::REG_ADC_CALIB, theVal2);
	bHalfBit =  (theVal1 > theVal2) ? false : true;

	// Detect the Bias
	setTheADCCtrlRegister(ChipIndex, 1, 0, 2, bDiscrSign, 1, bHalfBit);
	fBoards.at(0)->SendOpCode ( Alpide::OPCODE_ADCMEASURE, fChips.at(ChipIndex)->GetConfig()->GetChipId());
	usleep(4000); // > of 5 milli sec
	fChips.at(ChipIndex)->ReadRegister( Alpide::REG_ADC_CALIB, Bias);

	return(Bias);
}


void readTemp() {

  TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC*> (fBoards.at(0));

  // Allocate the memory for host the results
  uint16_t *theResult = (uint16_t *)malloc(sizeof(uint16_t) * (fChips.size()+1) ); //
  if( theResult == NULL ) {
	  std::cerr << "Test_temperature : Error to allocate memory" << std::endl;
	  return;
  }
  uint16_t Bias;
  bool Sign, Half;
  float theValue;
  uint16_t theChipId;

  std::cout <<  "\tChipId\tBias\tRaw\tTemp."  << std::endl;
  // Set all chips for Temperature Measurement
  for (int i = 0; i < fChips.size(); i ++) {
	  if (! fChips.at(i)->GetConfig()->IsEnabled()) continue;

	  Bias = CalibrateADC(i, Sign, Half);
	  theChipId = fChips.at(i)->GetConfig()->GetChipId();

	  setTheDacMonitor(i, 0, 0, false, false, 1);
	  setTheADCCtrlRegister(i, 0, 8, 2, Sign, 1, Half);

	  fBoards.at(0)->SendOpCode ( Alpide::OPCODE_ADCMEASURE,  theChipId);
	  usleep(5000); // Wait for the measurement > of 5 milli sec
	  fChips.at(i)->ReadRegister( Alpide::REG_ADC_AVSS, theResult[i]);
	  theResult[i] -= Bias;
	  theValue =  ( ((float)theResult[i]) * 0.1281) + 6.8; // first approximation
 	  std::cout << i << ")\t" << theChipId << "\t" << Bias << "\t" << theResult[i] << "\t" << theValue << " " << std::endl;

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
    initSetup(fConfig,  &fBoards,  &fBoardType, &fChips);

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

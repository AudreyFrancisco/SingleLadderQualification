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
#include <iostream>
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


// !!! NOTE: Scan parameters are now set via Config file

// This routine reads ThresholdSummary files for a chip (produced by tuning scan +
// routines), sets VCASN appropriately, and then begins a threshold scan.
// This program MUST be given a ThresholdSummary...dat filename as a command line argument.
// The name must have the format ThresholdSummary_XXXXXX_XXXXXX_Chip#..., where # is the chip number; stuff past the chip name doesn't matter.

TBoardType fBoardType;
std::vector <TReadoutBoard *> fBoards;
std::vector <TAlpide *>       fChips;
TConfig *fConfig;

int myNTriggers;
int myMaskStages;
int myPixPerRegion;

int myChargeStart;
int myChargeStop;
int myChargeStep;  // currently unused

int fEnabled = 0;  // variable to count number of enabled chips; leave at 0

int**** HitData;
int ChargePoints[100];
int ievt = 0;

std::string summaryName; //for reading ThresholdSummary files in fillVcasn


void InitHitData() {
  HitData = new int***[16];
  for (int i=0; i<16; ++i) {
    HitData[i] = new int**[100];
    for (int j=0; j<100; ++j) {
      HitData[i][j] = new int*[512];
      for (int k=0; k<1024; ++k) {
        HitData[i][j][k] = new int[1024];
      }
    }
  }
}


void DeleteHitData() {
  if (HitData) {
    for (int i=0; i<16; ++i) {
      if (HitData[i]) {
        for (int j=0; j<100; ++j) {
          if (HitData[i][j]) {
            for (int k=0; k<512; ++k) {
              delete[] HitData[i][j][k];
            }
            delete[] HitData[i][j];
          }
        }
        delete[] HitData[i];
      }
    }
    delete[] HitData;
  }
}


void InitScanParameters() {
  myMaskStages   = fConfig->GetScanConfig()->GetParamValue("NMASKSTAGES");
  myPixPerRegion = fConfig->GetScanConfig()->GetParamValue("PIXPERREGION");
  myNTriggers    = fConfig->GetScanConfig()->GetParamValue("NINJ");
  myChargeStart  = fConfig->GetScanConfig()->GetParamValue("CHARGESTART");
  myChargeStop   = fConfig->GetScanConfig()->GetParamValue("CHARGESTOP");
  myChargeStep   = fConfig->GetScanConfig()->GetParamValue("CHARGESTEP");
}


void ClearHitData() {
  for (int ichip = 0; ichip < 16; ichip ++) {
    for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
      ChargePoints[icharge-myChargeStart] = icharge;
      for (int icol = 0; icol < 512; icol ++) {
        for (int iaddr = 0; iaddr < 1024; iaddr ++) {
          HitData[ichip][icharge-myChargeStart][icol][iaddr] = 0;
        }
      }
    }
  }
}


void CopyHitData(std::vector <TPixHit> *Hits, int charge) {
  for (unsigned int ihit = 0; ihit < Hits->size(); ihit ++) {
    HitData[Hits->at(ihit).chipId][charge-myChargeStart][Hits->at(ihit).dcol + Hits->at(ihit).region * 16][Hits->at(ihit).address] ++;
  }
  Hits->clear();
}


void WriteDataToFile (const char *fName, bool Recreate) {
  char  fNameChip[100];
  FILE *fp;

  char fNameTemp[100];
  sprintf(fNameTemp,"%s", fName);
  std::cout << "WriteData:  fNameTemp = " << fNameTemp << std::endl;
  strtok (fNameTemp, ".");
  bool  HasData;

  for (unsigned int ichip = 0; ichip < fChips.size(); ichip ++) {
    int chipId = fChips.at(ichip)->GetConfig()->GetChipId() & 0xf;
    int ctrInt = fChips.at(ichip)->GetConfig()->GetCtrInt();
    if (!fChips.at(ichip)->GetConfig()->IsEnabled()) continue;  // write files only for enabled chips
    if (fChips.size() > 1) {
      sprintf(fNameChip, "%s_Chip%d_%d.dat", fNameTemp, chipId, ctrInt);
    }
    else {
      sprintf(fNameChip, "%s.dat", fNameTemp);
    }
    if (Recreate) fp = fopen(fNameChip, "w");
    else          fp = fopen(fNameChip, "a");

    for (unsigned int icol = 0; icol < 512; icol ++) {
      for (unsigned int iaddr = 0; iaddr < 1024; iaddr ++) {
        HasData = false;
        for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
          if (HitData[chipId][icharge - myChargeStart][icol][iaddr] > 0) HasData = true;
        }

        if (HasData) {
          for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
            fprintf(fp, "%d %d %d %d\n", icol, iaddr, icharge, HitData[chipId][icharge - myChargeStart][icol][iaddr]);
	  }
        }
      }
    }
    fclose (fp);
  }
}


void configureFromu(TAlpide *chip) {
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  0x20);            // fromu config 1: digital pulsing (put to 0x20 for analogue)
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  chip->GetConfig()->GetParamValue("STROBEDURATION"));  // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, chip->GetConfig()->GetParamValue("STROBEDELAYCHIP"));   // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, chip->GetConfig()->GetParamValue("PULSEDURATION"));   // fromu pulsing 2: pulse length
  //return 0;
}


void configureChip(TAlpide *chip) {
  AlpideConfig::BaseConfig(chip);

  configureFromu(chip);

  AlpideConfig::ConfigureCMU (chip);

  //return 0;
}


void WriteScanConfig(const char *fName, TAlpide *chip, TReadoutBoardDAQ *daqBoard) {
  char Config[1000];
  FILE *fp = fopen(fName, "w");

  chip     -> DumpConfig("", false, Config);
  std::cout << Config << std::endl;
  fprintf(fp, "%s\n", Config);
  if(daqBoard) daqBoard -> DumpConfig("", false, Config);
  fprintf(fp, "%s\n", Config);
  std::cout << Config << std::endl;

  fprintf(fp, "\n");

  fprintf(fp, "NTRIGGERS %i\n", myNTriggers);
  fprintf(fp, "MASKSTAGES %i\n", myMaskStages);

  fprintf(fp, "CHARGESTART %i\n", myChargeStart);
  fprintf(fp, "CHARGESTOP %i\n", myChargeStop);


  fclose(fp);
}


void fillVcasn(float *vcasn) { //WIP
  int old_vcas; //only current is used for now; the rest may be used later
  int old_ith;
  int goodPixels;
  float voltage;
  float voltageRMS;
  float noise;
  float noiseRMS;
  char name[100];

  std::cout << "Filling Vcasn" << std::endl;
  for(unsigned int i = 0; i < fChips.size(); i++) {
    //get file; name of one of them is in summaryName.
    //Only use first 34 chars of summaryName; insert chip # right after.

    sprintf(name, "%s%i_0.dat", summaryName.c_str(), i);
    FILE *fp = fopen(name, "r");
    //load file into array
    if(fp) {
      int result = fscanf(fp, "%i %i %i %f %f %f %f", &old_vcas, &old_ith, &goodPixels, &voltage,
                          &voltageRMS, &noise, &noiseRMS);
      if (result!=7) {
          std::cout << "Did not read all 7 values" << std::endl;
      }
      vcasn[i]=voltage;
      fclose(fp);
    } else {
      std::cout << "Unable to open file." << std::endl;
      vcasn[i]=-1;
    }
  }
}



void scan() {
  unsigned char         buffer[1024*4000];
  int                   n_bytes_data, n_bytes_header, n_bytes_trailer;
  int                   nBad = 0, nSkipped = 0, prioErrors =0, errors8b10b = 0;
  float *vcasn = new float[14]; //shouldn't have >14 chips
  TBoardHeader          boardInfo;
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;
  std::vector<int> myVPULSEH;

  TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC*> (fBoards.at(0));

  Hits->clear();
  if (myMOSAIC) {
    myMOSAIC->StartRun();
  }

  fillVcasn(vcasn); //NEW--fill vcasn with calibrated values for each chip

  for (unsigned int i = 0; i < fChips.size(); i++) { //Read VPULSEH from Config and save it at vector temporarily
    myVPULSEH.push_back(fChips.at(i)->GetConfig()->GetParamValue("VPULSEH"));
  }

  std::cout << "Initializing Vcasn array" << std::endl;
  //set VCASN for each chip here!
  for (int i = (int)fChips.size()-1; i>-1; i--) {
    if (! fChips.at(i)->GetConfig()->IsEnabled()) continue;
    fChips.at(i)->WriteRegister(Alpide::REG_VCASN, (int)(vcasn[i]+.5));
    //casts to int, with rounding
    fChips.at(i)->WriteRegister(Alpide::REG_VCASN2, (int)(vcasn[i]+.5)+12); //added recently
  }

  for (int istage = 0; istage < myMaskStages; istage ++) {
    std::cout << "Mask stage " << istage << std::endl;
    for (unsigned int i = 0; i < fChips.size(); i ++) {
      if (! fChips.at(i)->GetConfig()->IsEnabled()) continue;
      AlpideConfig::ConfigureMaskStage(fChips.at(i), myPixPerRegion, istage);
    }

    for (int icharge = myChargeStart; icharge < myChargeStop; icharge ++) {
      //std::cout << "Charge = " << icharge << std::endl;
      for (unsigned int i = 0; i < fChips.size(); i ++) {
        if (! fChips.at(i)->GetConfig()->IsEnabled()) continue;
        if (vcasn[i] == -1) continue;  //Summary file does not exist

        fChips.at(i)->WriteRegister (Alpide::REG_VPULSEL, myVPULSEH[i] - icharge);  //Automatically matches max pulse = VPULSEH in config
      }
      fBoards.at(0)->Trigger(myNTriggers);

//std::cout << " >>>>" << myMOSAIC->GetConfig()->GetPollingDataTimeout() << endl;

      int itrg = 0;
      int trials = 0;
      while(itrg < myNTriggers * fEnabled) {
        if (fBoards.at(0)->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
          usleep(1000); // Increment from 100us
          trials ++;
          if (trials == 10) {
        	std::cout << "Reached 10 timeouts, giving up on this event" << std::endl;
            itrg = myNTriggers * fEnabled;
            nSkipped ++;
            trials = 0;
          }
          continue;
        }
        else {
          BoardDecoder::DecodeEvent(fBoards.at(0)->GetConfig()->GetBoardType(), buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
          if (boardInfo.decoder10b8bError) errors8b10b++;
          // decode Chip event
          int n_bytes_chipevent=n_bytes_data-n_bytes_header;//-n_bytes_trailer;
          if (boardInfo.eoeCount < 2) n_bytes_chipevent -= n_bytes_trailer;
          if (!AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits, 0, boardInfo.channel, prioErrors)) {
   	    	std::cout << "Found bad event, length = " << n_bytes_chipevent << std::endl;
	    	nBad ++;
            if (nBad > 10) continue;
	    	FILE *fDebug = fopen ("DebugData.dat", "a");
            fprintf(fDebug, "Bad event:\n");
            for (int iByte=0; iByte<n_bytes_data + 1; ++iByte) {
              fprintf (fDebug, "%02x ", (int) buffer[iByte]);
            }
            fprintf(fDebug, "\nFull Event:\n");
            for (unsigned int ibyte = 0; ibyte < fDebugBuffer.size(); ibyte ++) {
              fprintf (fDebug, "%02x ", (int) fDebugBuffer.at(ibyte));
            }
            fprintf(fDebug, "\n\n");
            fclose (fDebug);
	  	  }
          itrg++;
        }
      }
	  //usleep(100);

      //std::cout << "Number of hits: " << Hits->size() << std::endl;
      CopyHitData(Hits, icharge);
    }
  }

  if (myMOSAIC) {
    myMOSAIC->StopRun();
  }


  std::cout << std::endl;
  if (myMOSAIC) {
    myMOSAIC->StopRun();
    std::cout << "Total number of 8b10b decoder errors: " << errors8b10b << std::endl;
  }
  std::cout << "Number of corrupt events:             " << nBad       << std::endl;
  std::cout << "Number of skipped points:             " << nSkipped   << std::endl;
  std::cout << "Priority encoder errors:              " << prioErrors << std::endl;
  std::cout << std::endl;
  std::cout << fEnabled << " chips were enabled for scan." << std::endl << std::endl;
}


int main(int argc, char** argv) {
  if(!argv[1]) {
    std::cout << "ERROR:  No Summary file provided by the command line!" << std::endl;
    return 1;
  }
  summaryName = argv[1]; //use the first 35 characters ONLY--see above
  summaryName = summaryName.substr(0,35);
  std::cout << "Summary " << summaryName << std::endl;

  decodeCommandParameters(argc, argv);
  InitHitData();
  initSetup(fConfig,  &fBoards,  &fBoardType, &fChips);
  InitScanParameters();
  char Suffix[20], fName[100]; //, Config[1000];

  ClearHitData();
  time_t       t = time(0);   // get time now
  struct tm *now = localtime( & t );
  sprintf(Suffix, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

  TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));

  if (fBoards.size() == 1) {

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

    for (unsigned int i = 0; i < fChips.size(); i ++) {
      if (fChips.at(i)->GetConfig()->IsEnabled()) {
        fEnabled ++;
        configureChip (fChips.at(i));
      }
      else if (fChips.at(i)->GetConfig()->HasEnabledSlave()) {
	AlpideConfig::BaseConfigPLL(fChips.at(i));
      }
    }

    std::cout << "Found " << fEnabled << " enabled chips." << std::endl;
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);

    // put your test here...
    if (fBoards.at(0)->GetConfig()->GetBoardType() == boardDAQ) {
      // for the DAQ board the delay between pulse and strobe is 12.5ns * pulse delay + 25 ns * strobe delay
      // pulse delay cannot be 0, therefore set strobe delay to 0 and use only pulse delay
      fBoards.at(0)->SetTriggerConfig (true, false,
                                       0,
                                       2 * fBoards.at(0)->GetConfig()->GetParamValue("STROBEDELAYBOARD"));
      fBoards.at(0)->SetTriggerSource (trigExt);
    }
    else {
      fBoards.at(0)->SetTriggerConfig (true, true,
                                       fBoards.at(0)->GetConfig()->GetParamValue("STROBEDELAYBOARD"),
                                       fBoards.at(0)->GetConfig()->GetParamValue("PULSEDELAY"));
      fBoards.at(0)->SetTriggerSource (trigInt);
    }

    scan();
    std::cout << "SUFFIX: " << Suffix << std::endl;
    sprintf(fName, "Data/ThresholdScan_%s.dat", Suffix);
    WriteDataToFile (fName, true);
    sprintf(fName, "Data/ScanConfig_%s_0.cfg", Suffix);

    if (myDAQBoard) {
      WriteScanConfig (fName, fChips.at(0), myDAQBoard);
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  DeleteHitData();
  return 0;
}

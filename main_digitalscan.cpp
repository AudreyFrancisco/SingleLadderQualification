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



// !!! NOTE: Scan parameters are now set via Config file

int myNTriggers;
int myMaskStages;
int myPixPerRegion;

int fEnabled = 0;  // variable to count number of enabled chips; leave at 0

int HitData[16][512][1024];


void InitScanParameters() {
  myMaskStages    = fConfig->GetScanConfig()->GetParamValue("NMASKSTAGES");
  myPixPerRegion  = fConfig->GetScanConfig()->GetParamValue("PIXPERREGION");
  myNTriggers     = fConfig->GetScanConfig()->GetParamValue("NINJ");
}


void ClearHitData() {
  for (int ichip = 0; ichip < 16; ichip ++) {
    for (int icol = 0; icol < 512; icol ++) {
      for (int iaddr = 0; iaddr < 1024; iaddr ++) {
        HitData[ichip][icol][iaddr] = 0;
      }
    }
  }
}


void CopyHitData(std::vector <TPixHit> *Hits) {
  for (int ihit = 0; ihit < Hits->size(); ihit ++) {
    int chipId  = Hits->at(ihit).chipId;
    int dcol    = Hits->at(ihit).dcol;
    int region  = Hits->at(ihit).region;
    int address = Hits->at(ihit).address;
    if ((chipId < 0) || (dcol < 0) || (region < 0) || (address < 0)) {
      std::cout << "Bad pixel coordinates ( <0), skipping hit" << std::endl;
    }
    else {
      HitData[Hits->at(ihit).chipId][Hits->at(ihit).dcol + Hits->at(ihit).region * 16][Hits->at(ihit).address] ++;
    }
  }
  Hits->clear();
}


bool HasData(int chipId) {
  for (int icol = 0; icol < 512; icol ++) {
    for (int iaddr = 0; iaddr < 1024; iaddr ++) {
      if (HitData[chipId][icol][iaddr] > 0) return true;
    }
  }
  return false;
}


void WriteDataToFile (const char *fName, bool Recreate) {
  char  fNameChip[100];
  FILE *fp;

  char fNameTemp[100];
  sprintf(fNameTemp,"%s", fName);
  strtok (fNameTemp, "."); 

  for (int ichip = 0; ichip < fChips.size(); ichip ++) {
    std::cout << "ichip = "<<ichip << std::endl;
    int chipId = fChips.at(ichip)->GetConfig()->GetChipId() & 0xf;
    if (!HasData(chipId)) continue;  // write files only for chips with data
    if (fChips.size() > 1) {
      sprintf(fNameChip, "%s_Chip%d.dat", fNameTemp, chipId);
    }
    else {
      sprintf(fNameChip, "%s.dat", fNameTemp);
    }
    std::cout << "Writing data to file "<< fNameChip <<std::endl;

    if (Recreate) fp = fopen(fNameChip, "w");
    else          fp = fopen(fNameChip, "a");
    for (int icol = 0; icol < 512; icol ++) {
      for (int iaddr = 0; iaddr < 1024; iaddr ++) {
        if (HitData[chipId][icol][iaddr] > 0) {
          fprintf(fp, "%d %d %d\n", icol, iaddr, HitData[chipId][icol][iaddr]);
        }
      }
    }
    if (fp) fclose (fp);
  }

}


// initialisation of Fromu
int configureFromu(TAlpide *chip) {
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  0x0);             // fromu config 1: digital pulsing (put to 0x20 for analogue)
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  chip->GetConfig()->GetParamValue("STROBEDURATION"));  // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, chip->GetConfig()->GetParamValue("STROBEDELAYCHIP"));   // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, chip->GetConfig()->GetParamValue("PULSEDURATION"));   // fromu pulsing 2: pulse length 
}


int configureChip(TAlpide *chip) {
  AlpideConfig::BaseConfig(chip);

  configureFromu(chip);

  AlpideConfig::ConfigureCMU (chip);
}


void scan() {   
  unsigned char         buffer[1024*4000]; 
  int                   n_bytes_data, n_bytes_header, n_bytes_trailer, errors8b10b, nClosedEvents = 0;
  int                   nBad = 0;
  TBoardHeader          boardInfo;
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;

  TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC*> (fBoards.at(0));

  if (myMOSAIC) {
    myMOSAIC->StartRun();
  }

  for (int istage = 0; istage < myMaskStages; istage ++) {
    std::cout << "Mask stage " << istage << std::endl;
    for (int i = 0; i < fChips.size(); i ++) {
      if (! fChips.at(i)->GetConfig()->IsEnabled()) continue;
      AlpideConfig::ConfigureMaskStage (fChips.at(i), myPixPerRegion, istage);
    }

    //uint16_t Value;
    //fChips.at(0)->ReadRegister(Alpide::REG_CMUDMU_CONFIG, Value);
    //std::cout << "CMU DMU Config: 0x" << std::hex << Value << std::dec << std::endl;
    //fChips.at(0)->ReadRegister(Alpide::REG_FROMU_STATUS1, Value);
    //std::cout << "Trigger counter before: " << Value << std::endl;
    fBoards.at(0)->Trigger(myNTriggers);
    //fChips.at(0)->ReadRegister(Alpide::REG_FROMU_STATUS1, Value);
    //std::cout << "Trigger counter after: " << Value << std::endl;

    //fBoards.at(0)->SendOpCode(Alpide::OPCODE_DEBUG);
    //AlpideConfig::PrintDebugStream(fChips.at(0));

    int itrg = 0;
    while(itrg < myNTriggers * fEnabled) {
      if (fBoards.at(0)->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
        usleep(100);
        continue;
      }
      else {
        //std::cout << "received Event" << itrg << " with length " << n_bytes_data << std::endl; 
        //for (int iByte=0; iByte<n_bytes_data; ++iByte) {
        //  printf ("%02x ", (int) buffer[iByte]);
        //}
        //std::cout << std::endl;
            
        // decode DAQboard event
        BoardDecoder::DecodeEvent(fBoards.at(0)->GetConfig()->GetBoardType(), buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
	//std::cout << "Closed data counter: " <<  boardInfo.eoeCount << std::endl;
        if (boardInfo.eoeCount) {
          nClosedEvents = boardInfo.eoeCount;
        }
        else {
 	      nClosedEvents = 1;
        }
        if (boardInfo.decoder10b8bError) errors8b10b++;
        // decode Chip event
        int n_bytes_chipevent=n_bytes_data-n_bytes_header-n_bytes_trailer;
        if (!AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits)) {
	      std::cout << "Found bad event " << std::endl;
	      nBad ++;
          if (nBad > 10) continue;
	      FILE *fDebug = fopen ("DebugData.dat", "a");
          for (int iByte=0; iByte<n_bytes_data; ++iByte) {
            fprintf (fDebug, "%02x ", (int) buffer[iByte]);
          }
          fclose (fDebug);
        }
        //std::cout << "total number of hits found: " << Hits->size() << std::endl;

        itrg+= nClosedEvents;
      }
    }

    
    //std::cout << "Hit pixels: " << std::endl;
    //for (int i=0; i<Hits->size(); i++) {
    //  std::cout << i << ":\t region: " << Hits->at(i).region << "\tdcol: " << Hits->at(i).dcol << "\taddres: " << Hits->at(i).address << std::endl; 
    //}
    CopyHitData(Hits);
  }
  if (myMOSAIC) {
    myMOSAIC->StopRun();
    std::cout << "Total number of 8b10b decoder errors: " << errors8b10b << std::endl;
  }
}


int main() {
  initSetup();

  sleep(1);
  char Suffix[20], fName[100];

  InitScanParameters();
  ClearHitData();
  time_t       t = time(0);   // get time now
  struct tm *now = localtime( & t );
  sprintf(Suffix, "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

  TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (fBoards.at(0));
  
  if (fBoards.size() == 1) {
     
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_GRST);
    fBoards.at(0)->SendOpCode (Alpide::OPCODE_PRST);

    for (int i = 0; i < fChips.size(); i ++) {
      if (! fChips.at(i)->GetConfig()->IsEnabled()) continue;
      fEnabled ++;
      std::cout << "Configuring chip " << i << ", chip ID = "<< fChips.at(i)->GetConfig()->GetChipId()<< std::endl;
      configureChip (fChips.at(i));
    }
    std::cout << "Found " << fEnabled << " enabled chips" << std::endl;

    fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);     

    // put your test here... 
    if (fBoards.at(0)->GetConfig()->GetBoardType() == boardMOSAIC) {
      fBoards.at(0)->SetTriggerConfig (true, true,
                                       fBoards.at(0)->GetConfig()->GetParamValue("STROBEDELAYBOARD"),
                                       fBoards.at(0)->GetConfig()->GetParamValue("PULSEDELAY")); 
      fBoards.at(0)->SetTriggerSource (trigInt);
    }
    else if (fBoards.at(0)->GetConfig()->GetBoardType() == boardDAQ) {
      fBoards.at(0)->SetTriggerConfig (true, false, 
                                       fBoards.at(0)->GetConfig()->GetParamValue("STROBEDELAYBOARD"),
                                       fBoards.at(0)->GetConfig()->GetParamValue("PULSEDELAY"));
      fBoards.at(0)->SetTriggerSource (trigExt);
    }

    scan();

    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }


  sprintf(fName, "Data/DigitalScan_%s.dat", Suffix);
  WriteDataToFile (fName, true);
  return 0;
}

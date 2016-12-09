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

int myVCASN   = 57;
int myITHR    = 51;
int myVCASN2  = 64;
int myVCLIP   = 0;
int myVRESETD = 147;

int myStrobeLength = 20;      // strobe length in units of 25 ns
int myStrobeDelay  = 10;
int myPulseLength  = 500;

int myPulseDelay   = 40;
//int myNTriggers    = 1000000;
int myNTriggers    = 1000000000;
//int myNTriggers    = 100;

int fEnabled = 0;  // variable to count number of enabled chips; leave at 0

int HitData     [16][512][1024];

vector<int> myChipId(32,0);

int CHIPRCVMAP [] = { 3, 5, 7, 8, 6, 4, 2, 1, 0 };
int RCVCHIPMAP [] = { 8, 7, 6, 0, 5, 1, 4, 2, 3 };

int chipIdToRcv(int chipId) {
    if(chipId < 0 || chipId > 8) {
        std::cout << "WARNING, chipIdToRcv, chipId out of range!" << std::endl;
        return -1;
    }
    else return CHIPRCVMAP[chipId];
}

int rcvToChipId(int rcv) {
    if(rcv < 0 || rcv > 8) {
        std::cout << "WARNING, rcvToChipId, reciever out of range!" << std::endl;
        return -1;
    }
    else return RCVCHIPMAP[rcv];
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
            HitData[chipId][dcol + region * 16][address] ++;
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

    char  fNameTemp[100];
    sprintf(fNameTemp,"%s", fName);
    strtok (fNameTemp, ".");

    for (int ichip = 0; ichip < fChips.size(); ichip ++) {
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
                if (HitData[ichip][icol][iaddr] > 0) {
                    fprintf(fp, "%d %d %d\n", icol, iaddr, HitData[ichip][icol][iaddr]);
                }
            }
        }
        if (fp) fclose (fp);
    }
}

vector<FILE*> InitDataFile(const char *fName, bool Recreate) {
    char  fNameChip[100];
    vector<FILE*> fp(fChips.size());
    cout << fChips.size() << endl;

    char  fNameTemp[100];
    sprintf(fNameTemp,"%s", fName);
    strtok (fNameTemp, ".");

    for (int ichip = 0; ichip < fChips.size(); ichip ++) {
        int chipId = fChips.at(ichip)->GetConfig()->GetChipId() & 0xf;
        myChipId[chipId] = ichip;
        if (fChips.size() > 1) {
            sprintf(fNameChip, "%s_Chip%d.dat", fNameTemp, chipId);
        }
        else {
            sprintf(fNameChip, "%s.dat", fNameTemp);
        }
        std::cout << "Writing data to file "<< fNameChip <<std::endl;

        if (Recreate) fp[ichip] = fopen(fNameChip, "w");
        else          fp[ichip] = fopen(fNameChip, "a");
    }
    return fp;
}

void WriteDataToFile(vector<FILE*> fp,vector<TPixHit>* Hits, int nevent) {
    for (int ihit = 0; ihit < Hits->size(); ihit ++) {
        int chipId  = Hits->at(ihit).chipId;
        int dcol    = Hits->at(ihit).dcol;
        int region  = Hits->at(ihit).region;
        int address = Hits->at(ihit).address;
        if ((chipId < 0) || (dcol < 0) || (region < 0) || (address < 0)) {
            std::cout << "Bad pixel coordinates ( <0), skipping hit" << std::endl;
        }
        else {
//        cout << myChipId[chipId] << endl;
            fprintf(fp[myChipId[chipId]], "%d %d %d %d\n", nevent,dcol + region*16, address, 1);
        }
    }
    Hits->clear();
}



// initialisation of Fromu
int configureFromu(TAlpide *chip) {
    chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  0x0);            // fromu config 1: digital pulsing (put to 0x20 for analogue)
    chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  myStrobeLength);  // fromu config 2: strobe length
    chip->WriteRegister(Alpide::REG_FROMU_PULSING1, myStrobeDelay);   // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
    // chip->WriteRegister(Alpide::REG_FROMU_PULSING2, myPulseLength);   // fromu pulsing 2: pulse length 
}


// initialisation of fixed mask
int configureMask(TAlpide *chip) {
    AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_MASK,   false);
    AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_SELECT, false);
}


int configureChip(TAlpide *chip) {
    AlpideConfig::BaseConfig(chip);

    configureFromu(chip);
    configureMask (chip);
    AlpideConfig::ConfigureCMU (chip);
    //  chip->WriteRegister (Alpide::REG_MODECONTROL, 0x21); // strobed readout mode
}

void WriteScanConfig(const char *fName, TAlpide *chip, TReadoutBoardDAQ *daqBoard) {
    char Config[1000];
    FILE *fp = fopen(fName, "w");

    chip     -> DumpConfig("", false, Config);
    //std::cout << Config << std::endl;
    fprintf(fp, "%s\n", Config);
    if (daqBoard) daqBoard -> DumpConfig("", false, Config);
    fprintf(fp, "%s\n", Config);
    //std::cout << Config << std::endl;

    fprintf(fp, "\n", Config);

    fprintf(fp, "NTRIGGERS %i\n", myNTriggers);
    
    fclose(fp);
}


void scan(vector<FILE*> fp) {   
    unsigned char         buffer[1024*4000]; 
    int                   n_bytes_data, n_bytes_header, n_bytes_trailer, nClosedEvents = 0, skipped = 0;
    TBoardHeader          boardInfo;
    std::vector<TPixHit> *Hits = new std::vector<TPixHit>;


    std::cout << "NTriggers: " << myNTriggers << std::endl;

    TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC*> (fBoards.at(0));

    if (myMOSAIC) {
        myMOSAIC->StartRun();
    }
    
    // This is needed to crash the run immediately if there is something wrong. Otherwise it is just stuck  FIX ME!!!
    for(int ichip=0; ichip < fEnabled; ++ichip) {
        std::cout << fBoards.at(0)->ReadEventData(n_bytes_data, buffer) << std::endl;
    }

    int received_cnt[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    bool flagMissingData = false;
    int recTriggers = 0; // number of read out triggers
    while(recTriggers < myNTriggers) {
        // polling for triggers
        int readyTriggers = myMOSAIC->GetTriggerCount();
        //std::cout << "Waiting for triggers" << std::flush;
        while(readyTriggers == recTriggers) {
            usleep(1e5);
            readyTriggers = myMOSAIC->GetTriggerCount();
            std::cout << "." << std::flush;
        }
        //std::cout << std::endl;
        int trigsToRead = readyTriggers > myNTriggers ? myNTriggers - recTriggers : readyTriggers - recTriggers;

        for(int itrig=0; itrig < trigsToRead; ++itrig) {
            int triggerRcvd[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // flag for each receiever. checking if corresponding reciever has been processed
            for(int ichip=flagMissingData; ichip < fEnabled; ++ichip) {
                if(flagMissingData) {
                    std::cout << "INFO, main_na61, " << recTriggers << " " << itrig << " " << ichip 
                              << ", missing / advanced data situation solved" << std::endl;
                    flagMissingData = false;
                }
                // wait for data if needed
                while (fBoards.at(0)->ReadEventData(n_bytes_data, buffer) == -1) {
                    std::cout << "Waiting for data..." << std::endl;
                    usleep(1e2);
                }
                
                // decode DAQboard event
                BoardDecoder::DecodeEvent(fBoards.at(0)->GetConfig()->GetBoardType(), buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);

                int rcv = boardInfo.channel-1; // MOSAIC is counting receivers from 1
                if(triggerRcvd[rcv]) {
                    std::cout << "ERROR, main_na61, " << recTriggers << " " << itrig << " " << ichip 
                              << ", already processed reciever " << rcv << " - chip " << rcvToChipId(rcv) << std::endl;
                    flagMissingData = true;
                    //std::cout << "Processed receivers: ";
                    //for(int i=0; i<9; ++i) 
                    //    std::cout << triggerRcvd[i] << "  ";
                    //std::cout << std::endl;
                }

                int expChipId = rcvToChipId(rcv); // expected chip id
                if (expChipId == -1) {
                    std::cout << "FATAL, main_na61, " << recTriggers << " " << itrig << " " << ichip 
                              << ", ChipID - RCV problem. Manual code check needed! Exiting!" << std::endl;
                    return;
                }
                received_cnt[expChipId]++;
                triggerRcvd[rcv]++;

                // decode Chip event
                int n_bytes_chipevent=n_bytes_data-n_bytes_header-n_bytes_trailer;
                bool Decode = AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits);
          
                if(!Decode) {
                    std::cout << "WARNING, main_na61, " << recTriggers << " " << itrig << " " << ichip
                              << ", Decode failed! (receiver " << rcv << " - chipID" << rcvToChipId(rcv) << ")!" << std::endl; 
                }

                if(Hits->size()) {
                    // check hits consistency
                    int prevChipId = -999;
                    for (int ihit = 0; ihit < Hits->size(); ihit ++) {
                        int iChipId  = Hits->at(ihit).chipId;
                        if(iChipId < 0 || iChipId > 8)
                            std::cout << "WARNING, main_na61, " << recTriggers << " " << itrig << " " << ichip 
                                      << ", ChipId out of range fo IB HIC = " << iChipId << std::endl;
                        if(prevChipId != -999 && prevChipId != iChipId)
                            std::cout << "WARNING, main_na61, " << recTriggers << " " << itrig << " " << ichip 
                                      << ", ChipId within same Hits changes! Previous " << prevChipId << " is now " << iChipId << std::endl;
                        prevChipId = iChipId;
                    }
                    if(prevChipId != rcvToChipId(rcv))
                        std::cout << "WARNING, main_na61, " << recTriggers << " " << itrig << " " << ichip
                                  << ", ChipId " << prevChipId << " not expected on reciever " << rcv << std::endl;                    
                }
                WriteDataToFile(fp, Hits, recTriggers + flagMissingData);
                if(flagMissingData) break;
            } // FOR chips
            recTriggers++;
        } // FOR triggers
        std::cout << "\r"
            //<< "Received triggers: " << itrg/fEnabled + itrain*nTrigsThisTrain << "   " 
                  << "Recorded Trigger Counter: " << recTriggers << "   "
                  << "MOSAIC Trigger Counter: " << myMOSAIC->GetTriggerCount() << "   "
            //<< "boardInfo.channel = " << boardInfo.channel << "   "
                  << std::flush;

        for (int ichips = 0; ichips < 9; ichips++) {
            fflush(fp[ichips]);
        }
        //std::cout << "Number of hits: " << Hits->size() << std::endl;
    } // WHILE Triggers
    std::cout << std::endl;

 
    if (myMOSAIC) {
        myMOSAIC->StopRun();
    }


    for(int i=0; i<9; ++i) 
        std::cout << received_cnt[i] << "  ";
    std::cout << std::endl;
}


int main(int argc, char *argv[]) {
    initSetup();

    char Suffix[20], fName[100];

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
            configureChip (fChips.at(i));
        }

        fBoards.at(0)->SendOpCode (Alpide::OPCODE_RORST);     

        // put your test here... 
        if (fBoards.at(0)->GetConfig()->GetBoardType() == boardMOSAIC) {
            fBoards.at(0)->SetTriggerConfig (true, true, myPulseDelay, myPulseLength * 10);
            fBoards.at(0)->SetTriggerSource (trigExt);
        }
        else if (fBoards.at(0)->GetConfig()->GetBoardType() == boardDAQ) {
            fBoards.at(0)->SetTriggerConfig (true, false, myStrobeDelay, myPulseDelay);
            fBoards.at(0)->SetTriggerSource (trigExt);
        }
        sprintf(fName, "Data/NoiseOccupancy_%s.dat", Suffix);
        vector<FILE*> fp = InitDataFile(fName, true);
        if (argc == 2) myNTriggers = atoi(argv[1]);

        scan(fp);

        for (int i = 0; i < fChips.size(); i++) {
            fclose(fp[i]);
        }

        if (myDAQBoard) {
            sprintf(fName, "Data/ScanConfig_%s.cfg", Suffix);
            WriteScanConfig (fName, fChips.at(0), myDAQBoard);
            myDAQBoard->PowerOff();
            delete myDAQBoard;
        }
    }

    return 0;
}

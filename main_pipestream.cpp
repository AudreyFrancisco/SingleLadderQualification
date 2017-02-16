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

#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <signal.h>


TBoardType fBoardType;
std::vector <TReadoutBoard *> fBoards;
std::vector <TAlpide *>       fChips;
TConfig *fConfig;

int myVCASN   = 57;
int myITHR    = 50;
int myVCASN2  = 64;
int myVCLIP   = 0;
int myVRESETD = 147;

int myStrobeLength = 20;      // strobe length in units of 25 ns
int myStrobeDelay  = 10;
int myPulseLength  = 4000;

int myPulseDelay   = 40;
//int myNTriggers    = 1000000;
int myNTriggers    = 1000000;
//int myNTriggers    = 100;

int fEnabled = 0;  // variable to count number of enabled chips; leave at 0



// -------------------------------------------------
#define DIMOFUNIXBUFFER 768
#define MICROSECTIMEOUT 200000
#define LINUXSOCKETNAME "/tmp/matestream"
#define BINARY_STREAM 1
#define ASCII_STREAM 2

 int theTypeOfStream = BINARY_STREAM;
 char SOCKNAME[255] = LINUXSOCKETNAME;
 int unixSocketHandler  =0;
 struct sockaddr_un unixSocketAddress;
 int theLenOfTXBuffer = 0;
 int numOfConnectedClients = 0;
 struct sockaddr_un clientAddress;
 socklen_t clientAddLen;
 int clientSocketHandler = NULL;
 bool isLocalCopyEnabled = false;
 int theTXTimeout = 0;
 char *theBuffer = NULL;
 char *theInsertionPtr = NULL;
 bool isSocketConnected = false;
 bool isBufferFull = false;
 pthread_mutex_t theUnixSocketServerMutex;
 pthread_t theUnixSocketServerThread;
 FILE *fhLocalStream;

 unsigned long numberOfEvents[16];
// --------------------------------------------------

void rea2xy(unsigned int region, unsigned int encoder, unsigned int address, unsigned int *x, unsigned int *y)
{
	unsigned int ap = (address & 0x3);
	*x = (region << 5) + (encoder << 1) + ((ap == 0x01 || ap == 0x02) ? 1 :0);
	*y = address >> 1;
	return;
}


void socketFlushBuffer()
{
	if( write(clientSocketHandler, theBuffer, theInsertionPtr-theBuffer-1) != (theInsertionPtr-theBuffer-1)) {
		std::cerr << "Failed to send on the UNIX socket" << std::endl;
	}
	theInsertionPtr = theBuffer;
	return;
}


void recordTheHit(int ADecChip, int ARegion, int AEncoder, int AAddress)
{
	char buf[20];
	unsigned int x, y;
	uint32_t hit;

	if(!isSocketConnected) return; // nothing to send ...
	pthread_mutex_lock(&theUnixSocketServerMutex);
	rea2xy(ARegion,AEncoder, AAddress, &x, &y);
	if(theTypeOfStream == BINARY_STREAM) {
		hit =  (y  | (x << 9) | (ADecChip << 19)) & 0x7FFFFFFF;
		*((uint32_t *)theInsertionPtr) = hit;
		theInsertionPtr += sizeof(uint32_t);
	} else {
		sprintf(buf,"%d,%d,%d\n", ADecChip, x,y);
		strcpy(theInsertionPtr,buf);
		theInsertionPtr += strlen(buf);
	}
	if((theInsertionPtr - theBuffer) > theLenOfTXBuffer) { // it's time to send
		socketFlushBuffer();
	}
	pthread_mutex_unlock(&theUnixSocketServerMutex);

	if(isLocalCopyEnabled) {
		fprintf(fhLocalStream, "%02d:%02d:%03d:%03d\n",ADecChip,ARegion,AEncoder,AAddress);
	}
}

void recordTheSoE(int ADecChip)
{
	char buf[20];
	uint32_t hit;

	numberOfEvents[ADecChip]++;

	if(!isSocketConnected) return; // nothing to send ...
	pthread_mutex_lock(&theUnixSocketServerMutex);
	if(theTypeOfStream == BINARY_STREAM) {
		hit = ((numberOfEvents[ADecChip] & 0x7FFFF) | (ADecChip << 19)) | 0x80000000;
		*((uint32_t *)theInsertionPtr) = hit;
		theInsertionPtr += sizeof(uint32_t);
	} else {
		sprintf(buf,"%d,%d\n", ADecChip,numberOfEvents[ADecChip]);
		strcpy(theInsertionPtr,buf);
		theInsertionPtr += strlen(buf);
	}
	if((theInsertionPtr - theBuffer) > theLenOfTXBuffer) { // it's time to send
		socketFlushBuffer();
	}
	pthread_mutex_unlock(&theUnixSocketServerMutex);

	if(isLocalCopyEnabled) {
		fprintf(fhLocalStream, "SOE %02d:%06d\n",ADecChip,numberOfEvents[ADecChip]);
	}
}

void CopyHitData(std::vector <TPixHit> *Hits) {
	int nHits =  Hits->size();
	if(nHits <= 0) return;

	recordTheSoE(Hits->at(0).chipId);
	for (int ihit = 0; ihit < Hits->size(); ihit ++) {
		int chipId  = Hits->at(ihit).chipId;
		int dcol    = Hits->at(ihit).dcol;
		int region  = Hits->at(ihit).region;
		int address = Hits->at(ihit).address;
		if ((chipId < 0) || (dcol < 0) || (region < 0) || (address < 0)) {
			std::cout << "Bad pixel coordinates ( <0), skipping hit" << std::endl;
		} else {
			recordTheHit(chipId,region, dcol,address);
		}
	}
	return;
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
  //chip->WriteRegister (Alpide::REG_MODECONTROL, 0x21); // strobed readout mode
}



void *runSocketServer(void *param)
{
	// here we create the UNIX socket
	unixSocketHandler = socket(AF_UNIX, SOCK_STREAM, 0);            /* Create socket */
	if (unixSocketHandler == -1) {
		std::cerr << "Error to create the UNIX socket. Abort!" << std::endl;
		exit(-1);
	}
	memset(&unixSocketAddress, 0, sizeof(struct sockaddr_un));     /* Clear structure */
	unixSocketAddress.sun_family = AF_UNIX;                            /* UNIX domain address */
	if (*SOCKNAME == '\0') {
		*unixSocketAddress.sun_path = '\0';
	    strncpy(unixSocketAddress.sun_path+1, SOCKNAME+1, sizeof(unixSocketAddress.sun_path)-2);
	} else {
		strncpy(unixSocketAddress.sun_path, SOCKNAME, sizeof(unixSocketAddress.sun_path) - 1);
		unlink(SOCKNAME); // remove if any
	}
	if (::bind(unixSocketHandler, (struct sockaddr *) &unixSocketAddress, sizeof(struct sockaddr_un)) == -1) {
		std::cerr << "Failed to bind the UNIX socket. Abort ! (" << unixSocketAddress.sun_path << ")" << std::endl;
		exit(-1);
	}
	std::cout << "UNIX Socket server created in : %s" << unixSocketAddress.sun_path << std::endl;
	while(true) {
		if( listen(unixSocketHandler, 1) == -1) {
			std::cerr << "Error to setup listening UNIX socket. Abort !" << std::endl;
			exit(1);
		}
		clientSocketHandler =  accept(unixSocketHandler,  (struct sockaddr *) &clientAddress, &clientAddLen);
		if(clientSocketHandler > 0) {
			numOfConnectedClients++;

			theBuffer = (char *)malloc(theLenOfTXBuffer+20);
			theInsertionPtr = theBuffer;
			isBufferFull = false;

			isSocketConnected = true;
		}
		while(isSocketConnected) {
			usleep(theTXTimeout);
			if(theInsertionPtr != theBuffer) {
				pthread_mutex_lock(&theUnixSocketServerMutex);
				socketFlushBuffer();
				pthread_mutex_unlock(&theUnixSocketServerMutex);
			}
		}
		if(theBuffer != NULL) { free(theBuffer); theBuffer = NULL; }
	}
	return(NULL);
}



void setTheStreamer()
{

	strcpy(SOCKNAME,LINUXSOCKETNAME); // the name of the UNIX socket

	theLenOfTXBuffer = DIMOFUNIXBUFFER;
	theTXTimeout = MICROSECTIMEOUT;

	if (pthread_mutex_init(&theUnixSocketServerMutex, NULL) != 0) {
		std::cerr << "Can't create thread mutex" << std::endl;
		exit(-1);
    }
	int par =0;
	int err = pthread_create(&theUnixSocketServerThread, NULL, runSocketServer, (void *)&par);
	if (err != 0) {
		std::cerr << "Can't create thread :[" <<  strerror(err) << "]" << std::endl;
		exit(-1);
	}  else {
		std::cout << "LINUX socket server thread created successfully" << std::endl;
	}
	return;

}


void unsetTheStreamer()
{
	if(theInsertionPtr != theBuffer) {
		pthread_mutex_lock(&theUnixSocketServerMutex);
		socketFlushBuffer();
		pthread_mutex_unlock(&theUnixSocketServerMutex);
	}
	//	closeFiles();

	if(theBuffer != NULL) {
		free(theBuffer);
		theBuffer = NULL;
	}
	if (*unixSocketAddress.sun_path != '\0') {
		unlink(SOCKNAME); // remove if any
	}
	if(theUnixSocketServerThread != NULL)
		pthread_kill(theUnixSocketServerThread, SIGINT);
	return;
}

void startTheStreaming()
{
	  unsigned char         buffer[1024*4000];
	  int                   n_bytes_data, n_bytes_header, n_bytes_trailer, nClosedEvents = 0, skipped = 0;
	  TBoardHeader          boardInfo;
	  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;

	  int nTrains, nRest, nTrigsThisTrain, nTrigsPerTrain = 100;

	  nTrains = myNTriggers / nTrigsPerTrain;
	  nRest   = myNTriggers % nTrigsPerTrain;

	  std::cout << "NTriggers: " << myNTriggers << std::endl;
	  std::cout << "NTriggersPerTrain: " << nTrigsPerTrain << std::endl;
	  std::cout << "NTrains: " << nTrains << std::endl;
	  std::cout << "NRest: " << nRest << std::endl;

	  TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC*> (fBoards.at(0));

	  if (myMOSAIC) {
	    myMOSAIC->StartRun();
	  }

	  for (int itrain = 0; itrain <= nTrains; itrain ++) {
	    std::cout << "Train: " << itrain << std::endl;
	    if (itrain == nTrains) {
	      nTrigsThisTrain = nRest;
	    }
	    else {
	      nTrigsThisTrain = nTrigsPerTrain;
	    }

	    fBoards.at(0)->Trigger(nTrigsThisTrain);

	    int itrg = 0;
	    int trials = 0;
	    while(itrg < nTrigsThisTrain * fEnabled) {
	        if (fBoards.at(0)->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
	          usleep(100);
	          trials ++;
	          if (trials == 3) {
	        	std::cout << "Reached 3 timeouts, giving up on this event" << std::endl;
	            itrg = nTrigsThisTrain * fEnabled;
	            skipped ++;
	            trials = 0;
	          }
	          continue;
	        }
	        else {
	          // decode DAQboard event
	          BoardDecoder::DecodeEvent(fBoards.at(0)->GetConfig()->GetBoardType(), buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
	          if (boardInfo.eoeCount) {
	            nClosedEvents = boardInfo.eoeCount;
	          }
	          else {
	   	        nClosedEvents = 1;
	          }
	          // decode Chip event
	          int n_bytes_chipevent=n_bytes_data-n_bytes_header-n_bytes_trailer;
	          bool Decode = AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits);
	          itrg+=nClosedEvents;
	          CopyHitData(Hits);
	        }
	      }
	      //std::cout << "Number of hits: " << Hits->size() << std::endl;

	  }
	  if (myMOSAIC) {
	    myMOSAIC->StopRun();
	  }


}

int main() {
  initSetup(fConfig, &fBoards, &fBoardType, &fChips);

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
      fBoards.at(0)->SetTriggerSource (trigInt);
    }
    else if (fBoards.at(0)->GetConfig()->GetBoardType() == boardDAQ) {
      fBoards.at(0)->SetTriggerConfig (true, false, myStrobeDelay, myPulseDelay);
      fBoards.at(0)->SetTriggerSource (trigExt);
    }

    setTheStreamer();


    startTheStreaming();


    unsetTheStreamer();

    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

  return 0;
}

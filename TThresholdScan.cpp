#include <unistd.h>
#include "TThresholdScan.h"
#include "TReadoutBoardMOSAIC.h"
#include "TReadoutBoardDAQ.h"
#include "AlpideDecoder.h"

TThresholdScan::TThresholdScan (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards) 
  : TMaskScan (config, chips, boards) 
{
  m_start[0]  = m_config->GetChargeStart();
  m_stop [0]  = m_config->GetChargeStop ();
  m_step [0]  = m_config->GetChargeStep ();
  m_VPULSEH   = 170;
  m_nTriggers = 50;
}


void TThresholdScan::PrepareStep (int loopIndex) 
{
  switch (loopIndex) {
  case 0:    // innermost loop: change VPULSEL
    for (int ichip = 0; ichip < m_chips.size(); ichip ++) {
      if (! m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      m_chips.at(ichip)->WriteRegister(Alpide::REG_VPULSEL, m_VPULSEH - m_value[0]);
    }
    break;
  case 1:    // 2nd loop: mask staging
    for (int ichip = 0; ichip < m_chips.size(); ichip ++) {
      if (! m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
      ConfigureMaskStage(m_chips.at(ichip), m_value[1]);
    }
    break;
  default: 
    break;
  }
}


void TThresholdScan::Execute() 
{
  unsigned char         buffer[1024*4000]; 
  int                   n_bytes_data, n_bytes_header, n_bytes_trailer;
  int                   nBad = 0, skipped = 0;
  TBoardHeader          boardInfo;
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;

  for (int iboard = 0; iboard < m_boards.size(); iboard ++) {
    m_boards.at(iboard)->Trigger(m_nTriggers);
  }

  for (int iboard = 0; iboard < m_boards.size(); iboard ++) {
    int itrg = 0;
    int trials = 0;
    while(itrg < m_nTriggers * m_enabled[iboard]) {
      if (m_boards.at(iboard)->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
        usleep(100);
        trials ++;
        if (trials == 3) {
      	  std::cout << "Board " << iboard << ": reached 3 timeouts, giving up on this event" << std::endl;
          itrg = m_nTriggers * m_enabled[iboard];
          skipped ++;
          trials = 0;
        }
        continue;
      }
      else {
        BoardDecoder::DecodeEvent(m_boards.at(iboard)->GetConfig()->GetBoardType(), buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
        // decode Chip event
        int n_bytes_chipevent=n_bytes_data-n_bytes_header;//-n_bytes_trailer;
        if (boardInfo.eoeCount < 2) n_bytes_chipevent -= n_bytes_trailer;
        if (!AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits)) {
          std::cout << "Found bad event, length = " << n_bytes_chipevent << std::endl;
          nBad ++;
          if (nBad > 10) continue;
	  FILE *fDebug = fopen ("DebugData.dat", "a");
          fprintf(fDebug, "Bad event:\n");
          for (int iByte=0; iByte<n_bytes_data + 1; ++iByte) {
            fprintf (fDebug, "%02x ", (int) buffer[iByte]);
          }
          fprintf(fDebug, "\nFull Event:\n"); 
          for (int ibyte = 0; ibyte < fDebugBuffer.size(); ibyte ++) {
            fprintf (fDebug, "%02x ", (int) fDebugBuffer.at(ibyte));
          }
          fprintf(fDebug, "\n\n");
          fclose (fDebug);
	}
        itrg++;
        }
      }
    //CopyHitData(Hits, m_value[0]);
  }
}


void TThresholdScan::Terminate() 
{
  // write Data;
  for (int iboard = 0; iboard < m_boards.size(); iboard ++) {
    TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC*> (m_boards.at(iboard));
    if (myMOSAIC) {
      myMOSAIC->StopRun();
      delete myMOSAIC;
    }
    TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (m_boards.at(iboard));
    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      delete myDAQBoard;
    }
  }

}


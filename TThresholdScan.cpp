#include <unistd.h>
#include "TThresholdScan.h"
#include "TReadoutBoardMOSAIC.h"
#include "TReadoutBoardDAQ.h"
#include "AlpideDecoder.h"
#include "AlpideConfig.h"


TThresholdScan::TThresholdScan (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards) 
  : TMaskScan (config, chips, boards) 
{
  m_start[0]  = m_config->GetChargeStart();
  m_stop [0]  = m_config->GetChargeStop ();
  m_step [0]  = m_config->GetChargeStep ();

  m_start[1]  = 0;
  m_step [1]  = 1;
  m_stop [1]  = m_config->GetNMaskStages();

  m_start[2]  = 0;
  m_step [2]  = 1;
  m_stop [2]  = 1;

  m_VPULSEH   = 170;
  m_nTriggers = m_config->GetParamValue("NINJ");
}


void TThresholdScan::ConfigureBoard(TReadoutBoard *board) 
{
  if (board->GetConfig()->GetBoardType() == boardMOSAIC) {
    board->SetTriggerConfig (true, true, 
                             board->GetConfig()->GetParamValue("STROBEDELAYBOARD"),
                             board->GetConfig()->GetParamValue("PULSEDELAY"));
    board->SetTriggerSource (trigInt);
  }
  else if (board->GetConfig()->GetBoardType() == boardDAQ) {
    // for the DAQ board the delay between pulse and strobe is 12.5ns * pulse delay + 25 ns * strobe delay
    // pulse delay cannot be 0, therefore set strobe delay to 0 and use only pulse delay
    board->SetTriggerConfig (true, false, 
                             0,
                             2 * board->GetConfig()->GetParamValue("STROBEDELAYBOARD"));
    board->SetTriggerSource (trigExt);
  }


}

 
void TThresholdScan::ConfigureFromu(TAlpide *chip) 
{
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  0x20);            // fromu config 1: digital pulsing (put to 0x20 for analogue)
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  chip->GetConfig()->GetParamValue("STROBEDURATION"));  // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, chip->GetConfig()->GetParamValue("STROBEDELAYCHIP"));   // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, chip->GetConfig()->GetParamValue("PULSEDURATION"));   // fromu pulsing 2: pulse length 
}


void TThresholdScan::ConfigureChip(TAlpide *chip)
{
  AlpideConfig::BaseConfig(chip);

  ConfigureFromu(chip);

  AlpideConfig::ConfigureCMU (chip);
}


void TThresholdScan::Init() {
  CountEnabledChips();
  for (int i = 0; i < m_boards.size(); i++) {
    std::cout << "Board " << i << ", found " << m_enabled[i] << " enabled chips" << std::endl;
    ConfigureBoard(m_boards.at(i));

    m_boards.at(i)->SendOpCode (Alpide::OPCODE_GRST);
    m_boards.at(i)->SendOpCode (Alpide::OPCODE_PRST);
  }

  for (int i = 0; i < m_chips.size(); i ++) {
    if (! (m_chips.at(i)->GetConfig()->IsEnabled())) continue;
    ConfigureChip (m_chips.at(i));
  }

  for (int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode (Alpide::OPCODE_RORST);     
    TReadoutBoardMOSAIC *myMOSAIC = dynamic_cast<TReadoutBoardMOSAIC*> (m_boards.at(i));

    if (myMOSAIC) {
     myMOSAIC->StartRun();
    } 
  }
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
    std::cout << "Found " << Hits->size() << " hits" << std::endl;
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
      //delete myMOSAIC;
    }
    TReadoutBoardDAQ *myDAQBoard = dynamic_cast<TReadoutBoardDAQ*> (m_boards.at(iboard));
    if (myDAQBoard) {
      myDAQBoard->PowerOff();
      //delete myDAQBoard;
    }
  }

}


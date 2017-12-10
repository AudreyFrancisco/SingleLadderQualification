#include <string.h>
#include <string>

#include "TDataTaking.h"
#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "TReadoutBoardRU.h"

TDataTaking::TDataTaking (TScanConfig                   *config, 
                          std::vector <TAlpide *>        chips, 
                          std::vector <THic*>            hics, 
                          std::vector <TReadoutBoard *>  boards, 
                          std::deque<TScanHisto>        *histoQue, 
                          std::mutex                    *aMutex) 
  : TScan (config, chips, hics, boards, histoQue, aMutex) 
{
  m_backBias    = m_config->GetBackBias  ();
  int nTriggers = m_config->GetParamValue("NTRIG");

  if (nTriggers % kTrigPerTrain == 0) {
    m_nLast   = kTrigPerTrain;
    m_nTrains = nTriggers / kTrigPerTrain;
  }
  else {
    m_nLast   = nTriggers % kTrigPerTrain;
    m_nTrains = nTriggers / kTrigPerTrain + 1;
  }
  // divide triggers in trains
  m_start[0] = 0;
  m_step [0] = 1;
  m_stop [0] = m_nTrains;

  m_start[1] = 0;
  m_step [1] = 1;
  m_stop [1] = 1;

  m_start[2] = 0;
  m_step [2] = 1;
  m_stop [2] = 1;

  CreateScanHisto();
}


void TDataTaking::ConfigureFromu (TAlpide *chip)
{
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  0x0);    // digital pulsing        
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  chip->GetConfig()->GetParamValue("STROBEDURATION"));  // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, chip->GetConfig()->GetParamValue("STROBEDELAYCHIP"));   // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, m_pulseLength);   // fromu pulsing 2: pulse length
}


void TDataTaking::ConfigureBoard (TReadoutBoard *board)
{
  if (board->GetConfig()->GetBoardType() == boardDAQ) {
    // for the DAQ board the delay between pulse and strobe is 12.5ns * pulse delay + 25 ns * strobe delay
    // pulse delay cannot be 0, therefore set strobe delay to 0 and use only pulse delay
    board->SetTriggerConfig (true, false, 
                             0,
                             2 * board->GetConfig()->GetParamValue("STROBEDELAYBOARD"));
    board->SetTriggerSource (trigExt);
  }
  else {
    board->SetTriggerConfig (m_pulse, true, 
                             board->GetConfig()->GetParamValue("STROBEDELAYBOARD"),
                             board->GetConfig()->GetParamValue("PULSEDELAY"));
    board->SetTriggerSource (trigInt);
  }
}


THisto TDataTaking::CreateHisto () 
{
  THisto histo ("HitmapHisto", "HitmapHisto", 1024, 0, 1023, 512, 0, 511);
  return histo;
}


void TDataTaking::Init        ()
{
  TScan::Init();

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TPowerBoard *pb = m_hics.at(ihic)->GetPowerBoard();
    if (!pb) continue;
    if (m_backBias == 0) {
      m_hics.at(ihic)->SwitchBias (false);
      pb             ->SetBiasVoltage(0);
    }
    else {
      m_hics.at(ihic)->SwitchBias   (true);
      pb             ->SetBiasVoltage( (-1.)* m_backBias);
    }
  }

  CountEnabledChips();
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    std::cout << "Board " << i << ", found " << m_enabled[i] << " enabled chips" << std::endl;
    ConfigureBoard(m_boards.at(i));

    m_boards.at(i)->SendOpCode (Alpide::OPCODE_GRST);
    m_boards.at(i)->SendOpCode (Alpide::OPCODE_PRST);
  }

  for (unsigned int i = 0; i < m_chips.size(); i ++) {
    if (! (m_chips.at(i)->GetConfig()->IsEnabled())) continue;
    ConfigureChip (m_chips.at(i));
  }
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode (Alpide::OPCODE_RORST);     
    m_boards.at(i)->StartRun   ();
  }
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    TPowerBoard *pb = m_hics.at(ihic)->GetPowerBoard();
    if (!pb) continue;
    pb->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod());
  }
}


// check which HIC caused the timeout, i.e. did not send enough events
// called only in case a timeout occurs
void TDataTaking::FindTimeoutHics (int iboard, int *triggerCounts, int nTriggers)
{
  for (unsigned int iHic = 0; iHic < m_hics.size(); iHic++) {
    bool isOnBoard = false;
    int  nTrigs    = 0;
    for (unsigned int iRcv = 0; iRcv < MAX_MOSAICTRANRECV; iRcv++) {
      if (m_hics.at(iHic)->ContainsReceiver(iboard, iRcv)) {
        isOnBoard = true;
        nTrigs   += triggerCounts[iRcv];
      }
    }
    // HIC is connected to this readout board AND did not send enough events
    if ((isOnBoard) && (nTrigs < nTriggers * m_hics.at(iHic)->GetNEnabledChips())) {
      m_errorCounts.at(m_hics.at(iHic)->GetDbId()).nTimeout ++;
    }
  }
}


void TDataTaking::ReadEventData (std::vector <TPixHit> *Hits, int iboard, int nTriggers)
{
  unsigned char buffer[1024*4000]; 
  int           n_bytes_data, n_bytes_header, n_bytes_trailer;
  int           itrg = 0, trials = 0;
  TBoardHeader  boardInfo;
  int           nTrigPerHic[MAX_MOSAICTRANRECV];

  for (unsigned int i = 0; i < MAX_MOSAICTRANRECV; i++) {
    nTrigPerHic[i] = 0;
  }

  while (itrg < nTriggers * m_enabled[iboard]) {
    if (m_boards.at(iboard)->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
      usleep(100);
      trials ++;
      if (trials == 3) {
  	  std::cout << "Board " << iboard << ": reached 3 timeouts, giving up on this event" << std::endl;
        itrg = nTriggers * m_enabled[iboard];
        FindTimeoutHics(iboard, nTrigPerHic, nTriggers);
        m_errorCount.nTimeout ++;
        trials = 0;
      }
      continue;
    }
    else {
      BoardDecoder::DecodeEvent(m_boards.at(iboard)->GetConfig()->GetBoardType(), buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
      // decode Chip event
      if (boardInfo.decoder10b8bError) {
        m_errorCount.n8b10b++;
        if (FindHIC(iboard, boardInfo.channel).compare ("None") != 0) {
          m_errorCounts.at(FindHIC(iboard, boardInfo.channel)).n8b10b++;
	}
      }
      int n_bytes_chipevent=n_bytes_data-n_bytes_header;//-n_bytes_trailer;
      if (boardInfo.eoeCount < 2) n_bytes_chipevent -= n_bytes_trailer;
      if (!AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits, iboard, boardInfo.channel, m_errorCount.nPrioEncoder, &m_stuck)) {
        std::cout << "Found bad event, length = " << n_bytes_chipevent << std::endl;
        m_errorCount.nCorruptEvent ++;
        if (FindHIC(iboard, boardInfo.channel).compare ("None") != 0) {
          m_errorCounts.at(FindHIC(iboard, boardInfo.channel)).nCorruptEvent++;
	}
      }
      itrg++;
    }
  }
}


void TDataTaking::FillHistos     (std::vector<TPixHit> *Hits, int board) 
{
  common::TChipIndex idx;
  idx.boardIndex = board;
   
  for (unsigned int i = 0; i < Hits->size(); i++) {
    idx.dataReceiver = Hits->at(i).channel;
    idx.chipId       = Hits->at(i).chipId;

    int col       = Hits->at(i).region * 32 + Hits->at(i).dcol * 2;
    int leftRight = ((((Hits->at(i).address % 4) == 1) || ((Hits->at(i).address % 4) == 2))? 1:0); 
    col          += leftRight;
    m_histo->Incr(idx, col, Hits->at(i).address / 2);
  }					  
}


void TDataTaking::LoopEnd (int loopIndex)
{
  if (loopIndex == 0) {
    while (!(m_mutex->try_lock()));
    m_histoQue->push_back(*m_histo);
    m_mutex   ->unlock   ();
    m_histo   ->Clear    ();
  }
}


void TDataTaking::Execute     ()
{
  std::vector<TPixHit> *Hits = new std::vector<TPixHit>;

  // always send kTrigPerTrain triggers, except for last train (in case total nTrig is no multiple of kTrigPerTrain)
  int nTriggers = (m_value[0] == m_stop[0] - 1) ? m_nLast : kTrigPerTrain;

  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard ++) {
    m_boards.at(iboard)->Trigger(nTriggers);
  }
  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard ++) {
		Hits->clear();
    ReadEventData(Hits, iboard, nTriggers);
    FillHistos   (Hits, iboard);
  }
  delete Hits;
}


void TDataTaking::Terminate ()
{
  TScan::Terminate();
  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard ++) {
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

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if (m_backBias != 0) {
      TPowerBoard *pb = m_hics.at(ihic)->GetPowerBoard();
      if (!pb) continue;
      m_hics.at(ihic)->SwitchBias (false);
      pb->SetBiasVoltage(0);
    }
  }

  m_running = false;
}
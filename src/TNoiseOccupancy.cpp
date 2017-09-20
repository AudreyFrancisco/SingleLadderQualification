#include <string.h>
#include <string>

#include "TNoiseOccupancy.h"
#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoardMOSAIC.h"
#include "TReadoutBoardRU.h"

TNoiseOccupancy::TNoiseOccupancy (TScanConfig                   *config, 
                                  std::vector <TAlpide *>        chips, 
                                  std::vector <THic*>            hics, 
                                  std::vector <TReadoutBoard *>  boards, 
                                  std::deque<TScanHisto>        *histoQue, 
                                  std::mutex                    *aMutex) 
  : TScan (config, chips, hics, boards, histoQue, aMutex) 
{
  strcpy(m_name, "Noise Occupancy");

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


void TNoiseOccupancy::ConfigureFromu (TAlpide *chip)
{
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,  0x0);    // digital pulsing        
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG2,  chip->GetConfig()->GetParamValue("STROBEDURATION"));  // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1, chip->GetConfig()->GetParamValue("STROBEDELAYCHIP"));   // fromu pulsing 1: delay pulse - strobe (not used here, since using external strobe)
  chip->WriteRegister(Alpide::REG_FROMU_PULSING2, 0);   // fromu pulsing 2: pulse length
}


void TNoiseOccupancy::ConfigureChip  (TAlpide *chip)
{
  AlpideConfig::BaseConfig   (chip);
  ConfigureFromu             (chip);
  ConfigureMask              (chip, 0);
  AlpideConfig::ApplyMask    (chip, true);
  AlpideConfig::ConfigureCMU (chip);
}


// TODO: add masking of MaskedPixels
void TNoiseOccupancy::ConfigureMask (TAlpide *chip, std::vector <TPixHit> *MaskedPixels)
{
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_MASK,   false);
  AlpideConfig::WritePixRegAll (chip, Alpide::PIXREG_SELECT, false);
}


void TNoiseOccupancy::ConfigureBoard (TReadoutBoard *board)
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
    board->SetTriggerConfig (false, true, 
                             board->GetConfig()->GetParamValue("STROBEDELAYBOARD"),
                             board->GetConfig()->GetParamValue("PULSEDELAY"));
    board->SetTriggerSource (trigInt);
  }
}


THisto TNoiseOccupancy::CreateHisto () 
{
  THisto histo ("HitmapHisto", "HitmapHisto", 1024, 0, 1023, 512, 0, 511);
  return histo;
}


void TNoiseOccupancy::Init        ()
{
  TScan::Init();
  m_running = true;
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
}


void TNoiseOccupancy::ReadEventData (std::vector <TPixHit> *Hits, int iboard, int nTriggers)
{
  unsigned char buffer[1024*4000]; 
  int           n_bytes_data, n_bytes_header, n_bytes_trailer;
  int           itrg = 0, trials = 0;
  TBoardHeader  boardInfo;

  while (itrg < nTriggers * m_enabled[iboard]) {
    if (m_boards.at(iboard)->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
      usleep(100);
      trials ++;
      if (trials == 3) {
  	  std::cout << "Board " << iboard << ": reached 3 timeouts, giving up on this event" << std::endl;
        itrg = nTriggers * m_enabled[iboard];
        m_errorCount.nTimeout ++;
        trials = 0;
      }
      continue;
    }
    else {
      BoardDecoder::DecodeEvent(m_boards.at(iboard)->GetConfig()->GetBoardType(), buffer, n_bytes_data, n_bytes_header, n_bytes_trailer, boardInfo);
      // decode Chip event
      if (boardInfo.decoder10b8bError) m_errorCount.n8b10b++;
      int n_bytes_chipevent=n_bytes_data-n_bytes_header;//-n_bytes_trailer;
      if (boardInfo.eoeCount < 2) n_bytes_chipevent -= n_bytes_trailer;
      if (!AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits, iboard, boardInfo.channel, m_errorCount.nPrioEncoder, &m_stuck)) {
        std::cout << "Found bad event, length = " << n_bytes_chipevent << std::endl;
        m_errorCount.nCorruptEvent ++;
      }
      itrg++;
    }
  }
}


void TNoiseOccupancy::FillHistos     (std::vector<TPixHit> *Hits, int board) 
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


void TNoiseOccupancy::LoopEnd (int loopIndex)
{
  if (loopIndex == 0) {
    while (!(m_mutex->try_lock()));
    m_histoQue->push_back(*m_histo);
    m_mutex   ->unlock   ();
    m_histo   ->Clear    ();
  }
}


void TNoiseOccupancy::Execute     ()
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


void TNoiseOccupancy::Terminate ()
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
  m_running = false;
}

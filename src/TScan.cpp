#include <iostream>
#include <string.h>
#include <string>

#include "Common.h"
#include "AlpideConfig.h"
#include "TReadoutBoardMOSAIC.h"
#include "TReadoutBoardRU.h"
#include "TReadoutBoardDAQ.h"
#include "TScan.h"

bool fScanAbort;


TScan::TScan (TScanConfig                   *config,
              std::vector <TAlpide *>        chips,
              std::vector <THic *>           hics,
              std::vector <TReadoutBoard *>  boards,
              std::deque<TScanHisto>        *histoQue,
              std::mutex                    *aMutex)
{
  m_config = config;
  m_chips  = chips;
  m_hics   = hics;
  m_boards = boards;

  m_histoQue = histoQue;
  m_mutex    = aMutex;

  m_running  = false;
  fScanAbort = false;

  strcpy (m_state, "Waiting");
  CreateHicConditions();
}


void TScan::Init()
{
  strcpy(m_state, "Running");
  time_t       t = time(0);   // get time now
  struct tm *now = localtime( & t );

  sprintf(m_config->GetfNameSuffix(), "%02d%02d%02d_%02d%02d%02d", now->tm_year - 100, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

  // Power on HIC if not yet done (PowerOn() checks if already powered)
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_hics.at(ihic)->PowerOn();
    if(!m_hics.at(ihic)->GetPowerBoard()) continue;
    m_hics.at(ihic)->GetPowerBoard()->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod());
  }

  //char dummy[10];
  //std::cout << "after power on, press enter to proceed" << std::endl;
  //std::cin >> dummy;

  TReadoutBoardMOSAIC *mosaic = dynamic_cast<TReadoutBoardMOSAIC*> (m_boards.at(0));
  if (mosaic) {
    strcpy(m_conditions.m_fwVersion, mosaic->GetFwIdString());
  }
  strcpy(m_conditions.m_swVersion, VERSION);
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_tempStart = m_hics.at(ihic)->GetTemperature();
    m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vddaStart = m_hics.at(ihic)->GetAnalogueVoltage();
    m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_iddaStart = m_hics.at(ihic)->GetIdda();
    m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_idddStart = m_hics.at(ihic)->GetIddd();

    TErrorCounter errCount;
    errCount.nEnabled      = m_hics.at(ihic)->GetNEnabledChips();
    errCount.n8b10b        = 0;
    errCount.nCorruptEvent = 0;
    errCount.nPrioEncoder  = 0;
    errCount.nTimeout      = 0;
    m_errorCounts.insert (std::pair<std::string, TErrorCounter> (m_hics.at(ihic)->GetDbId(), errCount));
  }

  for (const auto& rChip : m_chips) {
    if (rChip->GetConfig()->IsEnabled()) m_conditions.m_chipConfigStart.push_back(rChip->DumpRegisters());
  }
}


// seems the board index is not accessible anywhere.
// for the time being do like this...
int  TScan::FindBoardIndex (TAlpide *chip)
{
  for (unsigned int i = 0; i < m_boards.size(); i++) {
    if (m_boards.at(i) == chip->GetReadoutBoard()) return i;
  }
  return -1;
}


std::string TScan::FindHIC(int boardIndex, int rcv)
{
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    if (m_hics.at(i)->ContainsReceiver(boardIndex, rcv)) {
      return m_hics.at(i)->GetDbId();
    }
  }
  return std::string ("None");
}


void TScan::Terminate()
{
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_tempEnd = m_hics.at(ihic)->GetTemperature();
    m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_vddaEnd = m_hics.at(ihic)->GetAnalogueVoltage();
    m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_iddaEnd = m_hics.at(ihic)->GetIdda();
    m_conditions.m_hicConditions.at(m_hics.at(ihic)->GetDbId())->m_idddEnd = m_hics.at(ihic)->GetIddd();
  }
  strcpy(m_state, "Done");

  // reset voltage drop correction, reset chips, apply voltage drop correction to reset state
  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if(!m_hics.at(ihic)->GetPowerBoard()) continue;
    m_hics.at(ihic)->GetPowerBoard()->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod(), true);
  }

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    m_boards.at(i)->SendOpCode (Alpide::OPCODE_GRST);
  }

  for (const auto& rChip : m_chips) {
    if (rChip->GetConfig()->IsEnabled()) m_conditions.m_chipConfigEnd.push_back(rChip->DumpRegisters());
  }

  for (unsigned int ihic = 0; ihic < m_hics.size(); ihic++) {
    if(!m_hics.at(ihic)->GetPowerBoard()) continue;
    m_hics.at(ihic)->GetPowerBoard()->CorrectVoltageDrop(m_hics.at(ihic)->GetPbMod(), false);
  }

  if (m_histo) delete m_histo;
}


bool TScan::Loop (int loopIndex)
{
  if (fScanAbort) return false;     // check for abort flag first

  if ((m_step[loopIndex] > 0) && (m_value[loopIndex] < m_stop[loopIndex])) return true;  // limit check for positive steps
  if ((m_step[loopIndex] < 0) && (m_value[loopIndex] > m_stop[loopIndex])) return true;  // same for negative steps

  return false;

}


void TScan::Next (int loopIndex)
{
  m_value[loopIndex] += m_step[loopIndex];
}


void TScan::CountEnabledChips()
{

  //std::cout << "in count enabled chips, boards_size = " << m_boards.size() << ", chips_size = " << m_chips.size() << std::endl;
  for (int i = 0; i < MAXBOARDS; i++) {
    m_enabled[i] = 0;
  }
  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard ++) {
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip ++) {
      if ((m_chips.at(ichip)->GetConfig()->IsEnabled()) && (m_chips.at(ichip)->GetReadoutBoard() == m_boards.at(iboard))) {
        m_enabled[iboard] ++;
      }
    }
  }

}

void TScan::CreateScanHisto ()
{
  common::TChipIndex id;
  m_histo = new TScanHisto();

  THisto histo = CreateHisto ();

  for (unsigned int iboard = 0; iboard < m_boards.size(); iboard ++) {
    for (unsigned int ichip = 0; ichip < m_chips.size(); ichip ++) {
      if ((m_chips.at(ichip)->GetConfig()->IsEnabled()) && (m_chips.at(ichip)->GetReadoutBoard() == m_boards.at(iboard))) {
        id.boardIndex       = iboard;
        id.dataReceiver     = m_chips.at(ichip)->GetConfig()->GetParamValue("RECEIVER");
        id.chipId           = m_chips.at(ichip)->GetConfig()->GetChipId();

        m_histo->AddHisto (id, histo);
      }
    }
  }
  std::cout << "CreateHisto: generated map with " << m_histo->GetSize() << " elements" << std::endl;
  m_histo->GetChipList(m_chipList);
}


TErrorCounter TScan::GetErrorCount (std::string hicId)
{
  auto hicCount = m_errorCounts.find(hicId);

  if (hicCount != m_errorCounts.end()) {
    return hicCount->second;
  }
  else {
    std::cout << "WARNING (TScan::GetErrorCount), hic not found, returning empty counter" << std::endl;
    TErrorCounter result;
    return result;
  }
}


TMaskScan::TMaskScan (TScanConfig                   *config,
                      std::vector <TAlpide *>        chips,
                      std::vector <THic *>           hics,
                      std::vector <TReadoutBoard *>  boards,
                      std::deque<TScanHisto>        *histoQue,
                      std::mutex                    *aMutex)
  : TScan(config, chips, hics, boards, histoQue, aMutex)
{
  m_pixPerStage  = m_config->GetParamValue("PIXPERREGION");
  m_stuck.clear    ();
  m_errorCount   = {};
  FILE *fp = fopen ("DebugData.dat", "w");
  fclose(fp);
}


// check which HIC caused the timeout, i.e. did not send enough events
// called only in case a timeout occurs
void TMaskScan::FindTimeoutHics (int iboard, int *triggerCounts)
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
    if ((isOnBoard) && (nTrigs < m_nTriggers * (int)(m_hics.at(iHic)->GetNEnabledChips()))) {
      m_errorCounts.at(m_hics.at(iHic)->GetDbId()).nTimeout ++;
    }
  }
}


void TMaskScan::ConfigureMaskStage(TAlpide *chip, int istage) {
  m_row = AlpideConfig::ConfigureMaskStage (chip, m_pixPerStage, istage);
}


void TMaskScan::ReadEventData (std::vector <TPixHit> *Hits, int iboard)
{
  unsigned char buffer[1024*4000];
  int           n_bytes_data, n_bytes_header, n_bytes_trailer;
  int           itrg = 0, trials = 0;
  int           nBad = 0;
  TBoardHeader  boardInfo;
  int           nTrigPerHic[MAX_MOSAICTRANRECV];

  for (unsigned int i = 0; i < MAX_MOSAICTRANRECV; i++) {
    nTrigPerHic[i] = 0;
  }

  while (itrg < m_nTriggers * m_enabled[iboard]) {
    if (m_boards.at(iboard)->ReadEventData(n_bytes_data, buffer) == -1) { // no event available in buffer yet, wait a bit
      usleep(1000);
      trials ++;
      if (trials == 3) {
        std::cout << "Board " << iboard << ": reached 3 timeouts, giving up on this event" << std::endl;
        itrg = m_nTriggers * m_enabled[iboard];
        FindTimeoutHics(iboard, nTrigPerHic);
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
      if (!AlpideDecoder::DecodeEvent(buffer + n_bytes_header, n_bytes_chipevent, Hits, iboard, boardInfo.channel, m_errorCounts.at(FindHIC(iboard, boardInfo.channel)).nPrioEncoder, &m_stuck)) {
        std::cout << "Found bad event, length = " << n_bytes_chipevent << std::endl;
        m_errorCount.nCorruptEvent ++;
        if (FindHIC(iboard, boardInfo.channel).compare ("None") != 0) {
          m_errorCounts.at(FindHIC(iboard, boardInfo.channel)).nCorruptEvent++;
        }
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
      nTrigPerHic[boardInfo.channel] ++;
      itrg++;
    }
  }
}


void TScan::CreateHicConditions()
{
  for (unsigned int i = 0; i < m_hics.size(); i++) {
    m_conditions.AddHicConditions(m_hics.at(i)->GetDbId(), new TScanConditionsHic());
  }
}


void TScan::WriteConditions (const char *fName, THic *aHic)
{
  FILE *fp = fopen (fName, "a");

  fprintf (fp, "Firmware version: %s\n",   m_conditions.m_fwVersion);
  fprintf (fp, "Software version: %s\n\n", m_conditions.m_swVersion);

  fprintf (fp, "Temp (start): %.1f\n", m_conditions.m_hicConditions.at(aHic->GetDbId())->m_tempStart);
  fprintf (fp, "Temp (end):   %.1f\n", m_conditions.m_hicConditions.at(aHic->GetDbId())->m_tempEnd);

  fprintf (fp, "Analogue Supply Voltage (start): %.3f\n", m_conditions.m_hicConditions.at(aHic->GetDbId())->m_vddaStart);
  fprintf (fp, "Analogue Supply Voltage (end):   %.3f\n", m_conditions.m_hicConditions.at(aHic->GetDbId())->m_vddaEnd);

  fprintf (fp, "IDDD (start): %.3f A\n", m_conditions.m_hicConditions.at(aHic->GetDbId())->m_idddStart);
  fprintf (fp, "IDDD (end):   %.3f A\n", m_conditions.m_hicConditions.at(aHic->GetDbId())->m_idddEnd);
  fprintf (fp, "IDDA (start): %.3f A\n", m_conditions.m_hicConditions.at(aHic->GetDbId())->m_iddaStart);
  fprintf (fp, "IDDA (end):   %.3f A\n", m_conditions.m_hicConditions.at(aHic->GetDbId())->m_iddaEnd);

  fprintf (fp, "\n");

  fclose (fp);
}

void TScan::WriteChipRegisters(const char *fName)
{
  FILE *fp = fopen (fName, "a");

  fprintf (fp, "\n");

  fputs   ("== Chip registers (start)\n", fp);
  for (const auto& str : m_conditions.m_chipConfigStart) fprintf(fp, "%s", str.c_str());
  fputs   ("== Chip registers (end)\n", fp);
  for (const auto& str : m_conditions.m_chipConfigEnd)   fprintf(fp, "%s", str.c_str());
  fputs   ("==\n", fp);

  fclose (fp);
}


int TScanConditions::AddHicConditions (std::string hicId, TScanConditionsHic *hicCond)
{
  m_hicConditions.insert(std::pair<std::string, TScanConditionsHic*> (hicId, hicCond));

  return m_hicConditions.size();
}

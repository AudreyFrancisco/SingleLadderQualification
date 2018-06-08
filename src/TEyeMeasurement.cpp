#include "TEyeMeasurement.h"
#include "AlpideConfig.h"
#include "TReadoutBoardMOSAIC.h"

#include <exception>
#include <iostream>
#include <string.h>
#include <string>

TEyeMeasurement::TEyeMeasurement(TScanConfig *config, std::vector<TAlpide *> chips,
                                 std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                                 std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex)
{
  m_parameters           = new TEyeParameters();
  m_parameters->backBias = 0;

  ((TEyeParameters *)m_parameters)->driverStrength = config->GetParamValue("EYEDRIVER");
  ((TEyeParameters *)m_parameters)->preemphasis    = config->GetParamValue("EYEPREEMP");

  SetName(); // Display name

  // TODO: Assign proper Mosaic
  m_chipId     = 0;
  m_receiverId = 0;

  // outer loop: loop over all chips
  // TODO: can this be done in parallel on all chips?
  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = m_chips.size();

  // loops over phase and amplitude
  m_start[1] = 0;
  m_step[1]  = 1;
  m_stop[1]  = 1;

  // innermost loop
  m_start[0] = 0;
  m_step[0]  = 1;
  m_stop[0]  = 1;

  m_hMin      = m_config->GetParamValue("EYEMINX");
  m_hMax      = m_config->GetParamValue("EYEMAXX");
  m_hStep     = m_config->GetParamValue("EYESTEPX");
  m_vMin      = m_config->GetParamValue("EYEMINY");
  m_vMax      = m_config->GetParamValue("EYEMAXY");
  m_vStep     = m_config->GetParamValue("EYESTEPY");
  minPrescale = m_config->GetParamValue("EYEDEPTHMIN");
  maxPrescale = m_config->GetParamValue("EYEDEPTHMIN");
}


void TEyeMeasurement::SetName()
{
  sprintf(m_name, "EyeMeasurement D%d P%d", ((TEyeParameters *)m_parameters)->driverStrength,
          ((TEyeParameters *)m_parameters)->preemphasis);
}


bool TEyeMeasurement::SetParameters(TScanParameters *pars)
{
  TEyeParameters *ePars = dynamic_cast<TEyeParameters *>(pars);
  if (ePars) {
    std::cout << "TEyeMeasurement: Updating parameters" << std::endl;
    ((TEyeParameters *)m_parameters)->driverStrength = ePars->driverStrength;
    ((TEyeParameters *)m_parameters)->preemphasis    = ePars->preemphasis;
    SetName();
    return true;
  }
  else {
    std::cout << "TEyeMeasurement::SetParameters: Error, bad parameter type, doing nothing"
              << std::endl;
    return false;
  }
}


THisto TEyeMeasurement::CreateHisto()
{
  THisto histo("EyeDiagram", "EyeDiagram", 1 + (m_stop[0] - m_start[0]) / m_step[0], m_start[0],
               m_stop[0], 1 + (m_stop[1] - m_start[1]) / m_step[1], m_start[1], m_stop[1]);
  return histo;
}


void TEyeMeasurement::Init()
{
  CreateScanHisto();

  TScan::Init();
  TEyeParameters *params = (TEyeParameters *)m_parameters;

  for (unsigned int ichip = 0; ichip < m_chips.size(); ichip++) {
    if (!m_chips.at(ichip)->GetConfig()->IsEnabled()) continue;
    int backupDriver = m_chips.at(ichip)->GetConfig()->GetParamValue("DTUDRIVER");
    int backupPreemp = m_chips.at(ichip)->GetConfig()->GetParamValue("DTUPREEMP");
    m_chips.at(ichip)->GetConfig()->SetParamValue("DTUDRIVER", params->driverStrength);
    m_chips.at(ichip)->GetConfig()->SetParamValue("DTUPREEMP", params->preemphasis);

    AlpideConfig::Init(m_chips.at(ichip));
    AlpideConfig::BaseConfig(m_chips.at(ichip));

    // Enable PRBS (1.2 Gbps)
    uint16_t value = 0;
    value |= 1;      // Test En = 1
    value |= 1 << 1; // Interal Pattern = 1 (Prbs Mode)
    value |= 1 << 5; // Bypass8b10b
    m_chips.at(ichip)->WriteRegister(Alpide::TRegister::REG_DTU_TEST1, value);

    // restore previous settings
    m_chips.at(ichip)->GetConfig()->SetParamValue("DTUDRIVER", backupDriver);
    m_chips.at(ichip)->GetConfig()->SetParamValue("DTUPREEMP", backupPreemp);
  }

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    TReadoutBoardMOSAIC *mosaic = (TReadoutBoardMOSAIC *)m_boards.at(i);
    mosaic->setSpeedMode(Mosaic::RCV_RATE_1200);
  }
  // Maximum prescale factor (Time spend per point)
  // 10 ~= log2(1.2Gbps)/(65536*20)

  m_running = true;
}


// prepare step prepares the loop step
// loopIndex 0, 1 (innermost): set the phase and amplitude values;
// loopIndex 2: change the chip under test
void TEyeMeasurement::PrepareStep(int loopIndex)
{
  switch (loopIndex) {
  case 0: // innermost loop
    break;
  case 1: // 2nd loop
    break;
  case 2:
    m_testChip   = m_chips.at(m_value[2]);
    m_chipId     = m_testChip->GetConfig()->GetChipId();
    m_boardIndex = FindBoardIndex(m_testChip);
    sprintf(m_state, "Running %d", m_value[2]);

    std::cout << "Testing chip : " << m_testChip->GetConfig()->GetChipId() << std::endl;
    m_board = dynamic_cast<TReadoutBoardMOSAIC *>(m_boards.at(m_boardIndex));
    if (!m_board) {
      std::cout << "Error: Wrong board";
      // TODO: Exit with error
    }
    m_receiverId = m_board->GetReceiver(m_chipId);
    break;
  default:
    break;
  }
}


// any actions to be done at the ebd of the loop,
// most likely only the pushing of the histo at the end of teh outermost loop
void TEyeMeasurement::LoopEnd(int loopIndex)
{
  if (loopIndex == 2) {
    while (!(m_mutex->try_lock()))
      ;
    m_histoQue->push_back(*m_histo);
    m_mutex->unlock();
    m_histo->Clear();
  }
}


// Execute does the actual measurement
// in this case the measureemnt for one point of the eye diagram
// results are saved into m_histo as described below
void TEyeMeasurement::Execute() { runFullScan(); }


void TEyeMeasurement::Terminate()
{
  TScan::Terminate();

  for (unsigned int i = 0; i < m_boards.size(); i++) {
    TReadoutBoardMOSAIC *mosaic = (TReadoutBoardMOSAIC *)m_boards.at(i);
    mosaic->setSpeedMode(((TBoardConfigMOSAIC *)m_board->GetConfig())->GetSpeedMode());
  }
  m_running = false;
}

void TEyeMeasurement::addValue(int vOffset, int hOffset, double scanValue)
{
  common::TChipIndex idx;
  idx.boardIndex   = m_boardIndex;
  idx.chipId       = m_chipId;
  idx.dataReceiver = m_testChip->GetConfig()->GetParamValue("RECEIVER");
  // TODO: take into account step width (if != 1)
  m_histo->Set(idx, (hOffset - m_hMin) / m_hStep, (vOffset - m_vMin) / m_vStep, scanValue);
}


double TEyeMeasurement::BERmeasure(int hOffset, int vOffset)
{
  static int currPrescale = minPrescale;
  uint32_t   errorCountReg;
  uint32_t   sampleCountReg;
  uint16_t   vertOffsetReg;

  // set ES_VERT_OFFSET	bit 7:sign. bits 6-0:offset
  if (vOffset < 0)
    vertOffsetReg = ((-vOffset) & 0x7f) | 0x80;
  else
    vertOffsetReg = vOffset & 0x7f;
  m_board->WriteTransceiverDRPField(m_receiverId, ES_VERT_OFFSET, ES_VERT_OFFSET_SIZE,
                                    ES_VERT_OFFSET_OFFSET, vertOffsetReg, false);

  // set ES_HORZ_OFFSET   [11:0]  bits10-0: Phase offset (2's complement)
  uint16_t horzOffsetReg = hOffset & 0x7ff;

  // bit 11:Phase unification(0:positive 1:negative)
  if (hOffset < 0) horzOffsetReg |= 0x800;
  m_board->WriteTransceiverDRP(m_receiverId, ES_HORZ_OFFSET, horzOffsetReg, false);

  for (bool goodMeasure = false; !goodMeasure;) {
    // setup ES_PRESCALE	[15:11]. Prescale = 2**(1+reg_value)
    m_board->WriteTransceiverDRPField(m_receiverId, ES_PRESCALE, ES_PRESCALE_SIZE,
                                      ES_PRESCALE_OFFSET, currPrescale, false);

    // set ES_CONTROL[0] to start the measure run
    // Configure and run measure
    m_board->WriteTransceiverDRPField(m_receiverId, ES_CONTROL, ES_CONTROL_SIZE, ES_CONTROL_OFFSET,
                                      0x1, true);

    // poll the es_control_status[0] for max 10s
    int i;
    for (i = 10000; i > 0; i--) {
      uint32_t val;
      usleep(1000);
      m_board->ReadTransceiverDRP(m_receiverId, ES_CONTROL_STATUS, &val);
      if (val & ES_CONTROL_STATUS_DONE) break;
    }
    if (i == 0) throw std::runtime_error("Timeout reading es_control_status");

    // stop run resetting ES_CONTROL[0]
    m_board->WriteTransceiverDRPField(m_receiverId, ES_CONTROL, ES_CONTROL_SIZE, ES_CONTROL_OFFSET,
                                      0x0);

    // read es_error_count and es_sample_count
    m_board->ReadTransceiverDRP(m_receiverId, ES_ERROR_COUNT, &errorCountReg, false);
    m_board->ReadTransceiverDRP(m_receiverId, ES_SAMPLE_COUNT, &sampleCountReg, true);
    if (m_verbose) {
      printf("Ch: %d ", static_cast<int>(m_chipId));
      printf("vOffset: %d ", vOffset);
      printf("hOffset: %d ", hOffset);
      printf("errorCount: %u ", (unsigned int)errorCountReg);
      printf("sampleCount: %u ", (unsigned int)sampleCountReg);
      printf("currPrescale: %u \n", (unsigned int)currPrescale);
    }

    if (errorCountReg == 0xffff && currPrescale == 0) {
      goodMeasure = true;
    }
    else if (sampleCountReg == 0xffff && errorCountReg > 0x7fff) {
      goodMeasure = true;
    }
    else if (sampleCountReg == 0xffff && currPrescale == maxPrescale) {
      goodMeasure = true;
    }
    else if (errorCountReg == 0xffff && currPrescale > 0) {
      currPrescale--;
    }
    else if (errorCountReg <= 0x7fff) { // measure time too short
      if (currPrescale < maxPrescale) {
        currPrescale++;
      }
    }
    else {
      goodMeasure = true;
    }
  }

  return ((double)errorCountReg /
          ((double)BUS_WIDTH * (double)sampleCountReg * (1UL << (currPrescale + 1))));
}

void TEyeMeasurement::runHScan(int vOffset)
{
  int zeroCount = 0;
  // Reset the FSM
  // stop run resetting ES_CONTROL[0]
  m_board->WriteTransceiverDRPField(m_receiverId, ES_CONTROL, ES_CONTROL_SIZE, ES_CONTROL_OFFSET,
                                    0x0, true);

  int hMin = m_vMin + std::abs(m_vMin) % m_hStep;
  int hMax = m_hMax - m_hMax % m_hStep;

  for (int hOffset = hMin; hOffset < 0; hOffset += m_hStep) {
    double b = BERmeasure(hOffset, vOffset);
    addValue(vOffset, hOffset, b);
    if (b == 0.0)
      zeroCount++;
    else
      zeroCount = 0;
    if (zeroCount == MAX_ZERO_RESULTS) break;
  }

  zeroCount = 0;
  for (int hOffset = hMax; hOffset >= 0; hOffset -= m_hStep) {
    double b = BERmeasure(hOffset, vOffset);
    addValue(vOffset, hOffset, b);
    if (b == 0.0)
      zeroCount++;
    else
      zeroCount = 0;
    if (zeroCount == MAX_ZERO_RESULTS) break;
  }
}
void TEyeMeasurement::runFullScan()
{
  int vMin = m_vMin + std::abs(m_vMin) % m_vStep;
  int vMax = m_vMax - m_vMax % m_vStep;

  for (int vOffset = vMin; vOffset <= vMax; vOffset += m_vStep) {
    runHScan(vOffset);
  }
}

#include "TEyeMeasurement.h"
#include "AlpideConfig.h"
#include "TReadoutBoardMOSAIC.h"

#include <exception>
#include <iostream>
#include <string.h>
#include <string>


constexpr int MAX_HORZ_OFFSET = 128;
constexpr int MIN_HORZ_OFFSET = -128;
constexpr int MAX_VERT_OFFSET = 127;
constexpr int MIN_VERT_OFFSET = -127;
constexpr int BUS_WIDTH = 20;

constexpr int ES_QUALIFIER_0 = 0x2c;
constexpr int ES_QUALIFIER_1 = 0x2d;
constexpr int ES_QUALIFIER_2 = 0x2e;
constexpr int ES_QUALIFIER_3 = 0x2f;
constexpr int ES_QUALIFIER_4 = 0x30;
constexpr int ES_QUAL_MASK_0 = 0x31;
constexpr int ES_QUAL_MASK_1 = 0x32;
constexpr int ES_QUAL_MASK_2 = 0x33;
constexpr int ES_QUAL_MASK_3 = 0x34;
constexpr int ES_QUAL_MASK_4 = 0x35;
constexpr int ES_SDATA_MASK_0 = 0x36;
constexpr int ES_SDATA_MASK_1 = 0x37;
constexpr int ES_SDATA_MASK_2 = 0x38;
constexpr int ES_SDATA_MASK_3 = 0x39;
constexpr int ES_SDATA_MASK_4 = 0x3a;
constexpr int ES_PRESCALE = 0x3b; // bits [15:11]
constexpr int ES_PRESCALE_SIZE = 5;
constexpr int ES_PRESCALE_OFFSET = 11;
constexpr int ES_VERT_OFFSET = 0x3b; // bits [7:0]
constexpr int ES_VERT_OFFSET_SIZE = 8;
constexpr int ES_VERT_OFFSET_OFFSET = 0;
constexpr int ES_HORZ_OFFSET = 0x3c; // bits [11:0]
constexpr int ES_ERRDET_EN = 0x3d;   // bit 9
constexpr int ES_ERRDET_EN_SIZE = 1;
constexpr int ES_ERRDET_EN_OFFSET = 9;
constexpr int ES_EYE_SCAN_EN = 0x3d; // bit 8
constexpr int ES_EYE_SCAN_EN_SIZE = 1;
constexpr int ES_EYE_SCAN_EN_OFFSET = 8;
constexpr int ES_CONTROL = 0x3d; // bits [5:0]
constexpr int ES_CONTROL_SIZE = 6;
constexpr int ES_CONTROL_OFFSET = 0;
constexpr int USE_PCS_CLK_PHASE_SEL = 0x91;
constexpr int USE_PCS_CLK_PHASE_SEL_SIZE = 1;
constexpr int USE_PCS_CLK_PHASE_SEL_OFFSET = 14;

// Read only registers
constexpr int ES_ERROR_COUNT = 0x151;
constexpr int ES_SAMPLE_COUNT = 0x152;
constexpr int ES_CONTROL_STATUS = 0x153;
constexpr int ES_CONTROL_STATUS_DONE = 0x0001;
constexpr int ES_CONTROL_STATUS_FSM = 0x000e;
constexpr int ES_RDATA_4 = 0x154;
constexpr int ES_RDATA_3 = 0x155;
constexpr int ES_RDATA_2 = 0x156;
constexpr int ES_RDATA_1 = 0x157;
constexpr int ES_RDATA_0 = 0x158;
constexpr int ES_SDATA_4 = 0x159;
constexpr int ES_SDATA_3 = 0x15a;
constexpr int ES_SDATA_2 = 0x15b;
constexpr int ES_SDATA_1 = 0x15c;
constexpr int ES_SDATA_0 = 0x15d;


TEyeMeasurement::TEyeMeasurement(TScanConfig *config, std::vector<TAlpide *> chips,
                                 std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                                 std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex)
{
  strcpy(m_name, "Dctrl Measurement"); // Display name

  // TODO: Assign proper Mosaic

  // outer loop: loop over all chips
  // TODO: can this be done in parallel on all chips?
  m_start[2] = 0;
  m_step[2] = 1;
  m_stop[2] = m_chips.size();

  // loops over phase and amplitude
  // TODO: define step sizes and intervals
  m_start[1] = MIN_VERT_OFFSET;
  m_step[1] = 1; // TODO:
  m_stop[1] = MAX_VERT_OFFSET;

  // innermost loop
  m_start[0] = MIN_HORZ_OFFSET;
  m_step[0] = 1; // TODO:
  m_stop[0] = MAX_HORZ_OFFSET;

  // Other Parameters TODO:
  m_min_prescale = 0;
  m_max_prescale = 6; // max 0.1s at 1.2 Gbps

  // NOT supported for now (needs to change looping behaviour)
  m_max_zero_results = 0; // Max number of consecutive zero results

  CreateScanHisto();
}


THisto TEyeMeasurement::CreateHisto()
{
  THisto histo("EyeDiagram", "EyeDiagram", 1 + (m_stop[0] - m_start[0]) / m_step[0], m_start[0],
               m_stop[0], 1 + (m_stop[1] - m_start[1]) / m_step[1], m_start[1], m_stop[1]);
  return histo;
}


void TEyeMeasurement::Init()
{
  TScan::Init();

  // initialisations of chips and MOSAIC

  // Parameters to set up

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
    // Reset the FSM
    // stop run resetting ES_CONTROL[0]
    m_board->WriteTransceiverDRPField(m_testChip->GetConfig()->GetChipId(), ES_CONTROL,
                                      ES_CONTROL_SIZE, ES_CONTROL_OFFSET, 0x0, true);
    break;
  case 1: // 2nd loop

    break;
  case 2:
    m_testChip = m_chips.at(m_value[2]);
    m_boardIndex = FindBoardIndex(m_testChip);
    sprintf(m_state, "Running %d", m_value[2]);

    m_board = dynamic_cast<TReadoutBoardMOSAIC *>(m_boards.at(m_boardIndex));
    if (!m_board) {
      std::cout << "Error: Wrong board";
      // TODO: Exit with error
    }
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
void TEyeMeasurement::Execute()
{
  int hOffset = m_value[0];
  int vOffset = m_value[1];

  uint32_t errorCountReg;
  uint32_t sampleCountReg;
  uint16_t vertOffsetReg;

  // set ES_VERT_OFFSET	bit 7:sign. bits 6-0:offset
  if (vOffset < 0)
    vertOffsetReg = ((-vOffset) & 0x7f) | 0x80;
  else
    vertOffsetReg = vOffset & 0x7f;
  m_board->WriteTransceiverDRPField(m_testChip->GetConfig()->GetChipId(), ES_VERT_OFFSET,
                                    ES_VERT_OFFSET_SIZE, ES_VERT_OFFSET_OFFSET, vertOffsetReg,
                                    false);

  // set ES_HORZ_OFFSET   [11:0]  bits10-0: Phase offset (2's complement)
  uint16_t horzOffsetReg = hOffset & 0x7ff;

  // bit 11:Phase unification(0:positive 1:negative)
  if (hOffset < 0) horzOffsetReg |= 0x800;
  m_board->WriteTransceiverDRP(m_testChip->GetConfig()->GetChipId(), ES_HORZ_OFFSET, horzOffsetReg,
                               false);

  for (bool goodMeasure = false; !goodMeasure;) {
    // setup ES_PRESCALE	[15:11]. Prescale = 2**(1+reg_value)
    m_board->WriteTransceiverDRPField(m_testChip->GetConfig()->GetChipId(), ES_PRESCALE,
                                      ES_PRESCALE_SIZE, ES_PRESCALE_OFFSET, m_current_prescale,
                                      false);

    // set ES_CONTROL[0] to start the measure run
    // Configure and run measure
    m_board->WriteTransceiverDRPField(m_testChip->GetConfig()->GetChipId(), ES_CONTROL,
                                      ES_CONTROL_SIZE, ES_CONTROL_OFFSET, 0x1, true);

    // poll the es_control_status[0] for max 10s
    int i;
    for (i = 10000; i > 0; i--) {
      uint32_t val;
      usleep(1000);
      m_board->ReadTransceiverDRP(m_testChip->GetConfig()->GetChipId(), ES_CONTROL_STATUS, &val);
      if (val & ES_CONTROL_STATUS_DONE) break;
    }
    if (i == 0) throw std::runtime_error("Timeout reading es_control_status");

    // stop run resetting ES_CONTROL[0]
    m_board->WriteTransceiverDRPField(m_testChip->GetConfig()->GetChipId(), ES_CONTROL,
                                      ES_CONTROL_SIZE, ES_CONTROL_OFFSET, 0x0);

    // read es_error_count and es_sample_count
    m_board->ReadTransceiverDRP(m_testChip->GetConfig()->GetChipId(), ES_ERROR_COUNT,
                                &errorCountReg, false);
    m_board->ReadTransceiverDRP(m_testChip->GetConfig()->GetChipId(), ES_SAMPLE_COUNT,
                                &sampleCountReg, true);

    if (errorCountReg == 0xffff && m_current_prescale == 0) {
      goodMeasure = true;
    } else if (sampleCountReg == 0xffff && errorCountReg > 0x7fff) {
      goodMeasure = true;
    } else if (sampleCountReg == 0xffff && m_current_prescale == m_max_prescale) {
      goodMeasure = true;
    } else if (errorCountReg == 0xffff && m_current_prescale > 0) {
      m_current_prescale--;
    } else if (errorCountReg <= 0x7fff) { // measure time too short
      if (m_current_prescale < m_max_prescale) {
        m_current_prescale++;
      }
    } else {
      goodMeasure = true;
    }
  }

  double scanValue = ((double)errorCountReg / ((double)BUS_WIDTH * (double)sampleCountReg *
                                               (1UL << (m_current_prescale + 1))));

  // TODO: write scanValue to histogram
  std::cout << "X " << hOffset << ", Y " << vOffset << ", value: " << scanValue << "\n";
}


void TEyeMeasurement::Terminate()
{
  TScan::Terminate();
  m_running = false;
}

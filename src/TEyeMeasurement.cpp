#include "TEyeMeasurement.h"
#include "AlpideConfig.h"
#include <exception>
#include <iostream>
#include <string.h>
#include <string>

TEyeMeasurement::TEyeMeasurement(TScanConfig *config, std::vector<TAlpide *> chips,
                                 std::vector<THic *> hics, std::vector<TReadoutBoard *> boards,
                                 std::deque<TScanHisto> *histoQue, std::mutex *aMutex)
    : TScan(config, chips, hics, boards, histoQue, aMutex)
{
  strcpy(m_name, "Dctrl Measurement"); // Display name

  // outer loop: loop over all chips
  // TODO: can this be done in parallel on all chips?
  m_start[2] = 0;
  m_step[2]  = 1;
  m_stop[2]  = m_chips.size();

  // loops over phase and amplitude
  // TODO define step sizes and intervals
  m_start[1] = 0;
  m_step[1]  = 1;
  m_stop[1]  = 16;

  // innermost loop
  m_start[0] = 0;
  m_step[0]  = 1;
  m_stop[0]  = 16;

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
    m_boardIndex = FindBoardIndex(m_testChip);
    sprintf(m_state, "Running %d", m_value[2]);
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
void TEyeMeasurement::Execute() {}


void TEyeMeasurement::Terminate()
{
  TScan::Terminate();
  m_running = false;
}

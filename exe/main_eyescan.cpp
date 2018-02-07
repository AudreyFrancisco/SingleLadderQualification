// Template to prepare standard test routines
// ==========================================
//
// After successful call to initSetup() the elements of the setup are accessible
// in the two vectors
//   - fBoards: vector of readout boards (setups implemented here have only 1
// readout board, i.e. fBoards.at(0)
//   - fChips:  vector of chips, depending on setup type 1, 9 or 14 elements
//
// In order to have a generic scan, which works for single chips as well as for
// staves and modules,
// all chip accesses should be done with a loop over all elements of the chip
// vector.
// (see e.g. the configureChip loop in main)
// Board accesses are to be done via fBoards.at(0);
// For an example how to access board-specific functions see the power off at
// the end of main.
//
// The functions that should be modified for the specific test are
// configureChip() and main()

#include <fstream>
#include <exception>

#include <unistd.h>
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

// See table 4.20 of 7Series_GTP_Transceivers
// ES_HORZ_OFFSET for data rate Qrtr ( RXOUT_DIV = 4 )
#define MAX_HORZ_OFFSET 128
#define MIN_HORZ_OFFSET -128

#define MAX_VERT_OFFSET 127
#define MIN_VERT_OFFSET -127

// Bus width at Trnsceiver output
#define BUS_WIDTH 20

// Maximum prescale factor
// 10 ~= log2(1.2Gbps)/(65536*20)
// #define MAX_PRESCALE	9		// max one measure per second at 1.2
// Gbps
#define MAX_PRESCALE 6 // max 0.1s at 1.2 Gbps
// Max number of consecutive zero results
#define MAX_ZERO_RESULTS 3

// Todo: Integrate into Class structure
namespace eyescan {
enum GTP_DRP_ADDR {
  ES_QUALIFIER_0 = 0x2c,
  ES_QUALIFIER_1 = 0x2d,
  ES_QUALIFIER_2 = 0x2e,
  ES_QUALIFIER_3 = 0x2f,
  ES_QUALIFIER_4 = 0x30,
  ES_QUAL_MASK_0 = 0x31,
  ES_QUAL_MASK_1 = 0x32,
  ES_QUAL_MASK_2 = 0x33,
  ES_QUAL_MASK_3 = 0x34,
  ES_QUAL_MASK_4 = 0x35,
  ES_SDATA_MASK_0 = 0x36,
  ES_SDATA_MASK_1 = 0x37,
  ES_SDATA_MASK_2 = 0x38,
  ES_SDATA_MASK_3 = 0x39,
  ES_SDATA_MASK_4 = 0x3a,
  ES_PRESCALE = 0x3b, // bits [15:11]
  ES_PRESCALE_SIZE = 5,
  ES_PRESCALE_OFFSET = 11,
  ES_VERT_OFFSET = 0x3b, // bits [7:0]
  ES_VERT_OFFSET_SIZE = 8,
  ES_VERT_OFFSET_OFFSET = 0,
  ES_HORZ_OFFSET = 0x3c, // bits [11:0]
  ES_ERRDET_EN = 0x3d,   // bit 9
  ES_ERRDET_EN_SIZE = 1,
  ES_ERRDET_EN_OFFSET = 9,
  ES_EYE_SCAN_EN = 0x3d, // bit 8
  ES_EYE_SCAN_EN_SIZE = 1,
  ES_EYE_SCAN_EN_OFFSET = 8,
  ES_CONTROL = 0x3d, // bits [5:0]
  ES_CONTROL_SIZE = 6,
  ES_CONTROL_OFFSET = 0,
  USE_PCS_CLK_PHASE_SEL = 0x91,
  USE_PCS_CLK_PHASE_SEL_SIZE = 1,
  USE_PCS_CLK_PHASE_SEL_OFFSET = 14,

  // Read only registers
  ES_ERROR_COUNT = 0x151,
  ES_SAMPLE_COUNT = 0x152,
  ES_CONTROL_STATUS = 0x153,
  ES_CONTROL_STATUS_DONE = 0x0001,
  ES_CONTROL_STATUS_FSM = 0x000e,
  ES_RDATA_4 = 0x154,
  ES_RDATA_3 = 0x155,
  ES_RDATA_2 = 0x156,
  ES_RDATA_1 = 0x157,
  ES_RDATA_0 = 0x158,
  ES_SDATA_4 = 0x159,
  ES_SDATA_3 = 0x15a,
  ES_SDATA_2 = 0x15b,
  ES_SDATA_1 = 0x15c,
  ES_SDATA_0 = 0x15d
};

class EyeScan {
public:
  EyeScan(TReadoutBoardMOSAIC &board, size_t chipId, std::string filename, int hStep=1, int vStep=1, bool verbose=false)
    : m_board(board), m_chipId(chipId), hStep(hStep), vStep(vStep), m_verbose(verbose) {
    dataFile.open(filename);
    dataFile.setf(std::ios_base::scientific);
  }

  double BERmeasure(int hOffset, int vOffset, int maxPrescale) {
    static int currPrescale = 0;
    uint32_t errorCountReg;
    uint32_t sampleCountReg;
    uint16_t vertOffsetReg;

    // set ES_VERT_OFFSET	bit 7:sign. bits 6-0:offset
    if (vOffset < 0)
      vertOffsetReg = ((-vOffset) & 0x7f) | 0x80;
    else
      vertOffsetReg = vOffset & 0x7f;
    m_board.WriteTransceiverDRPField(m_chipId, ES_VERT_OFFSET,
                                     ES_VERT_OFFSET_SIZE, ES_VERT_OFFSET_OFFSET,
                                     vertOffsetReg, false);

    // set ES_HORZ_OFFSET   [11:0]  bits10-0: Phase offset (2's complement)
    uint16_t horzOffsetReg = hOffset & 0x7ff;

    // bit 11:Phase unification(0:positive 1:negative)
    if (hOffset < 0)
      horzOffsetReg |= 0x800;
    m_board.WriteTransceiverDRP(m_chipId, ES_HORZ_OFFSET, horzOffsetReg, false);

    for (bool goodMeasure = false; !goodMeasure;) {
      // setup ES_PRESCALE	[15:11]. Prescale = 2**(1+reg_value)
      m_board.WriteTransceiverDRPField(m_chipId, ES_PRESCALE, ES_PRESCALE_SIZE,
                                       ES_PRESCALE_OFFSET, currPrescale, false);

      // set ES_CONTROL[0] to start the measure run
      // Configure and run measure
      m_board.WriteTransceiverDRPField(m_chipId, ES_CONTROL, ES_CONTROL_SIZE,
                                       ES_CONTROL_OFFSET, 0x1, true);

      // poll the es_control_status[0] for max 10s
      int i;
      for (i = 10000; i > 0; i--) {
        uint32_t val;
        usleep(1000);
        m_board.ReadTransceiverDRP(m_chipId, ES_CONTROL_STATUS, &val);
        if (val & ES_CONTROL_STATUS_DONE)
          break;
      }
      if (i == 0)
        throw std::runtime_error("Timeout reading es_control_status");

      // stop run resetting ES_CONTROL[0]
      m_board.WriteTransceiverDRPField(m_chipId, ES_CONTROL, ES_CONTROL_SIZE,
                                       ES_CONTROL_OFFSET, 0x0);

      // read es_error_count and es_sample_count
      m_board.ReadTransceiverDRP(m_chipId, ES_ERROR_COUNT, &errorCountReg,
                                 false);
      m_board.ReadTransceiverDRP(m_chipId, ES_SAMPLE_COUNT, &sampleCountReg,
                                 true);
      if(m_verbose) {
	printf("Ch: %d ", static_cast<int>(m_chipId));
	printf("vOffset: %d ", vOffset);
        printf("hOffset: %d ", hOffset);
        printf("errorCount: %u ", (unsigned int)errorCountReg);
        printf("sampleCount: %u ", (unsigned int)sampleCountReg);
        printf("currPrescale: %u \n", (unsigned int)currPrescale);
      }

      if (errorCountReg == 0xffff && currPrescale == 0) {
        goodMeasure = true;
      } else if (sampleCountReg == 0xffff && errorCountReg > 0x7fff) {
        goodMeasure = true;
      } else if (sampleCountReg == 0xffff && currPrescale == maxPrescale) {
        goodMeasure = true;
      } else if (errorCountReg == 0xffff && currPrescale > 0) {
        currPrescale--;
      } else if (errorCountReg <= 0x7fff) { // measure time too short
        if (currPrescale < maxPrescale) {
          currPrescale++;
        }
      } else {
        goodMeasure = true;
      }
    }

    return ((double)errorCountReg /
            ((double)BUS_WIDTH * (double)sampleCountReg *
             (1UL << (currPrescale + 1))));
  }

  void runHScan(double *ber, int vOffset) {
    double b;
    int zeroCount = 0;

    std::cout << "Start Scan at " << vOffset << endl;

    // Reset the FSM
    // stop run resetting ES_CONTROL[0]
    m_board.WriteTransceiverDRPField(m_chipId, ES_CONTROL, ES_CONTROL_SIZE,
                                     ES_CONTROL_OFFSET, 0x0, true);

    int hMin = MIN_HORZ_OFFSET + std::abs(MIN_HORZ_OFFSET) % hStep;
    int hMax = MAX_HORZ_OFFSET - MAX_HORZ_OFFSET % hStep;

    for (int hOffset = hMin; hOffset < 0; hOffset+= hStep) {
      size_t idx = (hOffset-hMin)/hStep;
      b = ber[idx] =
          BERmeasure(hOffset, vOffset, MAX_PRESCALE);
      //	printf("UI: %0.3f BER: %e\n",
      //(float)hOffset/(float)(MAX_HORZ_OFFSET*2),
      // b);
      if (b == 0.0)
        zeroCount++;
      else
        zeroCount = 0;
      if (zeroCount == MAX_ZERO_RESULTS)
        break;
    }

    zeroCount = 0;
    for (int hOffset = hMax; hOffset >= 0; hOffset-= hStep) {
      size_t idx = (hOffset-hMin)/hStep;
      b = ber[idx] =
          BERmeasure(hOffset, vOffset, MAX_PRESCALE);
      //	printf("UI: %0.3f BER: %e\n",
      //(float)hOffset/(float)(MAX_HORZ_OFFSET*2),
      // b);
      if (b == 0.0)
        zeroCount++;
      else
        zeroCount = 0;
      if (zeroCount == MAX_ZERO_RESULTS)
        break;
    }
  }

  void runFullScan() {
    int hMin = MIN_HORZ_OFFSET + std::abs(MIN_HORZ_OFFSET) % hStep;
    int hMax = MAX_HORZ_OFFSET - MAX_HORZ_OFFSET % hStep;
    size_t hPoints = (hMax-hMin)/hStep + 1;

    const int resSize = hPoints;
    std::vector<double> ber(resSize);
    const double minMeasure =
        ((double)1.0 / ((double)BUS_WIDTH * (double)0xffff *
                        (double)(1UL << (MAX_PRESCALE + 1))));

    /*
      for (int i=MIN_HORZ_OFFSET; i<=MAX_HORZ_OFFSET; i++)
      printf("%0.3f ", (float)i/(float)(MAX_HORZ_OFFSET*2));
      printf("\n");
    */

    int vMin = MIN_VERT_OFFSET + std::abs(MIN_VERT_OFFSET) % vStep;
    int vMax = MAX_VERT_OFFSET - MAX_VERT_OFFSET % vStep;

    for (int vOffset = vMin; vOffset <= vMax; vOffset+= vStep) {
      for (int i = 0; i < resSize; i++)
        ber[i] = 0.0;
      runHScan(ber.data(), vOffset);
      for (int i = 0; i < resSize; i++)
        if (ber[i] == 0.0)
          ber[i] = minMeasure;

      for (int i = 0; i < resSize; i++)
        dataFile << " " << ber[i];
      dataFile << endl;
    }
  }

  void runBathTubScan() {
    const int resSize = 2 * MAX_HORZ_OFFSET + 1;
    double ber[resSize];
    const double minMeasure =
        ((double)1.0 / ((double)BUS_WIDTH * (double)0xffff *
                        (double)(1UL << (MAX_PRESCALE + 1))));

    /*
      for (int i=MIN_HORZ_OFFSET; i<=MAX_HORZ_OFFSET; i++)
      printf("%0.3f ", (float)i/(float)(MAX_HORZ_OFFSET*2));
      printf("\n");
    */

    for (int i = 0; i < resSize; i++)
      ber[i] = 0.0;
    runHScan(ber, 0);
    for (int i = 0; i < resSize; i++)
      if (ber[i] == 0.0)
        ber[i] = minMeasure;

    for (int i = 0; i < resSize; i++)
      dataFile << " " << ber[i];
    dataFile << endl;
  }

private:
  TReadoutBoardMOSAIC &m_board;
  size_t m_chipId;
  std::ofstream dataFile;
  int hStep;
  int vStep;
  bool m_verbose;
};

}

TBoardType fBoardType;
std::vector<TReadoutBoard *> fBoards;
std::vector<TAlpide *> fChips;
TConfig *fConfig;

int fErrCount0;
int fErrCount5;
int fErrCountf;

int fEnabled = 0; // variable to count number of enabled chips; leave at 0

int fTotalErr;

int configureChip(TAlpide *chip) {
  AlpideConfig::Init(chip);
  AlpideConfig::BaseConfig(chip);

  // Enable PRBS (1.2 Gbps)
  uint16_t value = 0;
  value |= 1;      // Test En = 1
  value |= 1 << 1; // Interal Pattern = 1 (Prbs Mode)
  value |= 1 << 5; // Bypass8b10b
  chip->WriteRegister(Alpide::TRegister::REG_DTU_TEST1, value);

  return 0;
}

int main(int argc, char **argv) {

  decodeCommandParameters(argc, argv);

  initSetup(fConfig, &fBoards, &fBoardType, &fChips);

  if (fBoards.size()) {

    for (const auto &rBoard : fBoards) {
      rBoard->SendOpCode(Alpide::OPCODE_GRST);
      rBoard->SendOpCode(Alpide::OPCODE_PRST);
    }

    for (const auto &rChip : fChips) {
      if (!rChip->GetConfig()->IsEnabled())
        continue;
      configureChip(rChip);
    }

    // Setup mosaic board for Eyescan
    TReadoutBoardMOSAIC *myMOSAIC =
        dynamic_cast<TReadoutBoardMOSAIC *>(fBoards.at(0));

    if (!myMOSAIC) {
      std::cout << "Eyescan only implemented for MOSAIC. Exit\n";
      return 1;
    }

    myMOSAIC->StartRun();

    // Eyescan
    bool verbose = true;
    for (const auto &rChip : fChips) {
      if (!rChip->GetConfig()->IsEnabled())
        continue;
      std::string filename("eye_ch");
      int chipId = rChip->GetConfig()->GetChipId();
      filename += std::to_string(chipId);
      filename += ".dat";
      eyescan::EyeScan scan(*myMOSAIC, chipId, filename,4,4,verbose);
      scan.runFullScan();
    }
  }

  return 0;
}

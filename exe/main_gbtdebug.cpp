#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "SetupHelpers.h"
#include "TAlpide.h"
#include "TReadoutBoardRU.h"
#include <algorithm>
#include <chrono>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unistd.h>

TBoardType                   fBoardType;
std::vector<TReadoutBoard *> fBoards;
std::vector<TAlpide *>       fChips;
TConfig *                    fConfig;

int configureFromu(TAlpide *chip)
{

  uint16_t data = (1 << 4) | (1 << 6);
  chip->WriteRegister(Alpide::REG_FROMU_CONFIG1,
                      data); // fromu config 1: digital pulsing (put to 0x20 for analogue)
  chip->WriteRegister(
      Alpide::REG_FROMU_CONFIG2,
      chip->GetConfig()->GetParamValue("STROBEDURATION")); // fromu config 2: strobe length
  chip->WriteRegister(Alpide::REG_FROMU_PULSING1,
                      chip->GetConfig()->GetParamValue("STROBEDELAYCHIP")); // fromu pulsing 1:
                                                                            // delay pulse - strobe
                                                                            // (not used here, since
                                                                            // using external
                                                                            // strobe)
  chip->WriteRegister(
      Alpide::REG_FROMU_PULSING2,
      chip->GetConfig()->GetParamValue("PULSEDURATION")); // fromu pulsing 2: pulse length
  return 0;
}

void InitChips(int numUnmaskedPixels)
{

  // This will mask all but the number of pixels you select between 0 and 1024 (a full column, just
  // for quick testing purposes)

  for (unsigned int i = 0; i < fChips.size(); i++) {
    auto ch     = fChips.at(i);
    auto chipId = fChips.at(i)->GetConfig()->GetChipId();

    std::cout << "Configure chip " << (int)chipId << " for readout\n";

    AlpideConfig::BaseConfig(ch);
    AlpideConfig::Init(ch);

    configureFromu(ch);
    AlpideConfig::ConfigureCMU(ch);

    if (numUnmaskedPixels == 0)
      AlpideConfig::BaseConfigMask(ch);
    else {
      for (int i = 0; i < numUnmaskedPixels; i++) {
        AlpideConfig::WritePixRegSingle(ch, Alpide::PIXREG_MASK, false, 0, i);
        AlpideConfig::WritePixRegSingle(ch, Alpide::PIXREG_SELECT, true, 0, i);
      }
    }
  }
}

void GbtDebug()
{

  TReadoutBoardRUv1 *theBoard = dynamic_cast<TReadoutBoardRUv1 *>(fBoards.at(0));
  theBoard->ResetAllCounters();
  theBoard->Trigger(100);
  usleep(100000);

  static UsbDev::DataBuffer buffer;
  UsbDev::DataBuffer        babybuffer;

  while (theBoard->readFromPort(TReadoutBoardRUv1::EP_DATA0_IN,
                                TReadoutBoardRUv1::EVENT_DATA_READ_CHUNK, babybuffer) > 0)
    buffer.insert(buffer.end(), babybuffer.begin(), babybuffer.end());


  int SOP          = 0;
  int EOP          = 0;
  int statusEnd    = 0;
  int channelWords = 0;
  int statusStart  = 0;
  int timeout      = 0;
  int RDH          = 0;
  theBoard->LatchAllCounters();
  uint16_t dp_fifo_full = theBoard->datapathmon->Read(17 * 8 + 11 + 2);
  // uint16_t pack_fifo_full = theBoard->gbt_packer_monitor_gth->Read(12);
  uint16_t pack_fifo_overflow = theBoard->gbt_packer_monitor_gth->Read(13);
  uint16_t lane_timeout       = theBoard->datapathmon->Read(17 * 8 + 12 + 2);


  uint16_t usb_words_msb     = theBoard->usb_if->Read(1);
  uint16_t usb_words_lsb     = theBoard->usb_if->Read(2);
  size_t   usb_if_bytes      = ((usb_words_lsb) | (usb_words_msb << 8)) * 4;
  uint16_t usb_dp2_overflow  = theBoard->usb_if->Read(3);
  uint16_t decode_errors     = theBoard->datapathmon->Read(17 * 8 + 2 + 2);
  uint16_t event_errors      = theBoard->datapathmon->Read(17 * 8 + 3 + 2);
  uint16_t event_count       = theBoard->datapathmon->Read(17 * 8 + 0 + 2);
  uint16_t elastic_overflow  = (theBoard->datapathmon->Read(17 * 8 + 10 + 2) & 0xff);
  uint16_t elastic_underflow = ((theBoard->datapathmon->Read(17 * 8 + 10 + 2) >> 8) & 0xff);

  if (event_errors != 0) std::cout << "WARNING: EVENT ERRORS: " << event_errors << std::endl;
  if (event_count != 100)
    std::cout << "WARNING: EVENT COUNTER MISMATCH, EXPECTED 100, GOT: " << event_count << std::endl;
  if (decode_errors != 0) std::cout << "WARNING: DECODE ERRORS: " << decode_errors << std::endl;
  if (pack_fifo_overflow != 0)
    std::cout << "WARNING: GBT PACKER FIFO OVERFLOW: " << pack_fifo_overflow << std::endl;
  if (lane_timeout != 0)
    std::cout << "WARNING: LANE PACKAGER LANE TIMEOUT: " << (lane_timeout & 0xff)
              << " AND GBT PACKER LANE TIMEOUT: " << ((lane_timeout >> 8) & 0xff) << std::endl;
  if (buffer.size() != usb_if_bytes)
    std::cout << "WARNING: MISMATCH IN BYTES RECEIVED BY USB IF AND BYTES READ \n";
  if (usb_dp2_overflow != 0) std::cout << "WARNING: OVERFLOW ON USB_IF DP2 FIFO \n";
  if (elastic_overflow != 0)
    std::cout << "WARNING: ELASTIC BUFFER OVERFLOW: " << elastic_overflow << std::endl;
  if (elastic_underflow != 0)
    std::cout << "WARNINGISH: ELASTIC BUFFER UNDERFLOW: " << elastic_underflow << std::endl;
  if (dp_fifo_full != 0)
    std::cout << "WARNING: DP FIFO FULL: " << (dp_fifo_full & 0xff) << 8
              << " AND FIFO OVERFLOW: " << ((dp_fifo_full >> 8) & 0xff) << 8 << std::endl;
  std::vector<UsbDev::DataBuffer> gbt_frames;
  for (size_t i = 0; i < buffer.size(); i += 12) {

    uint8_t lane        = buffer[i + 1];
    uint8_t timeoutStat = buffer[i];
    if (lane == 0x10) {
      int                j = 0;
      UsbDev::DataBuffer gbtFrame;
      while (buffer[i + 1 + j] != 0x20) {
        gbtFrame.push_back(buffer[j + i + 1]);
        gbtFrame.push_back(buffer[j + i]);
        gbtFrame.push_back(buffer[j + i + 7]);
        gbtFrame.push_back(buffer[j + i + 6]);
        gbtFrame.push_back(buffer[j + i + 5]);
        gbtFrame.push_back(buffer[j + i + 4]);
        gbtFrame.push_back(buffer[j + i + 11]);
        gbtFrame.push_back(buffer[j + i + 10]);
        gbtFrame.push_back(buffer[j + i + 9]);
        gbtFrame.push_back(buffer[j + i + 8]);
        j += 12;
      }

      // gotta get the EOP duh
      gbtFrame.push_back(buffer[j + i + 1]);
      gbtFrame.push_back(buffer[j + i]);
      gbtFrame.push_back(buffer[j + i + 7]);
      gbtFrame.push_back(buffer[j + i + 6]);
      gbtFrame.push_back(buffer[j + i + 5]);
      gbtFrame.push_back(buffer[j + i + 4]);
      gbtFrame.push_back(buffer[j + i + 11]);
      gbtFrame.push_back(buffer[j + i + 10]);
      gbtFrame.push_back(buffer[j + i + 9]);
      gbtFrame.push_back(buffer[j + i + 8]);

      gbt_frames.push_back(gbtFrame);
      SOP++;
    }
    if (lane == 0x0) RDH++;
    if (lane == 0xe0) statusStart++;
    if (lane == 0x09) channelWords++;
    if (lane == 0xf0) statusEnd++;
    if (lane == 0x20) EOP++;
    if ((lane == 0xf0) && (timeoutStat != 0x01)) timeout++;
  }
  std::cout << SOP << " SOP" << std::endl;
  std::cout << (int)RDH / 4 << " RAW DATA HEADERS" << std::endl;
  std::cout << statusStart << " STATUS START" << std::endl;
  std::cout << channelWords << " CHANNEL WORDS" << std::endl;
  std::cout << statusEnd << " STATUS END" << std::endl;
  std::cout << EOP << " EOP" << std::endl;
  std::cout << timeout << " TIMEOUT" << std::endl;
  std::cout << std::hex;

  // FILE OUTPUT
  ofstream textout;
  textout.open("empty_frames_no_divide_nopack.txt");
  textout << std::hex;
  for (size_t i = 0; i < gbt_frames.size(); i++) {
    for (size_t j = 0; j < gbt_frames.at(i).size(); j++) {
      textout << std::setfill('0') << std::setw(2) << (int)gbt_frames[i][j] << " ";
      if (j % 10 == 9) textout << "\n";
    }
  }
  textout.close();

  ofstream textout_div;
  textout_div.open("empty_frames_divide_nopack.txt");
  textout_div << std::hex;
  for (size_t i = 0; i < gbt_frames.size(); i++) {
    textout_div << std::dec << "............FRAME " << i << ".............\n" << std::hex;
    for (size_t j = 0; j < gbt_frames.at(i).size(); j++) {
      if (j % 10 == 0) textout_div << ".";
      textout_div << std::setfill('0') << std::setw(2) << (int)gbt_frames[i][j] << " ";
      if (j % 10 == 9) textout_div << ".\n";
    }
    textout_div << "................................\n";
  }
  textout_div.close();

  std::vector<int> gbt_frame_size;
  for (size_t i = 0; i < gbt_frames.size(); i++) {
    gbt_frame_size.push_back(gbt_frames.at(i).size());
  }

  sort(gbt_frame_size.begin(), gbt_frame_size.end());
  int number = gbt_frame_size[0];
  int max    = gbt_frame_size[0];
  int min    = gbt_frame_size[0];
  int mode   = 0;
  for (size_t i = 1, countMode = 1, count = 1; i < gbt_frame_size.size(); ++i) {
    if (gbt_frame_size[i] == number) ++countMode;
    if (countMode > count) {
      count = countMode;
      mode  = number;
    }
    if (gbt_frame_size[i] > max) max = gbt_frame_size[i];
    if (gbt_frame_size[i] < min) min = gbt_frame_size[i];
    number = gbt_frame_size[i];
  }
  int perfectFrameIndex = 0;

  for (size_t i = 0; i < gbt_frames.size(); i++) {
    if (gbt_frames.at(i).size() == (size_t)mode) {
      perfectFrameIndex = i;
    }
  }

  for (size_t i = 0; i < gbt_frames.at(perfectFrameIndex).size(); i++) {
    std::cout << std::setfill('0') << std::setw(2) << (int)gbt_frames[perfectFrameIndex][i] << " ";
    if (i % 10 == 9) std::cout << "\n";
  }
  std::cout << ".................PERFECT FRAME...............\n";

  for (size_t i = 0; i < gbt_frames.size(); i++) {
    if (gbt_frames.at(i).size() != (size_t)mode) {
      if (gbt_frames.at(i).size() > 80) {
        for (size_t j = 0; j < gbt_frames.at(i).size(); j += 10) {

          for (int k = 0; k < 10; k++) {
            std::cout << std::setfill('0') << std::setw(2) << (int)gbt_frames[i][k + j] << " ";
          }
          std::cout << "\n";
          if (gbt_frames[i][j] == 0x20)
            std::cout << std::dec << (int)gbt_frames.at(i).size() / 10 << " READ SIZE "
                      << (int)gbt_frames[i][j + 9] << " SEND SIZE \n"
                      << std::hex;
        }

        std::cout << std::dec << ".......UGLY FRAME..." << i << "..........." << std::hex
                  << std::endl;
      }
    }
  }

  theBoard->CleanUp();
}

int main(int argc, char **argv)
{

  std::cout << "Decode Command parameters\n";
  decodeCommandParameters(argc, argv);

  std::cout << "Create BoardConfig+Board\n";
  initSetup(fConfig, &fBoards, &fBoardType, &fChips);
  if (fBoards.at(0)->GetConfig()->GetBoardType() == boardRUv1) {
    TReadoutBoardRUv1 *theBoard = dynamic_cast<TReadoutBoardRUv1 *>(fBoards.at(0));
    theBoard->Initialize(1200);
  }
  InitChips(10); // argument is number of pixels to leave unmasked, "stress" readout

  // prepare for trigger/readout
  for (unsigned int i = 0; i < fBoards.size(); i++) {
    fBoards.at(i)->StartRun();
    fBoards.at(i)->SetTriggerConfig(true, true, 1000, 1000);
  }

  GbtDebug();

  std::cout << "TEST COMPLETE! \n";

  return 0;
}

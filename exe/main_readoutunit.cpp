#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "BoardDecoder.h"
#include "SetupHelpers.h"
#include "TAlpide.h"
#include "TReadoutBoardRU.h"
#include <chrono>
#include <exception>
#include <fstream>
#include <iostream>
#include <thread>
#include <unistd.h>

TBoardType                   fBoardType;
std::vector<TReadoutBoard *> fBoards;
std::vector<TAlpide *>       fChips;
TConfig *                    fConfig;
int                          numEnabled = 0;


void TestI2C()
{

  // This will write a config from an XML file (in GBTX_Configs) to GBTX_0 and read back to verify

  for (unsigned int i = 0; i < fBoards.size(); i++) {
    if (fBoards.at(i)->GetConfig()->GetBoardType() != boardRUv1)
      std::cout << "NO I2C INTERFACE FOUND\n";
    else {
      TReadoutBoardRUv1 *theBoard = dynamic_cast<TReadoutBoardRUv1 *>(fBoards.at(i));
      theBoard->i2c_gbtx->LoadConfig(0, "../RUv1Src/GBTX_Configs/GBTx0_Config_RUv1_1.xml");
      usleep(10000);
      if (theBoard->i2c_gbtx->isConfigLoaded(0, "../RUv1Src/GBTX_Configs/GBTx0_Config_RUv1_1.xml"))
        std::cout << "XML CONFIG WRITTEN SUCCESSFULLY \n";
      else
        std::cout << "XML CONFIG WAS NOT WRITTEN SUCCESSFULLY \n";
    }
  }
}


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

void InitChips(bool testRow)
{

  // This will mask all but the number of pixels you select between 0 and 1024 (a full column, just
  // for quick testing purposes)

  for (unsigned int i = 0; i < fChips.size(); i++) {

    if (fChips.at(i)->GetConfig()->IsEnabled()) {
      auto ch     = fChips.at(i);
      auto chipId = fChips.at(i)->GetConfig()->GetChipId();

      std::cout << "Configure chip " << (int)chipId << " for readout\n";

      AlpideConfig::BaseConfig(ch);
      // AlpideConfig::Init(ch);

      configureFromu(ch);
      AlpideConfig::ConfigureCMU(ch);

      if (!testRow)
        AlpideConfig::BaseConfigMask(ch);
      else {
        AlpideConfig::ConfigureMaskStage(ch, 32, 223);
      }
    }
  }
}

void TestChipRW()
{

  // Basic read/write chip register test (chip->ReadRegister is same as board->WriteChipRegister)

  uint16_t Value;
  std::cout << "Reset Chips (GRST)\n";
  for (unsigned int i = 0; i < fBoards.size(); i++) {
    fBoards.at(i)->SendOpCode(Alpide::OPCODE_GRST);
  }
  std::cout << "Chip register Tests"
            << "\n";
  for (unsigned int i = 0; i < fChips.size(); i++) {
    fChips.at(i)->WriteRegister(24, 10);
    try {
      fChips.at(i)->ReadRegister(24, Value);
      std::cout << "Chip ID " << fChips.at(i)->GetConfig()->GetChipId() << ", Value = 0x"
                << std::hex << (int)Value << std::dec << std::endl;
    }
    catch (exception &e) {
      std::cout << "Chip ID " << fChips.at(i)->GetConfig()->GetChipId()
                << ", not answering, exception: " << e.what() << std::endl;
    }
  }
}

void TestTrigger(int numTriggers, bool dumpConfig)
{

  // Triggers numTriggers times, dumps config if dumpconfig is true


  std::vector<uint8_t> buffer(1 * 1024 * 1024);
  int                  n_bytes_data = 0;
  std::cout << "Triggering " << numTriggers << " times across all boards \n";
  for (unsigned int i = 0; i < fBoards.size(); i++) {
    TBoardType boardType = fBoards.at(i)->GetConfig()->GetBoardType();
    if (boardType != boardRUv1) {
      fBoards.at(i)->Trigger(numTriggers);
      usleep(10000 * numTriggers);
      for (int j = 0; j < numTriggers; j++) {
        if (fBoards.at(i)->ReadEventData(n_bytes_data, buffer.data()) > 0) {
          std::cout << "EVENT: " << j << ", BYTES READ: " << n_bytes_data << std::endl;
        }
        else
          std::cout << "EVENT: " << j << ", DATA READ FAILED ON BOARD " << i << std::endl;
      }
    }
    else {
      TReadoutBoardRUv1 *theBoard = dynamic_cast<TReadoutBoardRUv1 *>(fBoards.at(i));
      theBoard->ResetAllCounters();
      theBoard->Trigger(numTriggers);
      usleep(10000 * numTriggers);
      for (int j = 0; j < numTriggers; j++) {
        if (theBoard->ReadEventData(n_bytes_data, buffer.data()) > 0) {
          std::cout << "EVENT: " << j << ", BYTES READ: " << n_bytes_data << std::endl;
        }
        else
          std::cout << "EVENT: " << j << ", DATA READ FAILED ON BOARD " << i << std::endl;
      }
    }

    if (dumpConfig && (boardType == boardRU)) {
      TReadoutBoardRU *ABoard = dynamic_cast<TReadoutBoardRU *>(fBoards.at(i));
      for (unsigned int k = 0; k < fChips.size(); ++k) {
        auto tr       = ABoard->transceiver_array[k];
        auto counters = tr->ReadCounters();
        std::cout << k << ";" << counters["Events NrEvents"] << ";"
                  << counters["Idlesuppress overflow"] << ";" << counters["8b10b Code Error"]
                  << "\n";
      }
    }
    if (dumpConfig && (boardType == boardRUv1)) {
      TReadoutBoardRUv1 *theBoard = dynamic_cast<TReadoutBoardRUv1 *>(fBoards.at(i));
      theBoard->LatchAllCounters();
      theBoard->DumpCounterConfig();
    }
  }
}

void CleanUp()
{
  for (unsigned int i = 0; i < fBoards.size(); i++) {
    TBoardType boardType = fBoards.at(i)->GetConfig()->GetBoardType();
    if (boardType == boardRU) {
      TReadoutBoardRU *ABoard = dynamic_cast<TReadoutBoardRU *>(fBoards.at(i));
      ABoard->CleanUp();
    }
    if (boardType == boardRUv1) {
      TReadoutBoardRUv1 *theBoard = dynamic_cast<TReadoutBoardRUv1 *>(fBoards.at(i));
      theBoard->CleanUp();
    }
  }
}

void TestEventDecode(int numTriggers, bool verbose)
{
  std::vector<uint8_t> buffer(1 * 1024 * 1024);
  int                  n_bytes_data, n_bytes_header, n_bytes_trailer, n_bytes_chipevent;
  int                  prioErrors = 0;
  TBoardHeader         boardInfo;
  // TBoardHeader          frameInfo;
  std::vector<TPixHit> *Hits     = new std::vector<TPixHit>;
  std::vector<TPixHit> *Stuck    = new std::vector<TPixHit>;
  int *                 chipId   = 0;
  unsigned int *        bc       = 0;
  int                   hitLimit = 20;


  std::cout << "TESTING EVENT DECODE WITH " << numTriggers << " TRIGGERS \n";
  for (unsigned int i = 0; i < fBoards.size(); i++) {
    TBoardType boardType = fBoards.at(i)->GetConfig()->GetBoardType();
    if (boardType != boardRUv1) {
      fBoards.at(i)->Trigger(numTriggers);
      usleep(10000 * numTriggers);
      for (int j = 0; j < numTriggers; j++) {
        if (fBoards.at(i)->ReadEventData(n_bytes_data, buffer.data()) > 0) {
          BoardDecoder::DecodeEvent(boardType, buffer.data(), n_bytes_data, n_bytes_header,
                                    n_bytes_trailer, boardInfo);
          n_bytes_chipevent = n_bytes_data - n_bytes_header - n_bytes_trailer;
          AlpideDecoder::DecodeEvent(buffer.data() + n_bytes_header, n_bytes_chipevent, Hits, 0,
                                     boardInfo.channel, prioErrors, hitLimit, Stuck, chipId, bc);
          std::cout << "NUMBER OF HITS: " << Hits->size() << std::endl;
        }
        else
          std::cout << "EVENT: " << j << ", DATA READ FAILED ON BOARD " << i << std::endl;
      }
    }
    else {
      TReadoutBoardRUv1 *theBoard = dynamic_cast<TReadoutBoardRUv1 *>(fBoards.at(i));
      theBoard->ResetAllCounters();
      for (int j = 0; j < numTriggers; j++) {
        theBoard->Trigger(1);
        usleep(10000);
        for (int count = 0; count < numEnabled; count++) {
          if (theBoard->ReadEventData(n_bytes_data, buffer.data()) > 0) {
            BoardDecoder::DecodeEvent(boardType, buffer.data(), n_bytes_data, n_bytes_header,
                                      n_bytes_trailer, boardInfo);
            n_bytes_chipevent = n_bytes_data - n_bytes_trailer - n_bytes_header;
            AlpideDecoder::DecodeEvent(buffer.data() + n_bytes_header, n_bytes_chipevent, Hits, 0,
                                       boardInfo.channel, prioErrors, hitLimit, Stuck, chipId, bc);
            std::cout << "NUMBER OF HITS: " << Hits->size() << std::endl;
          }
          else
            std::cout << "EVENT: " << j << ", DATA READ FAILED ON BOARD " << i << std::endl;
        }
        /*if (theBoard->ReadFrameData(n_bytes_data, buffer.data()) > 0) {
          BoardDecoder::DecodeGbtFrame(buffer.data(), n_bytes_data, frameInfo, true);
          // if(frameInfo.timeout == true) std::cout << "WARNING: LANE TIMEOUT, EVENT " << j <<
          // std::endl;
          // std::cout << std::hex << frameInfo.boardID << " BOARD ID" << std::dec << std::endl;
          }*/
        // else
        // std::cout << "FRAME: " << j << ", DATA READ FAILED ON BOARD " << i << std::endl;
      }
      if (verbose) {
        theBoard->LatchAllCounters();
        theBoard->DumpCounterConfig();
      }
    }
  }
}

int main(int argc, char **argv)
{

  bool testRow     = true;
  int  numTriggers = 100;


  std::cout << "Decode Command parameters\n";
  decodeCommandParameters(argc, argv);

  std::cout << "Create BoardConfig+Board\n";
  initSetup(fConfig, &fBoards, &fBoardType, &fChips);

  TReadoutBoardRUv1 *theBoard = dynamic_cast<TReadoutBoardRUv1 *>(fBoards.at(0));
  theBoard->checkGitHash();

  for (size_t i = 0; i < fChips.size(); i++) {
    if (fChips.at(i)->GetConfig()->IsEnabled()) numEnabled++;
  }
  // TestChipRW();
  InitChips(testRow);


  theBoard->Initialize(fChips.at(8)->GetConfig()->GetParamValue("LINKSPEED"));

  // prepare for trigger/readout
  for (unsigned int i = 0; i < fBoards.size(); i++) {
    fBoards.at(i)->StartRun();
    fBoards.at(i)->SetTriggerConfig(true, true, 100, 1000);
  }

  theBoard->SendStartOfTriggered();
  TestEventDecode(numTriggers, true);
  theBoard->SendEndOfTriggered();


  CleanUp();

  std::cout << "TEST COMPLETE! \n";

  return 0;
}

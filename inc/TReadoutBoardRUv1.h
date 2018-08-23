#ifndef TREADOUTBOARDRUV1_H
#define TREADOUTBOARDRUV1_H

#include "TBoardConfigRUv1.h"
#include "TConfig.h"
#include "TReadoutBoard.h"
#include <thread>
//#include "USB.h"

#include <deque>
#include <map>
#include <memory>

#include "RUv1Src/TRUv1DatapathMon.h"
#include "RUv1Src/TRUv1DctrlModule.h"
#include "RUv1Src/TRUv1DrpBridge.h"
#include "RUv1Src/TRUv1GBTXInject.h"
#include "RUv1Src/TRUv1GbtPacker.h"
#include "RUv1Src/TRUv1GbtPackerMonitor.h"
#include "RUv1Src/TRUv1GbtxCont.h"
#include "RUv1Src/TRUv1GbtxFlowMon.h"
#include "RUv1Src/TRUv1GpioFrontend.h"
#include "RUv1Src/TRUv1GthFrontend.h"
#include "RUv1Src/TRUv1PowerBoard.h"
#include "RUv1Src/TRUv1Sysmon.h"
#include "RUv1Src/TRUv1TriggerHandler.h"
#include "RUv1Src/TRUv1TriggerHandlerMonitor.h"
#include "RUv1Src/TRUv1WsI2cGbtx.h"
#include "RUv1Src/TRUv1WsMaster.h"
#include "RUv1Src/TRUv1WsRadMon.h"
#include "RUv1Src/TRUv1WsStatus.h"
#include "RUv1Src/TRUv1WsUsbIf.h"
#include "RUv1Src/TRUv1WsWait.h"
#include "RUv1Src/UsbDev.hpp"

class TReadoutBoardRUv1 : public TReadoutBoard {
public:
  struct ReadResult {
    uint16_t address;
    uint16_t data;
    bool     error;
    ReadResult(uint16_t address, uint16_t data, bool error)
        : address(address), data(data), error(error)
    {
    }
  };

  static const int     VID;
  static const int     PID;
  static const int     INTERFACE_NUMBER;
  static const uint8_t EP_CTL_OUT;
  static const uint8_t EP_CTL_IN;
  static const uint8_t EP_DATA0_IN;
  static const uint8_t EP_DATA1_IN;

  static const size_t EVENT_DATA_READ_CHUNK;
  static const size_t USB_TIMEOUT;
  static const int    MAX_RETRIES_READ;

  static const uint8_t MODULE_MASTER;
  static const uint8_t MODULE_STATUS;
  static const uint8_t MODULE_GTH_FRONTEND;
  static const uint8_t MODULE_DATAPATH_MONITOR;
  static const uint8_t MODULE_ALPIDE;
  static const uint8_t MODULE_I2C_GBT;
  static const uint8_t MODULE_I2C_PU1;
  static const uint8_t MODULE_I2C_PU2;
  static const uint8_t MODULE_GBTX0;
  static const uint8_t MODULE_GBTX2;
  static const uint8_t MODULE_WAIT;
  static const uint8_t MODULE_RADMON;
  static const uint8_t MODULE_SYSMON;
  static const uint8_t MODULE_GBTX_FLOW_MONITOR;
  static const uint8_t MODULE_USB_IF;
  static const uint8_t MODULE_USB_MASTER;
  static const uint8_t MODULE_TRG_HANDLER;
  static const uint8_t MODULE_TRG_HANDLER_MONITOR;
  static const uint8_t MODULE_GPIO_CONTROL;
  static const uint8_t MODULE_DATAPATH_MONITOR_GPIO_1;
  static const uint8_t MODULE_DATAPATH_MONITOR_GPIO_2;
  static const uint8_t MODULE_GBT_PACKER_GTH;
  static const uint8_t MODULE_GBT_PACKER_GPIO;
  static const uint8_t MODULE_GBTX_WORD_INJECT;
  static const uint8_t MODULE_GBT_PACKER_MONITOR_GTH;
  static const uint8_t MODULE_GTH_DRP;

  static const uint8_t MODULE_CRU_MASTER;
  static const uint8_t MODULE_CRU_STATUS;
  static const uint8_t MODULE_SCA;
  static const uint8_t MODULE_GBT_FPGA;
  static const uint8_t MODULE_CRU_WAIT;


private:
  void                    ContinuousRead();
  std::shared_ptr<UsbDev> m_usb;
  TBoardConfigRUv1 *      m_config;
  UsbDev::DataBuffer      m_buffer;
  uint32_t                m_readBytes;


  bool m_logging;

  // Triggeroptions
  bool m_enablePulse;
  bool m_enableTrigger;
  int  m_triggerDelay;
  int  m_pulseDelay;

  std::map<uint8_t, std::vector<uint8_t>> m_readoutBuffers;
  std::vector<UsbDev::DataBuffer>         m_gbtFrames;
  std::deque<std::vector<uint8_t>>        m_events;

  // Readout streams


public:
  // Modules
  std::shared_ptr<TRUv1DctrlModule>           dctrl;
  std::shared_ptr<TRUv1WsMaster>              master;
  std::shared_ptr<TRUv1GthFrontend>           gth;
  std::shared_ptr<TRUv1GpioFrontend>          gpio;
  std::shared_ptr<TRUv1GbtxCont>              gbtx_0;
  std::shared_ptr<TRUv1GbtxCont>              gbtx_2;
  std::shared_ptr<TRUv1WsStatus>              status;
  std::shared_ptr<TRUv1DatapathMon>           datapathmon;
  std::shared_ptr<TRUv1WsWait>                wait_module;
  std::shared_ptr<TRUv1WsI2cGbtx>             i2c_gbtx;
  std::shared_ptr<TRUv1WsRadMon>              radmon;
  std::shared_ptr<TRUv1Sysmon>                sysmon;
  std::shared_ptr<TRUv1GbtxFlowMon>           gbtx_flow_monitor;
  std::shared_ptr<TRUv1WsMaster>              master_usb;
  std::shared_ptr<TRUv1WsUsbIf>               usb_if;
  std::shared_ptr<TRUv1GbtPacker>             gbt_packer_gth;
  std::shared_ptr<TRUv1GbtPacker>             gbt_packer_gpio;
  std::shared_ptr<TRUv1TriggerHandler>        trigger_handler;
  std::shared_ptr<TRUv1TriggerHandlerMonitor> trigger_handler_monitor;
  std::shared_ptr<TRUv1GBTXInject>            gbtx_word_inject;
  std::shared_ptr<TRUv1GbtPackerMonitor>      gbt_packer_monitor_gth;
  std::shared_ptr<TRUv1DrpBridge>             drp_bridge;
  std::shared_ptr<TRUv1WishboneModule>        gbt_fpga;
  std::shared_ptr<TRUv1WishboneModule>        cru_master;
  std::shared_ptr<TRUv1WishboneModule>        cru_status;
  std::shared_ptr<TRUv1PowerBoard>            powerunit_1;
  std::shared_ptr<TRUv1PowerBoard>            powerunit_2;
  std::vector<std::thread>                    readThreads;

  // std::shared_ptr<TRUv1WishboneToFifo> wb2fifo;


public:
  TReadoutBoardRUv1(TBoardConfigRUv1 *config);

  virtual int WriteChipRegister(uint16_t Address, uint16_t Value, TAlpide *chipPtr = 0);
  virtual int ReadRegister(uint16_t Address, uint32_t &Value);
  virtual int WriteRegister(uint16_t Address, uint32_t Value);
  virtual int ReadChipRegister(uint16_t Address, uint16_t &Value, TAlpide *chipPtr = 0);
  virtual int SendOpCode(Alpide::TOpCode OpCode);
  virtual int SendOpCode(Alpide::TOpCode OpCode, TAlpide *chipPtr);
  virtual int SendCommand(Alpide::TCommand OpCode, TAlpide *chipPtr);

  virtual int  SetTriggerConfig(bool enablePulse, bool enableTrigger, int triggerDelay,
                                int pulseDelay);
  virtual void SetTriggerSource(TTriggerSource triggerSource);
  virtual void StartRun();
  virtual int  Trigger(int nTriggers);
  virtual int  ReadEventData(int &NBytes, unsigned char *Buffer);

  int ReadFrameData(int &NBytes, unsigned char *Buffer);

  int                     Initialize(int linkspeed = 1200);
  void                    MaskPackerLanes();
  void                    TriggerDebug(int nTriggers = 1);
  void                    ResetAllCounters();
  void                    LatchAllCounters();
  void                    PrepareTransceivers();
  void                    DumpCounterConfig();
  void                    registeredWrite(uint16_t module, uint16_t address, uint16_t data);
  void                    registeredRead(uint16_t module, uint16_t address);
  bool                    flush();
  size_t                  readFromPort(uint8_t port, size_t size, UsbDev::DataBuffer &buffer);
  void                    CleanUp();
  std::vector<ReadResult> readResults();

  void SendStartOfTriggered();
  void SendEndOfTriggered();
  void SendTrigger(uint16_t triggerType = 0x10, uint16_t bc = 0xdead, uint32_t orbit = 0xfaceb00c,
                   bool commit = true);
  void EmulateCRU();
  void checkGitHash();
  void fetchEventData();
  void SetupSensors();
  void SetLinkspeed(int linkspeed = 1200);
  void ElasticBufferFix(int linkspeed = 600);
};

#endif // TREADOUTBOARDRUV1_H

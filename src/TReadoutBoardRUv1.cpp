#include "TReadoutBoardRUv1.h"
#include "AlpideConfig.h"
#include "AlpideDecoder.h"
#include "TAlpide.h"
#include <algorithm>
#include <iomanip>
#include <mutex>

const int     TReadoutBoardRUv1::VID              = 0x04B4;
const int     TReadoutBoardRUv1::PID              = 0x0008;
const int     TReadoutBoardRUv1::INTERFACE_NUMBER = 2;
const uint8_t TReadoutBoardRUv1::EP_CTL_OUT       = 3;
const uint8_t TReadoutBoardRUv1::EP_CTL_IN        = 3;
const uint8_t TReadoutBoardRUv1::EP_DATA0_IN      = 4;
const uint8_t TReadoutBoardRUv1::EP_DATA1_IN      = 5;

const size_t TReadoutBoardRUv1::EVENT_DATA_READ_CHUNK = 1024 * 1024;
const size_t TReadoutBoardRUv1::USB_TIMEOUT           = 5;
const int    TReadoutBoardRUv1::MAX_RETRIES_READ      = 5;

const uint8_t TReadoutBoardRUv1::MODULE_MASTER                  = 0;
const uint8_t TReadoutBoardRUv1::MODULE_STATUS                  = 1;
const uint8_t TReadoutBoardRUv1::MODULE_GTH_FRONTEND            = 2;
const uint8_t TReadoutBoardRUv1::MODULE_DATAPATH_MONITOR        = 3;
const uint8_t TReadoutBoardRUv1::MODULE_ALPIDE                  = 4;
const uint8_t TReadoutBoardRUv1::MODULE_I2C_GBT                 = 5;
const uint8_t TReadoutBoardRUv1::MODULE_I2C_PU1                 = 6;
const uint8_t TReadoutBoardRUv1::MODULE_I2C_PU2                 = 7;
const uint8_t TReadoutBoardRUv1::MODULE_GBTX0                   = 8;
const uint8_t TReadoutBoardRUv1::MODULE_GBTX2                   = 9;
const uint8_t TReadoutBoardRUv1::MODULE_WAIT                    = 10;
const uint8_t TReadoutBoardRUv1::MODULE_RADMON                  = 11;
const uint8_t TReadoutBoardRUv1::MODULE_SYSMON                  = 12;
const uint8_t TReadoutBoardRUv1::MODULE_GBTX_FLOW_MONITOR       = 13;
const uint8_t TReadoutBoardRUv1::MODULE_USB_IF                  = 14;
const uint8_t TReadoutBoardRUv1::MODULE_USB_MASTER              = 15;
const uint8_t TReadoutBoardRUv1::MODULE_TRG_HANDLER             = 16;
const uint8_t TReadoutBoardRUv1::MODULE_TRG_HANDLER_MONITOR     = 17;
const uint8_t TReadoutBoardRUv1::MODULE_GPIO_CONTROL            = 18;
const uint8_t TReadoutBoardRUv1::MODULE_DATAPATH_MONITOR_GPIO_1 = 19;
const uint8_t TReadoutBoardRUv1::MODULE_DATAPATH_MONITOR_GPIO_2 = 20;
const uint8_t TReadoutBoardRUv1::MODULE_GBT_PACKER_GTH          = 21;
const uint8_t TReadoutBoardRUv1::MODULE_GBT_PACKER_GPIO         = 22;
const uint8_t TReadoutBoardRUv1::MODULE_GTH_DRP                 = 23;
const uint8_t TReadoutBoardRUv1::MODULE_GBT_PACKER_MONITOR_GTH  = 24;
const uint8_t TReadoutBoardRUv1::MODULE_GBTX_WORD_INJECT        = 26;
bool          triggerMode;

// THESE ARE EMULATOR MODULES ONLY
const uint8_t TReadoutBoardRUv1::MODULE_CRU_MASTER = 0x40;
const uint8_t TReadoutBoardRUv1::MODULE_CRU_STATUS = 0x41;
const uint8_t TReadoutBoardRUv1::MODULE_SCA        = 0x42;
const uint8_t TReadoutBoardRUv1::MODULE_GBT_FPGA   = 0x43;
const uint8_t TReadoutBoardRUv1::MODULE_CRU_WAIT   = 0x44;
bool          emulator                             = false;

// THESE ARE SPECIFIC TO NEW READOUT METHOD (continous readout from port)
bool       readoutBG = false;
std::mutex eventBufferMtx;
std::mutex gbtFrameMtx;


// Doing exactly what it says it does. Making sure everything is groovy wrt usb transactions.
int roundUpToMultipleV1(int numToRound, int multiple)
{
  if (multiple == 0) return numToRound;

  int remainder = numToRound % multiple;
  if (remainder == 0) return numToRound;

  return numToRound + multiple - remainder;
}

TReadoutBoardRUv1::TReadoutBoardRUv1(TBoardConfigRUv1 *config)
    : TReadoutBoard(config), m_buffer(), m_readBytes(0), m_logging(config->enableLogging()),
      m_enablePulse(false), m_enableTrigger(true), m_triggerDelay(0), m_pulseDelay(0)
{

  // M_USB: This is the module that handles reading/writing to the board at the lowest level, all
  // directly through LIBUSB. The 4th argument needs to be there (it's dedicated to the serial
  // number), but the 5th argument (DIPSWITCH) is what uniquely identifies the board if multiple are
  // plugged in.
  m_usb = std::make_shared<UsbDev>(TReadoutBoardRUv1::VID, TReadoutBoardRUv1::PID,
                                   TReadoutBoardRUv1::INTERFACE_NUMBER, "hehe facebook",
                                   config->GetParamValue("DIPSWITCH"));

  // NEXT TWO MODULES ARE TRANSCEIVER MODULES. For data taking, you must align and enable data! As
  // of now, it is not possible to have both gth + gpio enabled simultaenously on the firmware side
  // of things.
  gth =
      std::make_shared<TRUv1GthFrontend>(*this, TReadoutBoardRUv1::MODULE_GTH_FRONTEND, m_logging);
  gpio =
      std::make_shared<TRUv1GpioFrontend>(*this, TReadoutBoardRUv1::MODULE_GPIO_CONTROL, m_logging);

  // GBTX: Not the i2c interface, but you can enable/disable the controllers here.

  gbtx_0 = std::make_shared<TRUv1GbtxCont>(*this, TReadoutBoardRUv1::MODULE_GBTX0, m_logging);

  gbtx_2 = std::make_shared<TRUv1GbtxCont>(*this, TReadoutBoardRUv1::MODULE_GBTX2, m_logging);

  // STATUS: Module that contains cool status info like time/date/githash/dipswitch

  status = std::make_shared<TRUv1WsStatus>(*this, TReadoutBoardRUv1::MODULE_STATUS, m_logging);

  // DATAPATHMON: The #1 data infidelity debugger. Counts events, errors, decode errors, buffer
  // overflows, the whole shabang, right AFTER data is sent from the chip (before it gets to the GBT
  // packer). Hopefully all decode errors in the software side are reflected here.

  datapathmon = std::make_shared<TRUv1DatapathMon>(
      *this, TReadoutBoardRUv1::MODULE_DATAPATH_MONITOR, m_logging);

  // WS_WAIT: A wait module that "waits" in increments of the WS clock cycle (120 MHz I THINK,
  // please don't trust me)

  wait_module = std::make_shared<TRUv1WsWait>(*this, TReadoutBoardRUv1::MODULE_WAIT, m_logging);

  // I2C_GBTX: the I2c interface for the GBTX. I wrote a nice method that programs the GBTX from
  // this module, I'm pretty proud.

  i2c_gbtx = std::make_shared<TRUv1WsI2cGbtx>(*this, TReadoutBoardRUv1::MODULE_I2C_GBT, m_logging);

  // RAD/SYSMON: I've literally never used these, I'm sure they're important for radiation tests but
  // not for classifying hics/staves.

  radmon = std::make_shared<TRUv1WsRadMon>(*this, TReadoutBoardRUv1::MODULE_RADMON, m_logging);

  sysmon = std::make_shared<TRUv1Sysmon>(*this, TReadoutBoardRUv1::MODULE_SYSMON, m_logging);

  // GBTX_FLOW: Counts downlink transactions (SWT and triggers) and uplink transactions (SOP/EOP and
  // SWT). THIS DOES NOT WORK WITH USB READOUT (if you call TakeControl with gbtx_word_inject) FOR
  // OBVIOUS REASONS.

  gbtx_flow_monitor = std::make_shared<TRUv1GbtxFlowMon>(
      *this, TReadoutBoardRUv1::MODULE_GBTX_FLOW_MONITOR, m_logging);

  // MASTER_USB: OUTDATED, usb_if is the new king.

  master_usb =
      std::make_shared<TRUv1WsMaster>(*this, TReadoutBoardRUv1::MODULE_USB_MASTER, m_logging);

  usb_if = std::make_shared<TRUv1WsUsbIf>(*this, TReadoutBoardRUv1::MODULE_USB_IF, m_logging);

  // DCTRL: The classic dctrl module, handles all control transactions to the chips.

  dctrl = std::make_shared<TRUv1DctrlModule>(*this, TReadoutBoardRUv1::MODULE_ALPIDE, m_logging);

  // MASTER: You know for a name ike "master" you'd think this module would be more exciting, but on
  // the software level we don't really talk to it. This is the wishbone master.

  master = std::make_shared<TRUv1WsMaster>(*this, TReadoutBoardRUv1::MODULE_MASTER, m_logging);

  // TRIGGER_HANDLER: Handles the triggers lol. But really, this is where you can flag trigger OR
  // pulse (not set up to do both on the firmware level, though with WS_WAIT it should be possible
  // to handle this on software level). Can also enable continous mode with a fixed period, but that
  // would only send whatever it's flagged to send (i.e. trigger or pulse)

  trigger_handler = std::make_shared<TRUv1TriggerHandler>(
      *this, TReadoutBoardRUv1::MODULE_TRG_HANDLER, m_logging);

  // TRIGGER_HANDLER_MONITOR: Counts triggers sent, has some fifo overflow/full counters.

  trigger_handler_monitor = std::make_shared<TRUv1TriggerHandlerMonitor>(
      *this, TReadoutBoardRUv1::MODULE_TRG_HANDLER_MONITOR, m_logging);

  // GBT_PACKER: Module that handles the packaging of data, sends to CRU (and USB if you have the
  // right firmware). Protocol can be found on multiple TWIKI presentations.

  gbt_packer_gth =
      std::make_shared<TRUv1GbtPacker>(*this, TReadoutBoardRUv1::MODULE_GBT_PACKER_GTH, m_logging);

  gbt_packer_gpio =
      std::make_shared<TRUv1GbtPacker>(*this, TReadoutBoardRUv1::MODULE_GBT_PACKER_GPIO, m_logging);

  // WORD_INJECT: Module to inject words (i.e. triggers) as opposed to them being sent from CRU.
  // This is EXTREMELY important for USB readout, also must call TakeControl for this module to take
  // over the data/strobe/clock lanes.

  gbtx_word_inject = std::make_shared<TRUv1GBTXInject>(
      *this, TReadoutBoardRUv1::MODULE_GBTX_WORD_INJECT, m_logging);

  // GBT_PACKER_MONITOR: Monitors the gbt packer, has counters for things like packets
  // sent/received, empty packets, etc.

  gbt_packer_monitor_gth = std::make_shared<TRUv1GbtPackerMonitor>(
      *this, TReadoutBoardRUv1::MODULE_GBT_PACKER_MONITOR_GTH, m_logging);

  // DRP_BRIDGE: For reading/writing to the drp addresses on the transceivers, WB module added for
  // convenience

  drp_bridge =
      std::make_shared<TRUv1DrpBridge>(*this, TReadoutBoardRUv1::MODULE_GTH_DRP, m_logging);

  // POWER BOARD WB MODULES: Wishbone slaves responsible for communication with I2C interface to
  // powerboard

  powerunit_1 =
      std::make_shared<TRUv1PowerBoard>(*this, TReadoutBoardRUv1::MODULE_I2C_PU1, m_logging);

  powerunit_2 =
      std::make_shared<TRUv1PowerBoard>(*this, TReadoutBoardRUv1::MODULE_I2C_PU2, m_logging);

  // THESE ARE EMULATOR MODULES ONLY

  gbt_fpga =
      std::make_shared<TRUv1WishboneModule>(*this, TReadoutBoardRUv1::MODULE_GBT_FPGA, m_logging);

  cru_master =
      std::make_shared<TRUv1WishboneModule>(*this, TReadoutBoardRUv1::MODULE_CRU_MASTER, m_logging);

  cru_status =
      std::make_shared<TRUv1WishboneModule>(*this, TReadoutBoardRUv1::MODULE_CRU_STATUS, m_logging);
}


// fetchEventData: This guy reads from the data ports in LARGE chunks, then sifts through the "mess"
// to store the relevant info. m_events contains all the data from a single event on a single lane,
// where the last byte is the lane that event came from. m_gbtFrames contains gbtFrames, from SOP to
// EOP and everything in between. As of right now, only m_events is getting filled as m_gbtFrames
// isn't being read out yet so it will grow indefinitely.
void TReadoutBoardRUv1::fetchEventData()
{

  std::map<uint8_t, size_t> chipByteCounters;

  static UsbDev::DataBuffer buffer;
  readFromPort(TReadoutBoardRUv1::EP_DATA0_IN, TReadoutBoardRUv1::EVENT_DATA_READ_CHUNK, buffer);
  for (size_t i = 0; i < buffer.size(); i += 12) {

    uint8_t lane = buffer[i + 1];
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

      // gbtFrameMtx.lock();
      // m_gbtFrames.push_back(gbtFrame);
      // gbtFrameMtx.unlock();
    }

    if ((lane != 0) && (lane != 0x20) && (lane != 0x10) && (lane != 0xf0) && (lane != 0xe0)) {


      m_readoutBuffers[lane].push_back(buffer[i + 8]);
      m_readoutBuffers[lane].push_back(buffer[i + 9]);
      m_readoutBuffers[lane].push_back(buffer[i + 10]);
      m_readoutBuffers[lane].push_back(buffer[i + 11]);
      m_readoutBuffers[lane].push_back(buffer[i + 4]);
      m_readoutBuffers[lane].push_back(buffer[i + 5]);
      m_readoutBuffers[lane].push_back(buffer[i + 6]);
      m_readoutBuffers[lane].push_back(buffer[i + 7]);
      m_readoutBuffers[lane].push_back(buffer[i]);
    }
  }


  // Read out events
  for (auto &buffer : m_readoutBuffers) {
    auto  laneId     = buffer.first;
    auto &data       = buffer.second;
    int   eventStart = 0;
    int   eventEnd   = 0;
    bool  isError;


    while (AlpideDecoder::ExtractNextEvent(data.data(), data.size(), eventStart, eventEnd, isError,
                                           false)) {
      if (isError and m_logging) {
        std::cout << "Event decoding error on lane 0x" << std::hex << (int)laneId
                  << ". Event size: " << std::dec << eventEnd - eventStart << "\n";
      }


      // Extract event bytes and store in event list

      std::vector<uint8_t> eventData(begin(data) + eventStart, begin(data) + eventEnd);
      eventData.push_back((uint8_t)laneId -
                          1); // -1 because of the fun laneID protocol where it starts at 1
      if (!isError) {
        m_events.emplace_back(eventData);
        /*std::cout << "===== Event Data ======\n";
     std::cout << std::hex;
     for (int i = eventStart; i < eventEnd; ++i) {
         std::cout << std::setfill('0') << std::setw(2)
                   << (int)data[i] << " ";
         if (i % 20 == 19)
             std::cout << "\n";
     }
     std::cout << std::dec;
     std::cout << "\n========== /Event Data ==========\n";*/
      }
      // event is extracted, remove data from buffer
      data.erase(begin(data), begin(data) + eventEnd);
    }
  }
}

// ContinousRead: This is a CARBON copy of fetchEventData, but meant to be run on a separate thread
// (readThreads). As they both fill the previously mentioned vectors (m_events, m_gbtFrames), you
// should probably choose one or the other (preferably this one if you have the threads)
void TReadoutBoardRUv1::ContinuousRead()
{
  while (readoutBG) {
    std::map<uint8_t, size_t> chipByteCounters;

    static UsbDev::DataBuffer buffer;
    readFromPort(TReadoutBoardRUv1::EP_DATA0_IN, TReadoutBoardRUv1::EVENT_DATA_READ_CHUNK, buffer);
    for (size_t i = 0; i < buffer.size(); i += 12) {

      uint8_t lane = buffer[i + 1];
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

        /*gbtFrameMtx.lock();
        m_gbtFrames.push_back(gbtFrame);
        gbtFrameMtx.unlock();*/
      }

      if ((lane != 0) && (lane != 0x20) && (lane != 0x10) && (lane != 0xf0) && (lane != 0xe0)) {


        m_readoutBuffers[lane].push_back(buffer[i + 8]);
        m_readoutBuffers[lane].push_back(buffer[i + 9]);
        m_readoutBuffers[lane].push_back(buffer[i + 10]);
        m_readoutBuffers[lane].push_back(buffer[i + 11]);
        m_readoutBuffers[lane].push_back(buffer[i + 4]);
        m_readoutBuffers[lane].push_back(buffer[i + 5]);
        m_readoutBuffers[lane].push_back(buffer[i + 6]);
        m_readoutBuffers[lane].push_back(buffer[i + 7]);
        m_readoutBuffers[lane].push_back(buffer[i]);
      }
    }


    // Read out events
    for (auto &buffer : m_readoutBuffers) {
      auto  laneId     = buffer.first;
      auto &data       = buffer.second;
      int   eventStart = 0;
      int   eventEnd   = 0;
      bool  isError;


      while (AlpideDecoder::ExtractNextEvent(data.data(), data.size(), eventStart, eventEnd,
                                             isError, false)) {
        if (isError and m_logging) {
          std::cout << "Event decoding error on lane 0x" << std::hex << (int)laneId
                    << ". Event size: " << std::dec << eventEnd - eventStart << "\n";
        }


        // Extract event bytes and store in event list

        std::vector<uint8_t> eventData(begin(data) + eventStart, begin(data) + eventEnd);
        eventData.push_back((uint8_t)laneId -
                            1); // -1 because of the fun laneID protocol where it starts at 1
        if (!isError) {
          eventBufferMtx.lock();
          m_events.emplace_back(eventData);
          eventBufferMtx.unlock();

          /*std::cout << "===== Event Data ======\n";
     std::cout << std::hex;
     for (int i = eventStart; i < eventEnd; ++i) {
         std::cout << std::setfill('0') << std::setw(2)
                   << (int)data[i] << " ";
         if (i % 20 == 19)
             std::cout << "\n";
     }
     std::cout << std::dec;
     std::cout << "\n========== /Event Data ==========\n";*/
        }
        // event is extracted, remove data from buffer

        data.erase(begin(data), begin(data) + eventEnd);
      }
    }
  }
}

// registeredWrite: Handles write transactions at the lowest level, but doesn't actually commit the
// transaction, just stores it to the buffer to eventually be flushed.

void TReadoutBoardRUv1::registeredWrite(uint16_t module, uint16_t address, uint16_t data)
{

  uint8_t module_byte  = module & 0x7F;
  uint8_t address_byte = address & 0xFF;
  uint8_t data_low     = data >> 0 & 0xFF;
  uint8_t data_high    = data >> 8 & 0xFF;

  module_byte |= 0x80; // Write operation

  m_buffer.push_back(data_low);
  m_buffer.push_back(data_high);
  m_buffer.push_back(address_byte);
  m_buffer.push_back(module_byte);
}

// registeredRead: Same as registeredWrite, handles read transactions but doesn't commit, just adds
// to usb buffer for eventual flush.

void TReadoutBoardRUv1::registeredRead(uint16_t module, uint16_t address)
{
  uint8_t module_byte  = module & 0x7F;
  uint8_t address_byte = address & 0xFF;

  m_buffer.push_back(0);
  m_buffer.push_back(0);
  m_buffer.push_back(address_byte);
  m_buffer.push_back(module_byte);

  m_readBytes += 4;
}

// flush: flushes m_buffer to the board (writes to control DP)

bool TReadoutBoardRUv1::flush()
{
  size_t toWrite = m_buffer.size();
  size_t written = m_usb->writeData(EP_CTL_OUT, m_buffer, USB_TIMEOUT);
  /*std::cout << "Bytes written: " << written << "( ";
  for (auto b : m_buffer)
   std::cout << "0x" <<std::hex << (int)b << ",";
   std::cout << ")\n";*/
  m_buffer.clear();

  return toWrite == written;
}

// readFromPort: Does exactly what it says it's doing, reads from the given DP for a given size and
// scales the buffer to that size.

size_t TReadoutBoardRUv1::readFromPort(uint8_t port, size_t size, UsbDev::DataBuffer &buffer)
{
  buffer.resize(roundUpToMultipleV1(size, 1024));
  m_usb->readData(port, buffer, USB_TIMEOUT);
  return buffer.size();
  // returns bytesRead : size_t
  // cout << (int)buffer.size() << endl;
}

// readResults: Reads from the control DP until nothing is left, stores each result as an entry in
// the output vector.
std::vector<TReadoutBoardRUv1::ReadResult> TReadoutBoardRUv1::readResults()
{
  // Read from Control Port
  UsbDev::DataBuffer buffer_all, buffer_single;
  int                retries   = 0;
  size_t             data_left = m_readBytes;
  while (buffer_all.size() < m_readBytes && retries < MAX_RETRIES_READ) {

    readFromPort(EP_CTL_IN, data_left, buffer_single);
    buffer_all.insert(buffer_all.end(), buffer_single.begin(), buffer_single.end());

    if (buffer_all.size() < m_readBytes) {
      data_left = m_readBytes - buffer_all.size();
      ++retries;
    }
  }
  if (buffer_all.size() < m_readBytes) {
    if (m_logging)
      std::cout << "TReadoutBoardRUv1: could not read all data from Control Port. "
                   "Packet dropped";
  }
  m_readBytes = 0;

  // Process read data
  std::vector<TReadoutBoardRUv1::ReadResult> results;
  for (size_t i = 0; i < buffer_all.size(); i += 4) {
    uint16_t data       = buffer_all[i] | (buffer_all[i + 1] << 8);
    uint16_t address    = buffer_all[i + 2] | (buffer_all[i + 3] << 8);
    bool     read_error = (address & 0x8000) > 0;
    address &= 0x7FFF;
    if (read_error && m_logging) {
      std::cout << "TReadoutBoardRUv1: Wishbone error while reading: Address " << address << "\n";
    }
    // std::cout << "Result received: Address: " << std::hex << (int)address <<
    // ", data: " << std::hex
    //          << data << "read error: " << read_error << "\n";
    results.emplace_back(address, data, read_error);
  }
  return results;
}

// ReadRegister: Never use this. These wishbone modules exist in software for a reason, use
// module->Read(address) instead. I mean, obviously you can use this, and it will work, but boy does
// it suck to find the "Address" of interest here.

int TReadoutBoardRUv1::ReadRegister(uint16_t Address, uint32_t &Value)
{
  uint16_t module = Address >> 8 & 0xFF;
  // std::cout << module << " is the module" << std::endl;
  uint16_t sub_address = Address & 0xFF;
  // std::cout << sub_address << " is the sub address" << std::endl;
  registeredRead(module, sub_address);
  flush();
  auto results = readResults();
  if (results.size() != 1) {
    if (m_logging)
      std::cout << "TReadoutBoardRUv1: Expected 1 result, got " << results.size() << "\n";
    return -1;
  }
  else {
    Value = results[0].data;
  }
  return 0;
}

// WriteRegister: Again, never use this. Use module->Write(address, value) instead. Read above for
// more information.

int TReadoutBoardRUv1::WriteRegister(uint16_t Address, uint32_t Value)
{
  uint16_t module      = Address >> 8 & 0xFF;
  uint16_t sub_address = Address & 0xFF;
  uint16_t data        = Value & 0xFFFF;
  registeredWrite(module, sub_address, data);
  flush();

  return 0;
}

// WriteChipRegister: Writes to the given register on the given chip. NOTE: the input here is the
// TAlpide object, not the chipID (obviously it won't compile if you try something else, but these
// comments are so you don't make these mistakes in the first place!)

int TReadoutBoardRUv1::WriteChipRegister(uint16_t Address, uint16_t Value, TAlpide *chipPtr)
{
  uint8_t chipId = chipPtr->GetConfig()->GetChipId();
  dctrl->WriteChipRegister(Address, Value, chipId);
  return 0;
}

// ReadChipRegister: Sets the right control interface so we can actually read from the chip, then
// reads from the chip. NOTE: UNLIKE THE READ FUNCTIONS OF THE WISHBONE MODULES, THIS GUY REQUIRES A
// "VALUE" SO IT CAN STORE THE VALUE TO THAT VALUE IF YOU VALUE ITS VALUE.
int TReadoutBoardRUv1::ReadChipRegister(uint16_t Address, uint16_t &Value, TAlpide *chipPtr)
{
  uint8_t chipId = chipPtr->GetConfig()->GetChipId();
  // set control


  dctrl->SetConnector(GetControlInterface(chipId), false);
  // dctrl->SetConnector(1, false); // TODO: set connector properly

  return dctrl->ReadChipRegister(Address, Value, chipId);
}

// SendOPCode: Sends OP code to all chips on the board
int TReadoutBoardRUv1::SendOpCode(Alpide::TOpCode OpCode)
{
  dctrl->SendOpCode(OpCode);
  return 0;
}

// SendOPCode: Sends OP code to all chips on the board even if you give it a single chip. IDK what's
// going on here but I'm keeping this out of fear.
int TReadoutBoardRUv1::SendOpCode(Alpide::TOpCode OpCode, TAlpide *chipPtr)
{
  return SendOpCode(OpCode);
}

// SendCommand: Sends a command to a single chip through dctrl. Possible commands can be found in
// the bible AKA Alpide Operations Manual

int TReadoutBoardRUv1::SendCommand(Alpide::TCommand Command, TAlpide *chipPtr)
{
  return WriteChipRegister(Alpide::REG_COMMAND, Command, chipPtr);
}

// SetTriggerConfig: Sets the trigger config. However, as the trigger handler is only set up to send
// EITHER triggers OR pulses, having both delays is unecessary (for now), and only triggerDelay
// works. Also, trigger delay delays in increments of the WS clock cycle (times 4), or 25 ns.

int TReadoutBoardRUv1::SetTriggerConfig(bool enablePulse, bool enableTrigger, int triggerDelay,
                                        int pulseDelay)
{
  m_enablePulse   = enablePulse;
  m_enableTrigger = enableTrigger;
  m_triggerDelay  = triggerDelay;
  m_pulseDelay    = pulseDelay;

  if (m_enablePulse)
    trigger_handler->Write(2, 1); // 1 = enable pulse, 0 = enable trigger
  else
    trigger_handler->Write(2, 0);

  return 0;
}

// TriggerDebug: A simple debugging tool for event readout that just triggers->reads the event data
// size. Does no decoding, and is a bit outdated.
void TReadoutBoardRUv1::TriggerDebug(int nTriggers)
{
  int                  numBytes = 0;
  std::vector<uint8_t> buffer(1 * 1024 * 1024);

  gbtx_word_inject->StartTriggeredMode();

  usleep(1000);
  for (int i = 0; i < nTriggers; ++i) {

    if (m_triggerDelay > 0) usleep(4 * m_triggerDelay);
    if (m_enableTrigger) gbtx_word_inject->SendTrigger(1 << 4);
    usleep(10000);
    gbt_packer_gth->EnableDataForward(1);
    usleep(100000);
    std::cout << ReadEventData(numBytes, buffer.data()) << std::endl;
    usleep(1000);
    gbt_packer_gth->EnableDataForward(0);
  }
  usleep(1000);
  gbtx_word_inject->StartTriggeredMode(false); // end of TRIGGERED MODE
}

// SetTriggerSource: Does nothing as of now, will eventually be used to toggle between
// CRU/word_inject or even the LTU or whatever the dedicated trigger board is called.

void TReadoutBoardRUv1::SetTriggerSource(TTriggerSource triggerSource) {}

// Trigger: The classic trigger. If we're doing USB readout, this is handled through the word_inject
// module where it sends a startTriggeredMode command, then nTriggers physics triggers, then an
// endTriggered mode command. The start/end of triggered mode commands are only sent if the
// triggerMode flag is set (or not set), just in case you forget to send these commands in whatever
// test you're running. The same thing happens in the CRU emulator case, but all the triggers are
// sent using this class's SendTrigger command which will be explained later.

int TReadoutBoardRUv1::Trigger(int nTriggers)
{


  if (emulator) {
    if (!triggerMode) {
      SendStartOfTriggered();
    }
    for (int i = 0; i < nTriggers; ++i) {
      // NOTE: SingleWait(n) waits for n clock cycles (160 MHz -> n*6.25 ns)
      if (m_triggerDelay > 0) wait_module->SingleWait(4 * m_triggerDelay);
      if (m_enableTrigger) {
        SendTrigger(1 << 4);
      }
    }
    if (triggerMode) {
      SendEndOfTriggered();
    }
  }
  else {
    if (!triggerMode) {
      gbtx_word_inject->StartTriggeredMode();
    }

    for (int i = 0; i < nTriggers; ++i) {

      if (m_triggerDelay > 0) wait_module->SingleWait(4 * m_triggerDelay);
      if (m_enableTrigger) {
        gbtx_word_inject->SendTrigger(1 << 4);
      }
    }
    if (!triggerMode) {
      gbtx_word_inject->StartTriggeredMode(false);
    }
  }


  return 0;
}


// ReadEventData: Takes the first element (EXACTLY ONE EVENT) of m_events, copies it to the buffer,
// records its size, then deletes that element from m_events and returns the size of the event.

int TReadoutBoardRUv1::ReadEventData(int &NBytes, unsigned char *Buffer)
{

  // if(m_events.empty()) fetchEventData();
  if (!m_events.empty()) {
    auto event = m_events.front();
    m_events.pop_front();

    std::copy(begin(event), end(event), Buffer);
    NBytes = event.size();
  }
  else {
    return 0;
  }

  return NBytes;
}

// ReadFrameData: Copies the first element of m_gbtFrames to the buffer, returns the size of the
// frame
int TReadoutBoardRUv1::ReadFrameData(int &NBytes, unsigned char *Buffer)
{
  if (!m_gbtFrames.empty()) {
    auto frame = m_gbtFrames.front();
    m_gbtFrames.erase(m_gbtFrames.begin());

    std::copy(begin(frame), end(frame), Buffer);
    NBytes = frame.size();
  }
  else {
    return 0;
  }
  return NBytes;
}

// Initialize: Lets gbtx_word_inject take control of the clock/data/strobe from the gbtx if we're
// doing USB readout, then sets the linkspeed of all the enabled transceivers. ALSO HAS CURRENT
// WORKAROUND FOR ELASTIC BUFFER ISSUE, MUST REMOVE WHEN FIXED!!

int TReadoutBoardRUv1::Initialize(int linkspeed)
{

  if (!emulator) gbtx_word_inject->TakeControl();
  dctrl->SetConnector(m_config->getConnector());

  if (linkspeed == 600) {
    SetLinkspeed(600);
  }

  else if (linkspeed == 1200) {
    SetLinkspeed(1200);
  }

  else
    std::cout << "INVALID LINKSPEED, VALUES ARE 600 AND 1200\n";

  ElasticBufferFix(linkspeed);
  return 0;
}

// PrepareTransceivers: Aligns each enabled transceiver, then enables data forwarding on that
// transceiver. If the alignment process fails, the whole program is aborted (usually this means the
// chip linkspeed and the transciever linkspeed were not the same, or you have the linkspeed set to
// 1200 and the chip driver pre-emphasis is 0...)

void TReadoutBoardRUv1::PrepareTransceivers()
{
  for (size_t i = 0; i < fChipPositions.size(); i++) {

    if (fChipPositions.at(i).enabled) {
      int receiverNum = fChipPositions.at(i).receiver;
      if (receiverNum < 9) {
        gth->EnableAlignmentSingle(receiverNum);
        int maxRetries = 50;
        int retries    = 0;
        while (retries < maxRetries) {
          if (!((1 << receiverNum) & (gth->Read(1)))) {
            usleep(10000);
            retries++;
          }
          else
            retries = maxRetries;
        }
        if (!((1 << receiverNum) & (gth->Read(1)))) {
          std::cout << "TRANSCEIVER " << receiverNum << "  STILL NOT ALIGNED AFTER MAX RETRIES \n";
          abort();
        }

        gth->EnableDataSingle(receiverNum);
      }

      else {
        uint8_t gpioReceiver = receiverNum - 9;
        gpio->EnableAlignmentSingle(gpioReceiver);
        int maxRetries = 5;
        int retries    = 0;
        while (retries < maxRetries) {
          if (!((1 << gpioReceiver) & (gpio->Read(1)))) {
            usleep(10000);
            retries++;
          }
          else
            retries = maxRetries;
        }
        if (!((1 << gpioReceiver) & (gpio->Read(1)))) {
          std::cout << "TRANSCEIVER " << receiverNum << "  STILL NOT ALIGNED AFTER MAX RETRIES \n";
          abort();
        }
        gpio->EnableDataSingle(gpioReceiver);
      }
    }
  }
}

// StartRun: initiates the readout process, starts a new thread that continously reads out from the
// USB port. Process is: Clean ports->Start thread-> enable data forward on packers-> prepare
// transceivers-> mask packer lanes to avoid timeouts.

void TReadoutBoardRUv1::StartRun()
{

  std::cout << "Clean ports\n";
  UsbDev::DataBuffer buf;
  readFromPort(TReadoutBoardRUv1::EP_DATA0_IN, 1024 * 10000, buf);
  if (buf.size() != 0)
    std::cout << "THERE WAS JUNK IN THE PORT BEFORE TEST \n" << buf.size() << " BYTES \n";
  readFromPort(TReadoutBoardRUv1::EP_DATA1_IN, 1024 * 10000, buf);
  std::cout << "Clean ports done\n";

  readoutBG = true;

  readThreads.push_back(std::thread(&TReadoutBoardRUv1::ContinuousRead, this));

  usleep(1000);

  if (emulator) {

    gbt_packer_gpio->EnableDataForward(0);
    gbt_packer_gth->EnableDataForward(1);
    gth->Initialize(true);
    gpio->Initialize(true);
    usleep(1000);
    PrepareTransceivers();
    MaskPackerLanes();
  }

  else {
    gbt_packer_gpio->EnableDataForward(0);
    gbt_packer_gth->EnableDataForward(1);
    gth->Initialize(true);
    gpio->Initialize(true);
    usleep(1000);
    PrepareTransceivers();
    MaskPackerLanes();
  }
}

// SetupSensors: This is was an old function to debug disrepencies between the python scripts and
// this software, but it can still be used if you would like to setup the sensors in exactly the
// same way the python scripts do (digital pulsing with ONLY pulse commands being received- chip
// generates the strobe internally)

void TReadoutBoardRUv1::SetupSensors()
{

  uint8_t broadChipID = 0x0f;

  // ch.reset()
  dctrl->SendOpCode(Alpide::OPCODE_GRST);

  // self.rdo.gth.initialize()
  gth->Initialize();

  // ch.initialize()
  uint16_t dataWrite =
      (((0x0f & 0xf) << 0) | ((true & 0x1) << 4) | ((1 & 0x1) << 5) | ((true & 0x1) << 6));

  dctrl->WriteChipRegister(Alpide::REG_CMUDMU_CONFIG, dataWrite, broadChipID);

  // ch.setreg_dtu_dacs
  dataWrite = (((8 & 0xf) << 0) | ((0x8 & 0xf) << 4) | ((0x8 & 0xf) << 8));

  dctrl->WriteChipRegister(Alpide::REG_DTU_DACS, dataWrite, broadChipID);

  // ch.setreg_dtu_cfg()
  uint8_t pll_off_sig[3] = {0, 1, 0};
  for (int i = 0; i < 3; i++) {
    dataWrite = (((1 & 0x3) << 0) | ((1 & 0x1) << 2) | ((pll_off_sig[i] & 0x1) << 3) |
                 ((8 & 0xf) << 4) | ((0 & 0x1) << 8) | ((0 & 0x1) << 12));

    dctrl->WriteChipRegister(Alpide::REG_DTU_CONFIG, dataWrite, broadChipID);
  }

  // ch.write_chip_opcode(RORST)
  dctrl->SendOpCode(Alpide::OPCODE_RORST);

  // ch.setreg_fromu_cfg_1()
  dataWrite = (((0 & 0x7) << 0) | ((0 & 0x1) << 3) | ((1 & 0x1) << 4) | ((0 & 0x1) << 5) |
               ((1 & 0x1) << 6) | ((0 & 0x1) << 7) | ((0 & 0x7) << 8));

  dctrl->WriteChipRegister(Alpide::REG_FROMU_CONFIG1, dataWrite, broadChipID);

  // ch.setreg_fromu_cfg_2()
  dataWrite = (((0x3 & 0xffff) << 0));

  dctrl->WriteChipRegister(Alpide::REG_FROMU_CONFIG2, dataWrite, broadChipID);

  // ch.setreg_fromu_cfg_3()
  dataWrite = (((0x9 & 0xffff) << 0));

  dctrl->WriteChipRegister(Alpide::REG_FROMU_CONFIG3, dataWrite, broadChipID);

  // ch.setreg_fromu_pulsing_2()
  dataWrite = (((0x8 & 0xffff) << 0));

  dctrl->WriteChipRegister(Alpide::REG_FROMU_PULSING2, dataWrite, broadChipID);

  // ch.setreg_fromu_pulsing_1()
  dataWrite = (((0x1 & 0xffff) << 0));

  dctrl->WriteChipRegister(Alpide::REG_FROMU_PULSING1, dataWrite, broadChipID);

  // ch.setreg_mode_ctrl()
  dataWrite = (((1 & 0x3) << 0) | ((1 & 0x1) << 2) | ((1 & 0x1) << 3) | ((3 & 0x3) << 4) |
               ((1 & 0x1) << 6) | ((1 & 0x1) << 7) | ((1 & 0x1) << 8) | ((0 & 0x1) << 9));

  dctrl->WriteChipRegister(Alpide::REG_MODECONTROL, dataWrite, broadChipID);
  // uint16_t val;
  // dctrl->ReadChipRegister(Alpide::REG_MODECONTROL, val, broadChipID);
  // std::cout << (val & 0x30) << std::endl;
}

// MaskPackerLanes: Masks the unused lanes in the GBT packer to avoid timeouts (has no effect if all
// 9 chips are connected)
void TReadoutBoardRUv1::MaskPackerLanes()
{

  gbt_packer_gth->MaskAllLanes();
  gbt_packer_gpio->MaskAllLanes();

  for (size_t i = 0; i < fChipPositions.size(); i++) {


    if (fChipPositions.at(i).enabled) {
      uint16_t receiverNum = fChipPositions.at(i).receiver;
      if (receiverNum < 9) {
        gbt_packer_gth->UnmaskLane(receiverNum);
      }

      else {
        gbt_packer_gpio->UnmaskLane(receiverNum - 9);
      }
    }
  }
}

// CleanUp: Stops the readout thread, "cleans" the packer out if there were timeouts, "cleans" the
// usb port and tells you if there was stuff in the port after the test (there shouldn't be...),
// then disables the transceivers, and if we're doing USB things the word_inject module releases
// control. Also clears m_events/m_gbtFrames in case you still have instances of the board software
// object running, like in the GUI.

void TReadoutBoardRUv1::CleanUp()
{
  readoutBG = false;

  for (size_t j = 0; j < readThreads.size(); j++) {
    readThreads[j].join();
  }
  readThreads.clear();

  UsbDev::DataBuffer buf;

  dctrl->SetMask();
  Trigger(100);
  usleep(1000);
  std::cout << "Cleaning out packer \n";
  while (readFromPort(TReadoutBoardRUv1::EP_DATA0_IN, 1024 * 10000, buf) > 0)
    ;
  std::cout << "Packer CLEAN\n";
  dctrl->SetMask(0x1f);

  gbt_packer_gth->EnableDataForward(0);
  gbt_packer_gpio->EnableDataForward(0);
  gth->EnableData(false);
  gth->EnableAlignment(false);
  gpio->EnableData(false);
  gpio->EnableAlignment(false);
  gbtx_word_inject->TakeControl(false);

  std::cout << "Clean ports\n";
  readFromPort(TReadoutBoardRUv1::EP_DATA0_IN, 1024 * 10000, buf);
  if (buf.size() != 0)
    std::cout << "THERE WAS STILL JUNK IN THE USB PORT AFTER TEST \n" << buf.size() << " BYTES\n";
  std::cout << "Clean ports done\n";

  m_events.clear();
  m_gbtFrames.clear();
}

// checkGitHash: COUTS the firmware githash of the board.

void TReadoutBoardRUv1::checkGitHash()
{
  registeredRead(TReadoutBoardRUv1::MODULE_STATUS, 0);
  registeredRead(TReadoutBoardRUv1::MODULE_STATUS, 1);

  flush();
  auto results = readResults();
  if (results.size() != 2) {
    if (m_logging)
      std::cout << "TReadoutBoardRUv1: Expected 2 results, got " << results.size() << "\n";
  }
  else {
    uint16_t lsb = results[0].data;
    uint16_t msb = results[1].data;
    std::cout << "Git hash: " << std::hex << msb << lsb << "\n" << std::dec;
  }
}

// ResetAllCounters: Resets "all" the monitor counters.
void TReadoutBoardRUv1::ResetAllCounters() // resets all relevant monitor counters
{
  dctrl->ResetCounters();
  datapathmon->ResetCounters();
  trigger_handler_monitor->ResetCounters();
  gbt_packer_monitor_gth->ResetCounters();
  gbtx_flow_monitor->ResetCounters();
  usb_if->ResetCounters();
}

// LatchAllCounters: Latches "all" the monitor counters.
void TReadoutBoardRUv1::LatchAllCounters() // latches all relevant monitor counters
{
  dctrl->LatchCounters();
  datapathmon->LatchCounters();
  trigger_handler_monitor->LatchCounters();
  gbt_packer_monitor_gth->LatchCounters();
  gbtx_flow_monitor->LatchCounters();
  usb_if->LatchCounters();
}

// DumpCounterConfig: Dumps all the counters in a messy way (but it gets the job done...)

void TReadoutBoardRUv1::DumpCounterConfig()
{
  dctrl->CoutAllCounters();
  datapathmon->CoutAllCounters();
  gbt_packer_monitor_gth->CoutAllCounters();
  gbtx_flow_monitor->DumpConfig();
  usb_if->DumpConfig();
}

// SetLinkspeed: Sets the TRANSCEIVER linkspeed for all enabled chips.

void TReadoutBoardRUv1::SetLinkspeed(int linkspeed)
{

  if (linkspeed == 600) {


    for (size_t i = 0; i < fChipPositions.size(); i++) {
      if (fChipPositions.at(i).enabled) {
        int      receiverNum = fChipPositions.at(i).receiver;
        uint16_t value       = drp_bridge->ReadDRP(receiverNum, 0x63);
        value                = (value & ~0x7) | 3;
        drp_bridge->WriteDRP(receiverNum, 0x63, value);
      }
    }
  }

  else if (linkspeed == 1200) {

    for (size_t i = 0; i < fChipPositions.size(); i++) {
      if (fChipPositions.at(i).enabled) {
        int      receiverNum = fChipPositions.at(i).receiver;
        uint16_t value       = drp_bridge->ReadDRP(receiverNum, 0x63);
        value                = (value & ~0x7) | 2;
        drp_bridge->WriteDRP(receiverNum, 0x63, value);
      }
    }
  }

  else
    std::cout << "INVALID LINKSPEED, VALUES ARE 600 AND 1200\n";
}

// This should only be called once during init setup, it takes a while to establish the link (~4
// seconds)
void TReadoutBoardRUv1::EmulateCRU()
{
  emulator = true;
  gbt_fpga->Write(4, 0);
  if ((gbt_fpga->Read(0) & (1 << 3)) == 0) {
    gbt_fpga->Write(2, 0, false);
    gbt_fpga->Write(0, 0x1);
    gbt_fpga->Write(0, 0x78);
    int nRet = 0;
    while (nRet < 1000) {
      if ((gbt_fpga->Read(0) & (1 << 3)) == 0) {
        nRet++;
        usleep(2500);
      }
    }
    if ((gbt_fpga->Read(0) & (1 << 3)) == 0) std::cout << "GBT RX NOT READY AFTER MAX RETRIES \n";
    abort();
  }
  gbt_fpga->Write(4, 1);
  std::cout << "CRU EMULATOR INITIALIZED" << std::endl;
  std::cout << "CRU GITHASH: " << std::hex << ((cru_status->Read(1) << 16) | cru_status->Read(0))
            << std::dec << std::endl;
}


// SendStartOfTriggered: Lets everyone know we're about to go into trigger mode. (mainly the GBT
// packer)
void TReadoutBoardRUv1::SendStartOfTriggered()
{
  if (emulator)
    SendTrigger(1 << 7);
  else
    gbtx_word_inject->StartTriggeredMode();
  triggerMode = true;
}

// SendEndOfTriggered: Lets everyone know we're done with the trigger mode. (mainly the gbt packer)
void TReadoutBoardRUv1::SendEndOfTriggered()
{
  if (emulator)
    SendTrigger(1 << 8);
  else
    gbtx_word_inject->StartTriggeredMode(false);
  triggerMode = false;
}

// SendTrigger: ONLY FOR CRU EMULATOR, SENDS A TRIGGER OF A GIVEN TYPE TO THE GBTX.

void TReadoutBoardRUv1::SendTrigger(uint16_t triggerType, uint16_t bc, uint32_t orbit, bool commit)
{
  registeredWrite(TReadoutBoardRUv1::MODULE_GBT_FPGA, 11, (triggerType & 0xffff));
  registeredWrite(TReadoutBoardRUv1::MODULE_GBT_FPGA, 12, ((triggerType >> 16) & 0xffff));
  registeredWrite(TReadoutBoardRUv1::MODULE_GBT_FPGA, 13, (bc & 0xfff));
  registeredWrite(TReadoutBoardRUv1::MODULE_GBT_FPGA, 14, (orbit & 0xffff));
  registeredWrite(TReadoutBoardRUv1::MODULE_GBT_FPGA, 15, (0x8000 | ((orbit >> 16) & 0x7fff)));
  if (commit) flush();
}


// TODO: REMOVE THIS WHOLE FUNCTION ONCE MASTER LANE BUG IS FIXED
void TReadoutBoardRUv1::ElasticBufferFix(int linkspeed)
{

  TAlpide *masterChip = fChipPositions.at(8).alpidePtr;
  masterChip->GetConfig()->SetParamValue("LINKSPEED", linkspeed);
  AlpideConfig::BaseConfig(masterChip, true);

  int      masterReceiver = 0;
  uint16_t value          = drp_bridge->ReadDRP(masterReceiver, 0x63);
  if (linkspeed == 1200) value = (value & ~0x7) | 2;
  if (linkspeed == 600) value = (value & ~0x7) | 3;

  drp_bridge->WriteDRP(masterReceiver, 0x63, value);
  gth->Initialize(true);
  usleep(100);

  uint16_t val = 0;
  masterChip->ReadRegister(Alpide::REG_MODECONTROL, val);
  val &= ((1 << 5) | (1 << 4));
  val = val >> 4;


  value = drp_bridge->ReadDRP(masterReceiver, 0x63);
  value &= 0x7;

  switch (val) {
  case (0):
    std::cout << "MASTER CHIP LINKSPEED IS 400, THIS WON'T WORK WITH RUv1!!\n";
    break;
  case (1):
    std::cout << "MASTER CHIP LINKSPEED IS 600!\n";
    break;
  case (2):
    std::cout << "MASTER CHIP LINKSPEED IS 1200!\n";
    break;
  case (3):
    std::cout << "MASTER CHIP LINKSPEED IS 1200!\n";
    break;
  default:
    std::cout << "WTF IS THIS SHIT? MASTER CHIP LINKSPEED MISREAD\n";
    break;
  }

  switch (value) {
  case (2):
    std::cout << "MASTER LANE LINKSPEED IS 1200!\n";
    break;
  case (3):
    std::cout << "MASTER LANE LINKSPEED IS 600!\n";
    break;
  default:
    std::cout << "MASTER LANE LINKSPEED MISREAD OR ABNORMAL\n";
    break;
  }

  gth->EnableAlignmentSingle(masterReceiver);
  int maxRetries = 50;
  int retries    = 0;
  while (retries < maxRetries) {
    if (!((1 << masterReceiver) & (gth->Read(1)))) {
      usleep(10000);
      retries++;
    }
    else
      retries = maxRetries;
  }
  if (!((1 << masterReceiver) & (gth->Read(1)))) {
    std::cout << "MASTER LANE NOT ALIGNED AFTER MAX RETRIES\n";
    abort();
  }
  gth->EnableDataSingle(masterReceiver);
  gth->EnableData(false);
  gth->EnableAlignment(false);
}

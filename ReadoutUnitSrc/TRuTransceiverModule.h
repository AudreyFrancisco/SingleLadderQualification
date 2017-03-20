#ifndef TRUTRANSCEIVERMODULE_H
#define TRUTRANSCEIVERMODULE_H

#include <map>
#include <string>

#include "TRuWishboneModule.h"

#include "../TBoardConfigRU.h"

// Wishbone mapping:
//
//    0 -> DRP_ADDRESS: Address for Debug reconfiguration Port connection; R/W
//    1 -> DRP_DATA     Data for Debug reconfiguration Port: Access to
//                      This port triggers a DRP transaction
//    2 -> TRANSCEIVER Settings: R/W
//                     Bit0     : readout speed mode: 0: Inner barrel, 1: Outer barrel
//                     Bit1     : Input polarity: 0: normal, 1: inverted
//                     Bit2     : 1: Full GTX Receiver reset
//                     Bit3     : 1: Reset PMA subblock
//                     Bit4     : 1: Reset PCS subblock
//                     Bit5     : 1: Reset PRBS counter
//                     Bit  7:6 : NOT USED
//                     Bit 10:8 : PRBS pattern (0 -> prbs check off,
//                                            1 -> PRBS-7,
//                                            2 -> PRBS-15,
//                                            3 -> PRBS-23,
//                                            4 -> PRBS-31)
//
//    3 -> TRANSCEIVER Status: Status of receiver; Read only
//                             Bit 0: RX startup fsm done
//                             Bit 1: RX reset is done
//                             Bit 2: CPLL locked status
//
//    4 -> RUN Settings : Settings for data readout and run configuration: R/W
//                        Bit0: Run readout
//                        Bit1: allow bit/word allignment (0: freezes
//                        allignment in place, default: 1)
//    5 -> RUN Status: Status of data readout; Read only
//                     Bit0: Data is comma aligned
//                     Bit1: Chip is in BUSY state
//    6 -> COUNTER Reset: Reset status and error counters; R/W
//                        Bit  0: 8b10b decoder, Code error counter
//                        Bit  1: 8b10b decoder, Disparity counter
//                        Bit  2: Idle suppress module, idle counter
//                        Bit  3: Idle suppress module, overflow counter
//                        Bit  4: Idle suppress module, full counter
//                        Bit  5: Protocol checker module, event counter
//                        Bit  6: Protocol checker module, event error counter
//                        Bit  7: Protocol checker module, error counter
//                        Bit  8: Protocol checker module, busy violation counter
//                        Bit  9: Protocol checker module, double busy on counter
//                        Bit 10: Protocol checker module, double busy off counter
//                        Bit 11: Protocol checker module, empty region counter
//                        Bit 12: Protocol checker module, busy cycle counter
//                        Bit 13: Prbs Error Counter (fabric)
//
//    7 -> 8b10b Decoder, Errorcounter
//    8 -> 8b10b Decoder, Disparity counter
//    9 -> Idlesuppress, Idle counter
//    10 -> Idlesuppress, Overflow counter
//    11 -> Idlesuppress, Full counter
//    12 -> Dataformat, timeout
//    13 -> Events, Event counter
//    14 -> Events, Event error counter
//    15 -> Events, Error counter
//    16 -> Events, Busy violation counter
//    17 -> Events, Double busy on counter
//    18 -> Events, Double busy off counter
//    19 -> Events, Empty region counter
//    20 -> Events, Busy cycle counter
//    21 -> PRBS Error Counter (fabric)
//    32 -> LVDS Settings: R/W
//          Bit 0   : Receiver Enable
//          Bit 1   : Load Delay configuration
//          Bit 2   : Invert Polarity
//          Bit 11:3: Delay config (value of delay tap)
//
// Startup sequence
// 1) Set Transceiver settings (IB/OB mode)
// 2) set Run settings: Run -> 1, Allow allignment -> 1
// 3) Readout data from dataport

class TRuTransceiverModule : public TRuWishboneModule {
public:
    static const uint8_t DRP_ADDRESS = 0;
    static const uint8_t DRP_DATA = 1;
    static const uint8_t TRANSCEIVER_SETTINGS = 2;
    static const uint8_t RUN_SETTINGS = 4;
    static const uint8_t RUN_STATUS = 5;
    static const uint8_t COUNTER_RESET = 6;


  TRuTransceiverModule(TReadoutBoardRU &board, uint16_t moduleId, bool logging)
      : TRuWishboneModule(board, moduleId, logging) {}
  int Initialize(TBoardConfigRU::ReadoutSpeed RoSpeed, bool InvertPolarity);
  void DeactivateReadout();
  void ResetReceiver();
  void SetupPrbsChecker(uint8_t pattern=1);
  void ActivateReadout(bool Activate = true);
  void AllowAlignment(bool Allow=true);
  bool IsAligned();

  void ResetCounters();
  std::map<std::string, uint16_t> ReadCounters();
private:
  void WriteDrp(uint16_t Address, uint16_t Data);
  uint16_t ReadDrp(uint16_t Address);
  void SetRxOutDiv(uint8_t div);
};

#endif // TRUTRANSCEIVERMODULE_H

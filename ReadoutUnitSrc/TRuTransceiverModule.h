#ifndef TRUTRANSCEIVERMODULE_H
#define TRUTRANSCEIVERMODULE_H

#include <map>
#include <string>

#include "TRuWishboneModule.h"

#include "../TBoardConfigRU.h"

class TRuTransceiverModule : public TRuWishboneModule {
public:
    static const uint16_t DRP_ADDRESS = 0;
    static const uint16_t DRP_DATA = 1;


  TRuTransceiverModule(TReadoutBoardRU &board, uint16_t moduleId, bool logging)
      : TRuWishboneModule(board, moduleId, logging) {}
  int Initialize(TBoardConfigRU::ReadoutSpeed ro_speed, bool invertPolarity);
  void DeactivateReadout();
  void ResetReceiver();
  void SetupPrbsChecker();
  void ActivateReadout();
  void AllowAlignment();
  bool IsAligned();

  void ResetCounters();
  std::map<std::string, uint16_t> ReadCounters();
private:
  void WriteDrp(uint16_t Address, uint16_t Data);
  uint16_t ReadDrp(uint16_t Address);
  void SetRxOutDiv(uint8_t div);
};

#endif // TRUTRANSCEIVERMODULE_H

#ifndef TRUTRANSCEIVERMODULE_H
#define TRUTRANSCEIVERMODULE_H

#include <map>
#include <string>

#include "TRuWishboneModule.h"

#include "../TBoardConfigRU.h"

class TRuTransceiverModule : public TRuWishboneModule {
public:
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
};

#endif // TRUTRANSCEIVERMODULE_H

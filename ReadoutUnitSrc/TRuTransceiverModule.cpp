#include "TRuTransceiverModule.h"

int  TRuTransceiverModule::Initialize(TBoardConfigRU::ReadoutSpeed ro_speed, bool invertPolarity){}
void TRuTransceiverModule::DeactivateReadout(){}
void TRuTransceiverModule::ResetReceiver(){}
void TRuTransceiverModule::SetupPrbsChecker(){}
void TRuTransceiverModule::ActivateReadout(){}
void TRuTransceiverModule::AllowAlignment(){}
bool TRuTransceiverModule::IsAligned(){}

void TRuTransceiverModule::ResetCounters(){}
std::map<std::string, uint16_t> TRuTransceiverModule::ReadCounters(){}

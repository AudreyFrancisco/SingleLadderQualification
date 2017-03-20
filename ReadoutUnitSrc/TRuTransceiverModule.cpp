#include "TRuTransceiverModule.h"

int  TRuTransceiverModule::Initialize(TBoardConfigRU::ReadoutSpeed ro_speed, bool invertPolarity){
    switch(ro_speed) {
    case TBoardConfigRU::ReadoutSpeed::RO_1200:

        break;
    case TBoardConfigRU::ReadoutSpeed::RO_600: break;
    case TBoardConfigRU::ReadoutSpeed::RO_400: break;
    }
    return 0;
}
void TRuTransceiverModule::DeactivateReadout(){}
void TRuTransceiverModule::ResetReceiver(){}
void TRuTransceiverModule::SetupPrbsChecker(){}
void TRuTransceiverModule::ActivateReadout(){}
void TRuTransceiverModule::AllowAlignment(){}
bool TRuTransceiverModule::IsAligned(){}

void TRuTransceiverModule::ResetCounters(){}
std::map<std::string, uint16_t> TRuTransceiverModule::ReadCounters(){}

void TRuTransceiverModule::WriteDrp(uint16_t Address, uint16_t Data) {
    Write(TRuTransceiverModule::DRP_ADDRESS,Address,false);
    Write(TRuTransceiverModule::DRP_DATA, Data,true);
}

uint16_t TRuTransceiverModule::ReadDrp(uint16_t Address) {
    Write(TRuTransceiverModule::DRP_ADDRESS,Address,false);
    return Read(TRuTransceiverModule::DRP_DATA,true);
}

void TRuTransceiverModule::SetRxOutDiv(uint8_t div) {

}

#include "TRuTransceiverModule.h"

#include <iostream>
#include <map>
#include <initializer_list>
#include <algorithm>

#include "TReadoutBoardRU.h"

int  TRuTransceiverModule::Initialize(TBoardConfigRU::ReadoutSpeed RoSpeed, bool InvertPolarity){
    uint8_t transceiver_settings = 0;
    bool ob_mode = false;

    DeactivateReadout();

    switch(RoSpeed) {
    case TBoardConfigRU::ReadoutSpeed::RO_1200:
        SetRxOutDiv(4);
        break;
    case TBoardConfigRU::ReadoutSpeed::RO_600:
        SetRxOutDiv(8);
        break;
    case TBoardConfigRU::ReadoutSpeed::RO_400:
        SetRxOutDiv(4);
        ob_mode = true;
        break;
    }

    transceiver_settings |= (ob_mode)?1:0;
    transceiver_settings |= (InvertPolarity)?2:0;

    Write(TRuTransceiverModule::TRANSCEIVER_SETTINGS,transceiver_settings);

    AllowAlignment(true);
    ResetReceiver();

    return 0;
}
void TRuTransceiverModule::DeactivateReadout(){
    ActivateReadout(false);
}

void TRuTransceiverModule::ResetReceiver(){
    uint16_t data = Read(TRuTransceiverModule::TRANSCEIVER_SETTINGS);
    uint16_t data_rst_high = data | (1 << 2);
    uint16_t data_rst_low = data & ~(1<<2);
    Write(TRuTransceiverModule::TRANSCEIVER_SETTINGS,data_rst_high,false);
    Write(TRuTransceiverModule::TRANSCEIVER_SETTINGS,data_rst_low,true);
}

void TRuTransceiverModule::SetupPrbsChecker(uint8_t pattern){
    uint16_t data = Read(TRuTransceiverModule::TRANSCEIVER_SETTINGS);
    data |= pattern << 8;
}
void TRuTransceiverModule::ActivateReadout(bool Activate){
    uint16_t data = Read(TRuTransceiverModule::RUN_SETTINGS);

    if(Activate)
        data |= 1;
    else
        data &= ~1;

    Write(TRuTransceiverModule::RUN_SETTINGS,data);
}

void TRuTransceiverModule::AllowAlignment(bool Allow){
    uint16_t data = Read(TRuTransceiverModule::RUN_SETTINGS);
    if(Allow)
        data |= 2;
    else
        data &= ~2;

    Write(TRuTransceiverModule::RUN_SETTINGS,data);
}
bool TRuTransceiverModule::IsAligned(){
    uint16_t status = Read(TRuTransceiverModule::RUN_STATUS);
    return (status & 1) == 1;
}

void TRuTransceiverModule::ResetCounters(){
    Write(TRuTransceiverModule::COUNTER_RESET,0xFFFF,false);
    Write(TRuTransceiverModule::COUNTER_RESET,0,false);
}

std::map<std::string, uint16_t> TRuTransceiverModule::ReadCounters(){
    static std::vector<std::string> counterNames {"8b10b Code Error", "8b10b disparity Error", "Idlesupress idle counter",
                     "Idlesuppress overflow", "Idlesuppress Full", "Events NrEvents",
                     "Events EventErrors", "Events Errors", "Events Busyviolations",
                     "Events Double Busy On", "Events Double Busy Off", "Events Empty Regions", "Prbs Errors"};
    static std::vector<uint8_t> counterAddr {7,8,9,10,11,0xD,0xE,0xF,0x10,0x11,0x12,0x13,0x14};

    for(auto addr : counterAddr)
        Read(addr,false);
    m_board.flush();
    auto results = m_board.readResults();

    size_t resultSize = results.size();
    if(results.size() != counterAddr.size()) {
        if(m_logging)
            std::cout << "Mismatch in Result size: expected " << counterAddr.size() << ", got " << results.size() << "\n";

        resultSize = std::min(results.size(),counterAddr.size());
    }
    std::map<std::string,uint16_t> counterValues;

    for(int i = 0; i < resultSize; ++i) {
        auto result = results[i];
        auto name = counterNames[i];
        counterValues.emplace(name, result.data);
    }

    return counterValues;
}

void TRuTransceiverModule::WriteDrp(uint16_t Address, uint16_t Data) {
    Write(TRuTransceiverModule::DRP_ADDRESS,Address,false);
    Write(TRuTransceiverModule::DRP_DATA, Data,true);
}

uint16_t TRuTransceiverModule::ReadDrp(uint16_t Address) {
    Write(TRuTransceiverModule::DRP_ADDRESS,Address,false);
    return Read(TRuTransceiverModule::DRP_DATA,true);
}

void TRuTransceiverModule::SetRxOutDiv(uint8_t div) {
    uint16_t data = ReadDrp(0x0088);
    data &= 0x7;
    switch(div) {
    case 1: data |= 0; break;
    case 2: data |= 1; break;
    case 4: data |= 2; break;
    case 8: data |= 3; break;
    case 16:data |= 4; break;
    default: if(m_logging)
            std::cout << "Invalid RxOutDiv\n";
    }
    WriteDrp(0x0088,data);
}

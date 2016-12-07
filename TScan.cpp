#include "TScan.h"


TScan::TScan (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards) 
{
  m_config = config;
  m_chips  = chips; 
  m_boards = boards;

}


TMaskScan::TMaskScan (TScanConfig *config, std::vector <TAlpide *> chips, std::vector <TReadoutBoard *> boards) 
: TScan(config, chips, boards)
{
}


#include "TRUv1WishboneModule.h"
#include "TReadoutBoardRUv1.h"
#include <iostream>

void TRUv1WishboneModule::Write(uint8_t address, uint16_t data, bool commit)
{
  m_board.registeredWrite(m_moduleId, address, data);
  if (commit) m_board.flush();
}

uint16_t TRUv1WishboneModule::Read(uint8_t address, bool commit)
{
  m_board.registeredRead(m_moduleId, address);
  if (commit) {
    m_board.flush();
    auto results = m_board.readResults();
    if (results.size() != 1) {
      if (m_logging)
        std::cout << "TReadoutBoardRUv1: Expected 1 result, got " << results.size() << "\n";
      return 0;
    }
    else {
      return results[0].data;
    }
  }
  else {
    return 0;
  }
}

#ifndef TRUWISHBONEMODULE_H
#define TRUWISHBONEMODULE_H

#include <cstdint>

class TReadoutBoardRU;

class TRuWishboneModule {
protected:
  TReadoutBoardRU &m_board;
  uint8_t          m_moduleId;
  bool             m_logging;

public:
  TRuWishboneModule(TReadoutBoardRU &board, uint8_t moduleId, bool logging = false)
      : m_board(board), m_moduleId(moduleId), m_logging(logging)
  {
  }
  void     Write(uint8_t address, uint16_t data, bool commit = true);
  uint16_t Read(uint8_t address, bool commit = true);
};

#endif // TRUWISHBONEMODULE_H

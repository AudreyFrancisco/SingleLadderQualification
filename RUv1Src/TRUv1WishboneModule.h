#ifndef TRUV1WISHBONEMODULE_H
#define TRUV1WISHBONEMODULE_H

#include <cstdint>

class TReadoutBoardRUv1;

class TRUv1WishboneModule {
protected:
  TReadoutBoardRUv1 &m_board;
  uint8_t            m_moduleId;
  bool               m_logging;

public:
  TRUv1WishboneModule(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false)
      : m_board(board), m_moduleId(moduleId), m_logging(logging)
  {
  }
  void     Write(uint8_t address, uint16_t data, bool commit = true);
  uint16_t Read(uint8_t address, bool commit = true);
};

#endif // TRUV1WISHBONEMODULE_H

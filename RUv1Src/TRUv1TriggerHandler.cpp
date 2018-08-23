#include "TRUv1TriggerHandler.h"

#include <iostream>

#include "TReadoutBoardRUv1.h"

TRUv1TriggerHandler::TRUv1TriggerHandler(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)
    : TRUv1WishboneModule(board, moduleId, logging)
{
}

void TRUv1TriggerHandler::DumpConfig()
{
  std::cout << "....TRUV1TRIGGERHANDLER MODULE CONFIG....\n";
  for (int i = 0; i < 6; i++) {
    std::cout << "ADDRESS " << i << " HAS VALUE " << Read(i, true) << "\n";
  }
}

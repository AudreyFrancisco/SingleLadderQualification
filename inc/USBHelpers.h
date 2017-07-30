#ifndef _USBHELPERS_H_ 
#define _USBHELPERS_H_

#include <libusb-1.0/libusb.h>
#include <vector> 
#include "TReadoutBoardDAQ.h"
#include "TReadoutBoard.h"

static struct libusb_context *fContext = 0;

int  InitLibUsb    ();
bool IsDAQBoard    (libusb_device *device);
int  AddDAQBoard   (libusb_device *device, TConfig *config, std::vector <TReadoutBoard *> * boards);
int  FindDAQBoards (TConfig *config, std::vector <TReadoutBoard *> * boards);

#endif    /* _USBHELPERS_H_ */


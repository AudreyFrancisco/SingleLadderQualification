#include "USBHelpers.h"
#include "USB.h"

int InitLibUsb() {
    int err = libusb_init(&fContext);
    if (err) {
        std::cout << "Error " << err << " while trying to init libusb " << std::endl;
    }
    return err;
}


bool IsDAQBoard(libusb_device *device) {

    libusb_device_descriptor desc;
    libusb_get_device_descriptor(device, &desc);

    // std::cout << std::hex << "Vendor id " << (int)desc.idVendor << ", Product id " << (int)desc.idProduct << std::dec << std::endl;

    if ((desc.idVendor == DAQ_BOARD_VENDOR_ID) && (desc.idProduct == DAQ_BOARD_PRODUCT_ID)) {
      //std::cout << "Serial number " << (int)desc.iSerialNumber << std::endl;
        return true;
    }

    return false;
}


int AddDAQBoard (libusb_device *device, TConfig *config, std::vector <TReadoutBoard *> &boards) {
    TReadoutBoard *readoutBoard;
    // note: this should change to use the correct board config according to index or geographical id
    readoutBoard = new TReadoutBoardDAQ(device, config->GetBoardConfig(0));
    
    if (readoutBoard) {
        boards.push_back(readoutBoard);
    std::cout << "boards.size = " << boards.size() <<std::endl;
        return 0;
    }
    else {
        return -1;
    }
}


int FindDAQBoards (TConfig *config, std::vector <TReadoutBoard *> &boards) {
    int err     = 0;
    libusb_device **list;

    if (fContext == 0) {                       // replace by exception
      std::cout << "Error, libusb not initialised" << std::endl;
      return -1;
    }
    ssize_t cnt = libusb_get_device_list(fContext, &list);

    if (cnt < 0) {
        std::cout << "Error getting device list" << std::endl;
        return -1;
    }

    for (ssize_t i = 0; i < cnt; i++) {
        libusb_device *device = list[i];
        if (IsDAQBoard(device)) {
	  err = AddDAQBoard(device, config, boards);
            if (err) {
                std::cout << "Problem adding DAQ board" << std::endl;
                libusb_free_device_list(list, 1);
                return err;
            }
        }
    }
    std::cout << "boards.size = " << boards.size() <<std::endl;
    libusb_free_device_list(list, 1);
    return err;
}


//-----------------------------------------------------------------------------
// Title      : Usb Device abstraction
// Project    : ALICE ITS WP10
//-----------------------------------------------------------------------------
// File       : UsbDev.hpp
// Author     : Matthias Bonora (matthias.bonora@cern.ch)
// Company    : CERN / University of Salzburg
// Created    : 2015-11-26
// Last update: 2015-11-26
// Platform   : CERN 7 (CentOs)
// Target     : Linux / Windows
// Standard   : C++11
//-----------------------------------------------------------------------------
// Description: simple Class abstraction of libusb library
//-----------------------------------------------------------------------------
// Copyright (c)   2015
//-----------------------------------------------------------------------------
// Revisions  :
// Date        Version  Author        Description
// 2017/03/16  110      mbonora       Copied from RU repository, removed logging code/dependencies
//-----------------------------------------------------------------------------
#ifndef USBDEV_H
#define USBDEV_H

#include "TReadoutBoardRUv1.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

// libusb
#include <libusb-1.0/libusb.h>

/*! Device abstraction for Usb connection
  Encapsulates Device handles, performes allocation and deallocation,
  provides functions to read/write from an endpoint
*/
class UsbDev {
public:
  UsbDev(size_t VID = 0x04b4, size_t PID = 0x0008, int interface = 2,
         std::string serialNumber = std::string(), uint32_t dipSwitch = 1023)
      : m_VID(VID), m_PID(PID), m_interface(interface), m_serialNumber(serialNumber),
        m_dipSwitch(dipSwitch), m_deviceConnected(false), m_ctx(0, libusb_exit),
        m_device(0, std::bind(&UsbDev::closeDevice, this, std::placeholders::_1))
  {

    initLibUsb();
    initDevice();
    claimInterface();
  }

  typedef std::vector<unsigned char> DataBuffer;
  typedef std::shared_ptr<UsbDev>    Ptr;

  /*! Read Data from Usb Endpoint and write it to DataBuffer

    \param endpoint Endpoint number

    \param data The data buffer. Initial size describes the data size
                to read from usb. After the tranfer, the buffer is
                resized to the actual size read.

    \throw Exception if an error occures (Timeout does not count as error)
   */
  int readData(uint8_t endpoint, DataBuffer &data, size_t timeout)
  {
    int actualRead = 0;
    int ret = libusb_bulk_transfer(m_device.get(), (endpoint | LIBUSB_ENDPOINT_IN), &data.front(),
                                   data.size(), &actualRead, timeout);

    if (!(ret == 0 || ret == LIBUSB_ERROR_TIMEOUT)) {
      checkError(ret);
    }
    data.resize(actualRead);
    return actualRead;
  }

  /*! Write Data to USB endpoint
    \param endpoint Endpoint number

    \param data The data buffer. Initial size describes the data size
                to write to usb.

    \return the actual number of bytes written to usb
    \throw Exception if an error occures
   */
  int writeData(uint8_t endpoint, DataBuffer &data, size_t timeout)
  {
    int actualWritten = 0;

    int ret = libusb_bulk_transfer(m_device.get(), (endpoint | LIBUSB_ENDPOINT_OUT), &data.front(),
                                   data.size(), &actualWritten, timeout);

    if (!(ret == 0 || ret == LIBUSB_ERROR_TIMEOUT)) {
      checkError(ret);
    }
    return actualWritten;
  }

private:
  size_t      m_VID;
  size_t      m_PID;
  int         m_interface;
  std::string m_serialNumber;
  uint32_t    m_dipSwitch;
  bool        m_deviceConnected;

  struct ReadResult {
    uint16_t address;
    uint16_t data;
    bool     error;
    ReadResult(uint16_t address, uint16_t data, bool error)
        : address(address), data(data), error(error)
    {
    }
  };

  /*! Pointer Handle for usb context. Calls libusb_exit on destruction*/
  std::unique_ptr<libusb_context, decltype(&libusb_exit)> m_ctx;

  /*! Pointer Handle for device. On destruction releases the interface and
   * closes the device*/
  std::unique_ptr<libusb_device_handle, std::function<void(libusb_device_handle *)>> m_device;

  /*! Closes device connection.
     This includes releasing the interface and deleting the device handle
   */
  void closeDevice(libusb_device_handle *dev_handle)
  {
    if (dev_handle != nullptr) {
      int ret = libusb_release_interface(dev_handle, m_interface);
      if (ret != 0) {
        // could not release
        std::cout << "Could not release Usb Interface (Errorcode: " << ret << ")\n";
      }
      libusb_close(dev_handle); // close
    }
  }

  /*! Initialize libUsb environment */
  void initLibUsb()
  {
    libusb_context *ctx = NULL;
    // Initialize libusb context
    checkError(libusb_init(&ctx));
    m_ctx.reset(ctx);
  }

  /*! Initializes the USB device */
  void initDevice()
  {
    libusb_device_handle *dev_handle;

    if (m_serialNumber.empty()) {
      // open device with correct VID and PID
      dev_handle = libusb_open_device_with_vid_pid(m_ctx.get(), m_VID, m_PID);
      if (dev_handle == NULL) {
        throw std::runtime_error("Could not open device");
      }
      m_device.reset(dev_handle);
    }
    else {
      findRUv1();
    }
    m_deviceConnected = true;
  }

  /* Find device with VID, PID and serial number.
     Throw runtime_error if not found.
     Throw runtime_error on usb errors.
     return libusb_device_handle * to correct device
   */
  void findDevice()
  {
    libusb_device **list = NULL;

    try {
      bool    deviceFound = false;
      ssize_t nrDevices   = libusb_get_device_list(m_ctx.get(), &list);
      if (nrDevices < 0) {
        checkError(nrDevices);
      }

      for (ssize_t idx = 0; idx < nrDevices && !deviceFound; ++idx) {
        libusb_device *          device = list[idx];
        libusb_device_descriptor desc   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        libusb_device_handle *   dev_handle;

        checkError(libusb_get_device_descriptor(device, &desc));

        if (desc.idVendor == m_VID && desc.idProduct == m_PID) {

          checkError(libusb_open(device, &dev_handle));
          m_device.reset(dev_handle);

          size_t      maxLen = 256;
          std::string str;
          if (desc.iSerialNumber > 0) {
            std::vector<unsigned char> dataVec(maxLen);
            int len = libusb_get_string_descriptor_ascii(m_device.get(), desc.iSerialNumber,
                                                         &dataVec.front(), maxLen);
            if (len < 0) {
              checkError(len);
            }
            else {
              dataVec.resize(len);
              str.assign(dataVec.begin(), dataVec.end() - 1);
            }
          }
          std::cout << str << " SERIAL NUMBER?" << std::endl;
          if (m_serialNumber == str) {
            deviceFound = true;
          }
        }
      }
      if (!deviceFound) {
        throw std::runtime_error("Could not open device");
      }
      libusb_free_device_list(list, 0);
    }
    catch (std::runtime_error &e) {
      libusb_free_device_list(list, 0);
      throw e;
    }
  }

  void findRUv1()
  {
    libusb_device **list = NULL;

    try {
      bool    deviceFound = false;
      ssize_t nrDevices   = libusb_get_device_list(m_ctx.get(), &list);
      if (nrDevices < 0) {
        checkError(nrDevices);
      }

      for (ssize_t idx = 0; idx < nrDevices && !deviceFound; ++idx) {
        libusb_device *          device = list[idx];
        libusb_device_descriptor desc   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        libusb_device_handle *   dev_handle;

        checkError(libusb_get_device_descriptor(device, &desc));

        if (desc.idVendor == m_VID && desc.idProduct == m_PID) {

          if(!isOpen(libusb_open(device, &dev_handle))) continue;
          m_device.reset(dev_handle);
          DataBuffer buffDude;
          uint16_t   module   = 1;
          uint16_t   address  = 6;
          uint8_t    mod_byte = module & 0x7F;
          uint8_t    add_byte = address & 0xFF;

          buffDude.push_back(0);
          buffDude.push_back(0);
          buffDude.push_back(add_byte);
          buffDude.push_back(mod_byte);

          writeData(3, buffDude, 5);

          buffDude.clear();

          DataBuffer buffer_all, buffer_single;
          int        retries   = 0;
          size_t     data_left = 4;
          while (buffer_all.size() < 4 && retries < 5) {

            buffer_single.resize(roundUpToMultipleV1(data_left, 1024));
            readData(3, buffer_single, 5);
            buffer_all.insert(buffer_all.end(), buffer_single.begin(), buffer_single.end());

            if (buffer_all.size() < 4) {
              data_left = 4 - buffer_all.size();
              ++retries;
            }
          }
          if (buffer_all.size() < 4) {

            std::cout << "TReadoutBoardRUv1: could not read all data from Control Port. "
                         "Packet dropped";
          }

          // Process read data
          std::vector<ReadResult> results;
          for (size_t i = 0; i < buffer_all.size(); i += 4) {
            uint16_t data       = buffer_all[i] | (buffer_all[i + 1] << 8);
            uint16_t address    = buffer_all[i + 2] | (buffer_all[i + 3] << 8);
            bool     read_error = (address & 0x8000) > 0;
            address &= 0x7FFF;
            if (read_error) {
              std::cout << "TReadoutBoardRUv1: Wishbone error while reading: Address " << address
                        << "\n";
            }

            results.emplace_back(address, data, read_error);
          }

          if (results[0].data == m_dipSwitch) deviceFound = true;
        }
      }

      if (!deviceFound) {
        throw std::runtime_error("Could not open device");
      }
      libusb_free_device_list(list, 0);
    }
    catch (std::runtime_error &e) {
      libusb_free_device_list(list, 0);
      throw e;
    }
  }


  /*! Claims the usb interface; if required, detaches a kernel driver */
  void claimInterface()
  {
    if (libusb_kernel_driver_active(m_device.get(), m_interface) == 1) {
      checkError(libusb_detach_kernel_driver(m_device.get(), 0));
    }

    checkError(libusb_claim_interface(m_device.get(), m_interface));
  }

  /*! Check the return code for errors, throw an exception if the result
    was not OK
   */
  void checkError(int retCode)
  {
    if (retCode != 0) {
      if (retCode == LIBUSB_ERROR_NO_DEVICE) {
        m_deviceConnected = false;
      }
      std::ostringstream err_msg;
      err_msg << "Libusb returned with error code " << retCode;
      throw std::runtime_error(err_msg.str());
    }
  }

  bool isOpen(int retCode) 
  {
    if(retCode != 0) {
      return false;
    }
    return true;
  }

  int roundUpToMultipleV1(int numToRound, int multiple)
  {
    if (multiple == 0) return numToRound;

    int remainder = numToRound % multiple;
    if (remainder == 0) return numToRound;

    return numToRound + multiple - remainder;
  }
};

#endif // USBDEV_H

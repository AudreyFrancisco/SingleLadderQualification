
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
from future_builtins import *
from ctypes import *

import os
import sys
import random
import math
import argparse
import time
import serial

def initplate():
    sbeam=serial.Serial("/dev/ttyUSB0",115200, bytesize=8, parity='N', stopbits=2, timeout=1)
    if sbeam.isOpen :
        print("Beam controlling port open")
        return 0
    else :
        print("ERROR: Beam controlling port not connected! Closing..")
        return -1
#        sys.exit(-1)

def movehigh():
    plateok = 1
    while 1:
      for i in range(1, 6):
        print("Setting {} Plane to High position".format(i))
        command = "*B1OS{}H\r".format(i).encode()
        sbeam.write(command)
        print("Setting {} Plane to High position completed".format(i))
        readback=sbeam.readline()
        if readback != "*B10\r":
          print("ERROR: Not Received OK status from Controller!")
          print("Will try again..")
          plateok = 0
        else :
          print("Received OK from Controller")

      for i in range(1, 6):
        print("This command will be sent -> *B1IR{}\r".format(i))
        command = "*B1IR{}\r".format(i).encode()
        sbeam.write(command)
        message = sbeam.readline()
        if message == '*B10H\r':
          print("The plate Nt.{} is in High position".format(i))
        elif message == '*B10L\r':
          print("The plate Nt.{} is in Low position".format(i))
          print("Will try again to move plate to high position..")
          plateok = 0
      if plateok == 1:
        break
    return 0

def movelow()
    plateok = 1
    while 1:
      print("Setting Plane 1 to low position")
      command = "*B1OS1L\r".encode()
      sbeam.write(command)
      readback=sbeam.readline()
      if readback != "*B10\r":
        print("ERROR: Not Received OK status from Controller!")
        plateok = 0
      else :
        print("Received OK from Controller")
        print("Setting Plane {} to Low position completed".format(1))
      for i in range(3, 6):
        print("Setting Plane {} to Low position".format(i))
        command = "*B1OS{}L\r".format(i).encode()
        sbeam.write(command)
        readback=sbeam.readline()
        if readback != "*B10\r":
          print("ERROR: Not Received OK status from Controller!")
          plateok = 0
        else :
          print("Received OK from Controller")
          print("Setting Plane {} to Low position completed".format(i))
      if plateok == 1:
        for i in range(1, 6):
         print("This command will be sent -> *B1IR{}\r".format(i))
         command = "*B1IR{}\r".format(i).encode()
         sbeam.write(command)
         message = sbeam.readline()
         if message == '*B10H\r' and i != 2:
           print("The plate Nt.{} is in High position".format(i))
           print("Will try again to move plate to low position..")
           plateok = 0
         elif message == '*B10L\r':
           print("The plate Nt.{} is in Low position".format(i))
      if plateok == 1 :
          break

    return 0

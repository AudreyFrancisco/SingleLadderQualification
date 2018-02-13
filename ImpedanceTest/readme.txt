Installation
1) Connect the power supply to a USB port of the PC
2) Run command "dmesg" and check if a USB device corresponding to the power supply was detected
3) Check that the last message contains the text "FTDI USB Serial Device converter now attached to ttyUSB0". Note the name of attachment point if it's different from ttyUSB0. You will need to modify this name in the script.
4) Add your main user to dialout group. Remember to log out and log in to make the group inclusion effective.
5) Install python package "pyserial"
Running
1) Run ./IV-scan.py from the ImpedanceTest folder
2) When prompted enter the HIC name omitting the "OBHIC-". You can also read the barcode attached to the cover of the carrier plate.
3) Script will test DVDD, AVDD and BIAS circuits. Check the result for each circuit.

Comments and questions: serhiy.senyukov@cern.ch

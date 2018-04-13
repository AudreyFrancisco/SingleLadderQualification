Installation
1) Connect the power supply to a USB port of the PC
2) Switch on the USB interface of the power supply: Menu->Interface->Select Interface->USB
3) Run command "dmesg" and check if a USB device corresponding to the power supply was detected
4) Check that the last message contains the text "FTDI USB Serial Device converter now attached to ttyUSB0". Note the name of attachment point if it's different from ttyUSB0. You will need to modify this name in the script.
5) Check that the dialout group has write access to the attachment point
6) Add your main user to dialout group. Remember to log out and log in to make the group inclusion effective.
7) Install python package "pyserial" 

Database set-up
1) Create a database configuration file by copying DBConfig.cfg_example to DBConfig.cfg
2) Edit the DBConfig.cfg to set the following parameters:
    a) DBNAME=ITS
    b) LOCATION = 'Exact name of your institute as in the database'
    c) DBUSER = 'SURNAME NAME' of the user mostly doing the tests
    d) DBATTACHLIMIT=1   
    e) DBATTACHBASEPATH="/tmp/IVscan/"
    f) DBATTACHCOMMAND="echo %1 %2 %3"
    g) DBANEWDIRCOMMAND="echo %1"
    h) DBATTACHURIBASEPATH="https://cern.ch/hicTests/OBImpedance/SITE_NAME". Contact Serhiy Senyukov to know your SITE_NAME.
    
Running
1) Run ./IV-scan.py from the ImpedanceTest folder
2) When prompted enter the HIC name omitting the "OBHIC-". You can also read the barcode attached to the cover of the carrier plate.
3) Script will test DVDD, AVDD and BIAS circuits. Check the result for each circuit.

Comments and questions: serhiy.senyukov@cern.ch

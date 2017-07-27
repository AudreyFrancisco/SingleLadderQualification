#!/bin/bash

######################################################################################
#
# script for testing the power-up behaviour using the MOSAIC and a HAMEG power supply
#
######################################################################################

### Settings

N_TRIALS=100


## hameg and back bias
PSU_DEV=${PSU_DEV-'/dev/ttyHAMEG0'} # PSU file descriptor

# maximum supply current to the boards
PSU_CURR1=1.5    # digital
PSU_CURR2=0.5    # analogue
PSU_CURR3=0.020  # Vbb

PSU_VOLT1=1.8    # digital (goes directly to the chip!)
#PSU_VOLT2       # analogue = digital (goes directly to the chip!)
PSU_VOLT3=0.0    # Vbb


### working environment

HOME_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# dir with pALPIDEfs software
SOFTWARE_DIR=$(readlink -f $HOME_DIR/../)
ANA_DIR=$(readlink -f $SOFTWARE_DIR/../analysis/)
COMMON_DIR=$HOME_DIR/common/

DATE_TIME=`date +%Y%m%d_%H%M%S`
OUTFILE=$SOFTWARE_DIR/Data/powerup_test_${DATE_TIME}.log


start_clk () {
}

stop_clk () {


}

module_power_on () {
    ${COMMON_DIR}/hameg_module_test.py ${PSU_DEV} 1

}

moudle_power_off() {


if [ -z "$1" ]
then
  echo "Usage: run_irrad_chip_test.sh <CHIPID>"
  exit 0
fi
CHIP_ID=$1



# basic command
#CMD=$SOFTWARE_DIR/runTest


# initialise all power supplies
$COMMON_DIR/hameg_module_test.py $PSU_DEV 0 $PSU_CURR1 $PSU_CURR2 $PSU_CURR3 2>&1
sleep 1

# measure the current
$COMMON_DIR/hameg_module_test.py $PSU_DEV 2 2>&1
if [ ${PIPESTATUS[0]} -eq 1 ]
then
    echo "back-bias current too high, stopping measurement"  2>&1
    $COMMON_DIR/hameg_module_test.py $PSU_DEV 3 2>&1
    exit 1
fi

cd $HOME_DIR


#################################################################

#### with clock on
${HOME_DIR}/startclk

for i in $(seq 1 ${N_TRIALS})
do
    ${COMMON_DIR}/hameg_module_test.py ${PSU_DEV} 1 $PSU_VOLT1 $PSU_VOLT3 # power on the module

    CURRENTS=$(eval ${COMMON_DIR}/hameg_module_test.py ${PSU_DEV} 2 )
    CHIPS=$(eval ${HOME_DIR}/test_chip_count | cut -f1)

    echo -e "$i\t$CHIPS\t$CURRENTS" | tail -a $OUTFILE

    $COMMON_DIR/hameg_module_test.py $PSU_DEV 3 2>&1 # power off the module
done
${HOME_DIR}/stopclk

#### with clock disabled

$HOME_DIR/common/hameg2030.py $PSU_DEV 3

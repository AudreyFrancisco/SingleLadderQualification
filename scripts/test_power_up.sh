#!/bin/bash

######################################################################################
#
# script for testing the power-up behaviour using the MOSAIC and a HAMEG power supply
#
######################################################################################

### Settings

N_TRIALS=10
INTER_TEST_SLEEP=10 # seconds given to discharge the capacitors between power-off and power-on
CHIPS_PER_MODULE=9 # how many chips are expected to respond on this module

## hameg and back bias
PSU_DEV=${PSU_DEV-'/dev/ttyHAMEG0'} # PSU file descriptor

# maximum supply current to the boards
PSU_CURR1=1.3    # digital
PSU_CURR2=0.3    # analogue
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
OUTFILE_CLK=$SOFTWARE_DIR/Data/powerup_test_${DATE_TIME}_cont_clk.log
OUTFILE_NOCLK=$SOFTWARE_DIR/Data/powerup_test_${DATE_TIME}_clk_off.log


# initialise all power supplies
$COMMON_DIR/hameg_module_test.py $PSU_DEV 0 $PSU_CURR1 $PSU_CURR2 $PSU_CURR3 2>&1
sleep 1

cd $SOFTWARE_DIR


#################################################################

#### with clock on
${SOFTWARE_DIR}/startclk > /dev/null 2>&1

echo -e "#Iteration\tActiveChips\tDIDD (A)\tAIDD (A)\tIBB (A)\tActiveChipsRetry\tDIDD configured (A)\tAIDD configured (A)\tIBB configured (A)" | tee -a $OUTFILE_CLK
for i in $(seq 1 ${N_TRIALS})
do
    ${COMMON_DIR}/hameg_module_test.py ${PSU_DEV} 1 $PSU_VOLT1 $PSU_VOLT3 # power on the module
    sleep 1

    CURRENTS=$(eval ${COMMON_DIR}/hameg_module_test.py ${PSU_DEV} 2 )
    CHIPS=$(eval ${SOFTWARE_DIR}/test_chip_count | grep enabled | cut -f1 -d' ' )

    if [[ "$CHIPS" -ne "$CHIPS_PER_MODULE" ]]
    then
	sleep 2
	CHIPS_RETRY=$(eval ${SOFTWARE_DIR}/test_chip_count | grep enabled | cut -f1 -d' ' )
    else
	CHIPS_RETRY=-1
    fi
    sleep 1
    CURRENTS_CONFIGURED=$(eval ${COMMON_DIR}/hameg_module_test.py ${PSU_DEV} 2 )

    printf "%d\t%d\t%0.3f\t%0.3f\t%0.3f\t%d\t%0.3f\t%0.3f\t%0.3f\n" $i $CHIPS $CURRENTS $CHIPS_RETRY $CURRENTS_CONFIGURED | tee -a $OUTFILE_CLK

    $COMMON_DIR/hameg_module_test.py $PSU_DEV 3 2>&1 # power off the module
    ${SOFTWARE_DIR}/stopclk > /dev/null 2>&1
    sleep $INTER_TEST_SLEEP # wait between power off and power on
    ${SOFTWARE_DIR}/startclk > /dev/null 2>&1
done
${SOFTWARE_DIR}/stopclk > /dev/null 2>&1



#### with clock disabled


echo -e "#Iteration\tActiveChips\tDIDD w/o clk (A)\tAIDD w/o clk (A)\tIBB w/o clk (A)\tDIDD (A)\tAIDD (A)\tIBB (A)\tDIDD configured (A)\tAIDD configured (A)\tIBB configured (A)" | tee -a $OUTFILE_NOCLK

${SOFTWARE_DIR}/stopclk > /dev/null 2>&1

for i in $(seq 1 ${N_TRIALS})
do
    ${COMMON_DIR}/hameg_module_test.py ${PSU_DEV} 1 $PSU_VOLT1 $PSU_VOLT3 # power on the module
    sleep 1

    CURRENTS_NOCLK=$(eval ${COMMON_DIR}/hameg_module_test.py ${PSU_DEV} 2 )

    ${SOFTWARE_DIR}/startclk > /dev/null 2>&1
    sleep 1

    CURRENTS_CLK=$(eval ${COMMON_DIR}/hameg_module_test.py ${PSU_DEV} 2 )
    CHIPS=$(eval ${SOFTWARE_DIR}/test_chip_count | grep enabled | cut -f1 -d' ' )
    sleep 1
    CURRENTS_CONFIGURED=$(eval ${COMMON_DIR}/hameg_module_test.py ${PSU_DEV} 2 ))

    printf "%d\t%d\t%0.3f\t%0.3f\t%0.3f\t%0.3f\t%0.3f\t%0.3f\t%0.3f\t%0.3f\t%0.3f\n" $i $CHIPS $CURRENTS_NOCLK $CURRENTS_CLK $CURRENTS_CONFIGURED | tee -a $OUTFILE_NOCLK

    ${SOFTWARE_DIR}/stopclk > /dev/null 2>&1

    $COMMON_DIR/hameg_module_test.py $PSU_DEV 3 2>&1 # power off the module

    sleep $INTER_TEST_SLEEP # wait between power off and power on
done

$HOME_DIR/common/hameg_module_test.py $PSU_DEV 3

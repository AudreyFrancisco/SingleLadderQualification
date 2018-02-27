#!/bin/bash

N_TRIALS=10000


HOME_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PON=${HOME_DIR}/MosaicSrc/powerboard/powerOn_Stave_digital_first.sh
POFF=${HOME_DIR}/MosaicSrc/powerboard/powerOff_Stave.sh

DATE_TIME=`date +%Y%m%d_%H%M%S`

LOG=${HOME_DIR}/powerOnTest_Stave_ClkDigital1.7_250ms_AnalogueNominal_2ndStepDigital_50ms_CurrentLimit_${DATE_TIME}.log

rm -f ${LOG}


${HOME_DIR}/startclk          | tee -a ${LOG}

for i in $(seq 1 ${N_TRIALS})
do
    echo "#### TRIAL $i"          | tee -a ${LOG}
    ${PON}                        | tee -a ${LOG}
    ${HOME_DIR}/test_chip_count   | tee -a ${LOG}
done

${HOME_DIR}/stopclk           | tee -a ${LOG}

#!/bin/bash

N_TRIALS=1000


HOME_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DIR=$(readlink -f ${HOME_DIR}/../../)
PON=${DIR}/MosaicSrc/powerboard/powerOn_Stave.sh
POFF=${DIR}/MosaicSrc/powerboard/powerOff_Stave.sh

DATE_TIME=`date +%Y%m%d_%H%M%S`

LOG=${DIR}/Data/powerOnTest_Stave_AnalogueDigitalClk_${DATE_TIME}.log

rm -f ${LOG}

for i in $(seq 1 ${N_TRIALS})
do
    echo "#### TRIAL $i"     | tee -a ${LOG}
    ${DIR}/stopclk           | tee -a ${LOG}
    ${PON}                   | tee -a ${LOG}
    ${DIR}/startclk          | tee -a ${LOG}
    sleep 1                  | tee -a ${LOG}
    ${DIR}/test_chip_count   | tee -a ${LOG}
    ${POFF}                  | tee -a ${LOG}
done

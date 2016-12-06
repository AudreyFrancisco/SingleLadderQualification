#!/bin/bash


ROOT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd ) # determine where this script is located

LOG_FILE=${ROOT_DIR}/Data/por_clk_first_log_$(date +%Y-%m-%d_%H-%M-%S).txt
RESULT_FILE=${ROOT_DIR}/Data/por_clk_first_result_$(date +%Y-%m-%d_%H-%M-%S).txt

GOOD=0
BAD=0

./poweron_setup.py 2>&1 | tee -a ${LOG_FILE}

for iTrial in $(seq 1 10000)
do
    # power on
    ./poweron_setup.py 2>&1 | tee -a ${LOG_FILE}

    sleep 1

    # program
    cd  $(readlink -f ${ROOT_DIR}/../alpide-software-fx3-tools )
    ./program.sh  2>&1 | tee -a ${LOG_FILE}
    cd ${ROOT_DIR}

    sleep 0.1


    # do the test
    timeout 20 ./test_por_clk_first_reset 2>&1 | tee -a ${LOG_FILE}
    ret_value=$?

    if [[ "$ret_value" -eq 0 ]]
    then
          let GOOD=GOOD+1
          echo ${iTrial} 1 ${GOOD} ${BAD} | tee -a ${RESULT_FILE}
    else
        if [[ "$ret_value" -ge 124 ]]
        then
            let BAD=BAD+1
            echo ${iTrial} 1 ${GOOD} ${BAD} "TIMED OUT!" | tee -a ${RESULT_FILE}


        else
            let BAD=BAD+1
            echo ${iTrial} 1 ${GOOD} ${BAD} | tee -a ${RESULT_FILE}

        fi

        # power off
        ./poweroff_all.py 2>&1 | tee -a ${LOG_FILE}
    fi

    # power off
    ./poweroff.py 2>&1 | tee -a ${LOG_FILE}

    sleep 1
done

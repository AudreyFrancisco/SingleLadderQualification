#!/bin/bash

# voltages to be used with the 9m cables
VOLTAGES=(1.930 2.090 1.970 2.130 1.980 2.140 1.980 2.150 1.990 2.160 2.010 2.110 2.050 2.210 1.895 1.895 )
IP=192.168.168.250

HOME_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PBCTRL=${HOME_DIR}/pbctrl

# get to a defined state
${PBCTRL} -off ${IP} ;
${PBCTRL} -state ${IP}
sleep 1

for ch in $(seq 1 2 13)
do
    # digital - first step
	${PBCTRL} -Vout $ch 1.497 ${IP}
    ${PBCTRL} -Ith $ch 3.0 ${IP}
    ${PBCTRL} -onch $ch ${IP}
    ${PBCTRL} -Ith $ch 1.5 ${IP}

    # analogue
    ch_analogue=$(expr $ch - 1)
	${PBCTRL} -Vout ${ch_analogue} ${VOLTAGES[ch_analogue]} ${IP}

    ${PBCTRL} -Ith  ${ch_analogue} 3.0 ${IP}
    ${PBCTRL} -onch ${ch_analogue} ${IP}
    ${PBCTRL} -Ith  ${ch_analogue} 0.5 ${IP}

    # digital - second step
	${PBCTRL} -Vout $ch ${VOLTAGES[ch]} ${IP}
done

sleep 1 # wait

${PBCTRL} -state ${IP}
echo "### Stave ON ###"

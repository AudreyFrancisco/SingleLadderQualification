#!/bin/bash

INPUT_FOLDER=$(readlink -f $1)

#################################################################
### ENVIRONMENT
#################################################################
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR=$(readlink -f ${SCRIPT_DIR}/../../ )

command -v root >/dev/null 2>&1 || { echo "CERN ROOT not found.  Aborting." >&2; exit 1; }

cd $(readlink -f ${SCRIPT_DIR}/../)
find ${INPUT_FOLDER} -name "ThresholdScan*Chip0_0.dat" -exec root -l -b -q All_FitThresholdsIB.C+g'("'{}'")' \;


for i in $(seq 0 8)
do
    rm -f ${INPUT_FOLDER}/vgo_summary_chip${i}.txt;
    rm -f ${INPUT_FOLDER}/vbb_summary_chip${i}.txt;
    find ${INPUT_FOLDER}/VGO* -name "ThresholdSummary*Chip0_0.dat" -exec bash -c "echo -e {} | cut -d '/' -f8 | cut -d'O' -f2 | tr -d '\n'; head -n1 {}"  \; >> ${INPUT_FOLDER}/vgo_summary_chip${i}.txt
    find ${INPUT_FOLDER}/VBB* -name "ThresholdSummary*Chip0_0.dat" -exec bash -c "echo -e {} | cut -d '/' -f8 | cut -d'B' -f3 | tr -d '\n'; head -n1 {}"  \; >> ${INPUT_FOLDER}/vbb_summary_chip${i}.txt
done


cd ${INPUT_FOLDER}
gnuplot <<EOF

set terminal postscript enhanced color font 'Helvetica,10'
set output "summary.ps"


set xlabel 'Voltage (V)'
set ylabel 'Threshold (e^{-})'
set xrange [-0.35:0.35]

plot 'vgo_summary_chip0.txt' using 1:4 title 'V_{Offset} - Chip 0', \
     'vgo_summary_chip1.txt' using 1:4 title 'V_{Offset} - Chip 1', \
     'vgo_summary_chip2.txt' using 1:4 title 'V_{Offset} - Chip 2', \
     'vgo_summary_chip3.txt' using 1:4 title 'V_{Offset} - Chip 3', \
     'vgo_summary_chip4.txt' using 1:4 title 'V_{Offset} - Chip 4', \
     'vgo_summary_chip5.txt' using 1:4 title 'V_{Offset} - Chip 5', \
     'vgo_summary_chip6.txt' using 1:4 title 'V_{Offset} - Chip 6', \
     'vgo_summary_chip7.txt' using 1:4 title 'V_{Offset} - Chip 7', \
     'vbb_summary_chip0.txt' using 1:4 title 'V_{BB} - Chip 0', \
     'vbb_summary_chip1.txt' using 1:4 title 'V_{BB} - Chip 1', \
     'vbb_summary_chip2.txt' using 1:4 title 'V_{BB} - Chip 2', \
     'vbb_summary_chip3.txt' using 1:4 title 'V_{BB} - Chip 3', \
     'vbb_summary_chip4.txt' using 1:4 title 'V_{BB} - Chip 4', \
     'vbb_summary_chip5.txt' using 1:4 title 'V_{BB} - Chip 5', \
     'vbb_summary_chip6.txt' using 1:4 title 'V_{BB} - Chip 6', \
     'vbb_summary_chip7.txt' using 1:4 title 'V_{BB} - Chip 7'
EOF

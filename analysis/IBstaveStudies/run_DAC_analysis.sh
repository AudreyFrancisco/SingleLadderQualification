#!/bin/bash

INPUT_FOLDER=$(readlink -f $1)
SUFFIX=$2

#################################################################
### ENVIRONMENT
#################################################################
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR=$(readlink -f ${SCRIPT_DIR}/../../ )

command -v root >/dev/null 2>&1 || { echo "CERN ROOT not found.  Aborting." >&2; exit 1; }


#################################################################
### EXECUTION
#################################################################
cd ${INPUT_FOLDER}

OUTFILE=DAC_summary_${SUFFIX}.dat
echo -e "# DAC\tChip\tV\tDAC value\tADC value" > ${OUTFILE}

## generate a summary file (ASCII table)
for f in $(ls *DAC_*_${SUFFIX}.dat | grep -v summary )
do
    DAC=$(echo $f   | cut -d'_' -f2 )
    CHIP=$(echo $f  | cut -d'_' -f3 )
    CHIP=${CHIP:4}
    VOLT=$(echo $f  | cut -d'_' -f5 )
    VOLT=${VOLT::-1}

    while IFS= read LINE
    do
	echo "${DAC} ${CHIP} ${VOLT} ${LINE}" >> ${OUTFILE}
    done < "$f"
done

DACS=$(cat ${OUTFILE}       | grep -v "\#" | cut -f1 -d' ' | uniq       | tr '\n' ' ' )
CHIPS=$(cat ${OUTFILE}      | grep -v "\#" | cut -f2 -d' ' | sort -u -n | tr '\n' ' ' )
VOLTAGES=$(cat ${OUTFILE}   | grep -v "\#" | cut -f3 -d' ' | sort -u -n | tr '\n' ' ' )
DAC_VALUES=$(cat ${OUTFILE} | grep -v "\#" | cut -f4 -d' ' | sort -u -n | tr '\n' ' ' )

echo
echo "DACS: "${DACS}
echo
echo "CHIPS: "${CHIPS}
echo
echo "SUPPLY VOLTAGES: "${VOLTAGES}
echo
echo "SET VALUES: "${DAC_VALUES}
echo

## convert ASCII table to a ROOT TTree
root -l -b <<EOF
TFile* f = new TFile("DAC_summary_${SUFFIX}.root", "RECREATE");
TTree*  t = new TTree("DACsummary","");
Long64_t nlines = t->ReadFile("${OUTFILE}", "DAC/C:Chip/s:Voltage/D:SetValue/b:MeasValue/D");
std::cout << "Read " << nlines << " lines into the TTree" << std::endl;
t->Write();
f->Close();
delete f;
f = 0x0;
EOF

## archive the dat files
tar cjf DAC_summary_${SUFFIX}_archive.tar.bz2 *DAC_*_${SUFFIX}.dat --remove-files

## analyse the data
for d in VCASN VCASP VCASN2 VCLIP VTEMP
do
    echo "DAC: $d"
    echo "###############"
    root -l -b -q ${SCRIPT_DIR}/analyse_DAC.C'("'$(readlink -f DAC_summary_${SUFFIX}.root)'","'${d}'")'
done


## AVDD measurement file
cat AVDD_*_${SUFFIX}.dat > AVDD_summary_${SUFFIX}.dat
## convert ASCII table to a ROOT TTree
root -l -b <<EOF
TFile* f = new TFile("AVDD_summary_${SUFFIX}.root", "RECREATE");
TTree*  t = new TTree("AVDDsummary","");
Long64_t nlines = t->ReadFile("AVDD_summary_${SUFFIX}.dat", "AVDDsetV/D:Chip/s:Sample/b:AVDDmeasV/D:AVDDmeasADC/b:VTEMPmeasV/D:AVDDdacMeasV/D:AVDDdacMeasErrV/D");
std::cout << "Read " << nlines << " lines into the TTree" << std::endl;
t->Write();
f->Close();
delete f;
f = 0x0;
EOF

## archive the dat files
tar cjf AVDD_summary_${SUFFIX}_archive.tar.bz2 AVDD_*_${SUFFIX}.dat --remove-files
root -l -b -q ${SCRIPT_DIR}/analyse_AVDD.C'("'$(readlink -f AVDD_summary_${SUFFIX}.root)'")'

echo "# done."

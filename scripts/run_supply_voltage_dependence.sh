#!/bin/bash

######################################################################################
#
# script for running threshold measurements with different settings
# written by jacobus - j.w.van.hoorne@cern.ch
#
######################################################################################

if [ -z "$1" ]
then
  echo "Usage: run_supplyVoltage_scan.sh <CHIPID>"
  exit 0
fi
CHIP_ID=$1

#IRRAD_LEVEL=`common/getIrradLevel_pALPIDE3.py $CHIP_ID | tail -n1`
#cmp=`echo ${IRRAD_LEVEL} | sed -e 's/[eE]+*/\\*10\\^/'`
#if (( $(echo "if (${cmp} > -1) 1 else -1" | bc) == -1 ))
#then
#    echo "chip name or irradiation level of chip not found, please check!"
#    exit 1
#fi


#################################################################
### measurement config
#################################################################

##########################################
# DCOL (0..15) and ROW (0..512) of pulsed pixels
#COL=8
#ROW=5

#TEMPERATURE
TEMP=room

# Supply voltages
V_LIST=( $(seq 1.780 0.005 1.820) )
N_V=${#V_LIST[@]}

# Subset of testbeam settings
########################
# VBB
#N_VBB=3
#VBB_LIST=( 0.0 3.0 6.0 )
N_VBB=2
VBB_LIST=( 0.0 3.0 )

# VCASN
#N_VCASN=1
#N_VCASN=21
N_VCASN=5
declare -A VCASN_LIST
## 0V
#VCASN_LIST[0,0]=55

#VCASN_LIST[0,0]=40
#VCASN_LIST[0,1]=41
#VCASN_LIST[0,2]=42
#VCASN_LIST[0,3]=43
#VCASN_LIST[0,4]=44
#VCASN_LIST[0,5]=45
#VCASN_LIST[0,6]=46
#VCASN_LIST[0,7]=47
#VCASN_LIST[0,8]=48
#VCASN_LIST[0,9]=49
#VCASN_LIST[0,10]=50
#VCASN_LIST[0,11]=51
#VCASN_LIST[0,12]=52
#VCASN_LIST[0,13]=53
#VCASN_LIST[0,14]=54
#VCASN_LIST[0,15]=55
#VCASN_LIST[0,16]=56
#VCASN_LIST[0,17]=57
#VCASN_LIST[0,18]=58
#VCASN_LIST[0,19]=59
#VCASN_LIST[0,20]=60

VCASN_LIST[0,0]=40
VCASN_LIST[0,1]=45
VCASN_LIST[0,2]=50
VCASN_LIST[0,3]=55
VCASN_LIST[0,4]=60

## -6V
#VCASN_LIST[1,0]=100

#VCASN_LIST[1,0]=95
#VCASN_LIST[1,1]=96
#VCASN_LIST[1,2]=97
#VCASN_LIST[1,3]=98
#VCASN_LIST[1,4]=99
#VCASN_LIST[1,5]=100
#VCASN_LIST[1,6]=101
#VCASN_LIST[1,7]=102
#VCASN_LIST[1,8]=103
#VCASN_LIST[1,9]=104
#VCASN_LIST[1,10]=105
#VCASN_LIST[1,11]=106
#VCASN_LIST[1,12]=107
#VCASN_LIST[1,13]=108
#VCASN_LIST[1,14]=109
#VCASN_LIST[1,15]=110
#VCASN_LIST[1,16]=111
#VCASN_LIST[1,17]=112
#VCASN_LIST[1,18]=113
#VCASN_LIST[1,19]=114
#VCASN_LIST[1,20]=115

VCASN_LIST[1,0]=95
VCASN_LIST[1,1]=100
VCASN_LIST[1,2]=105
VCASN_LIST[1,3]=110
VCASN_LIST[1,4]=115


# ITHR
ITHR_LIST=( 30 50 70 100 150 )
#ITHR_LIST=( 50 )

# VCASN2
VCASN2=0 # should be VCASN+12, assigned later

# IRESET
IRESET_LIST=( 100 )

# IDB
IDB_LIST=( 29 )

# VCLIP
N_VCLIP=1
declare -A VCLIP_LIST
# 0V
VCLIP_LIST[0,0]=0
# -3V
#VCLIP_LIST[0,0]=60
VCLIP_LIST[1,0]=60
# -6V
#VCLIP_LIST[1,0]=100
#VCLIP_LIST[2,0]=100

#N_VCLIP=5
#declare -A VCLIP_LIST
## 0V
#VCLIP_LIST[0,0]=0
#VCLIP_LIST[0,1]=20
#VCLIP_LIST[0,2]=40
#VCLIP_LIST[0,3]=60
#VCLIP_LIST[0,4]=80
## -3V
#VCLIP_LIST[1,0]=50
#VCLIP_LIST[1,1]=60
#VCLIP_LIST[1,2]=80
#VCLIP_LIST[1,3]=100
#VCLIP_LIST[1,4]=120
## -6V
#VCLIP_LIST[2,0]=90
#VCLIP_LIST[2,1]=100
#VCLIP_LIST[2,2]=120
#VCLIP_LIST[2,3]=140
#VCLIP_LIST[2,4]=160

# VRESETP
N_VRESETP=1
declare -A VRESETP_LIST
VRESETP_LIST[0,0]=117
VRESETP_LIST[1,0]=147
#VRESETP_LIST[2,0]=117

# VRESETD
N_VRESETD=1
declare -A VRESETD_LIST
#VRESETD_LIST[0,0]=147
#VRESETD_LIST[1,0]=147
#VRESETD_LIST[2,0]=170
VRESETD_LIST[0,0]=117
VRESETD_LIST[1,0]=147


##########################################

# measurement home dir
HOME_DIR=`pwd`
# dir with pALPIDEfs software
SOFTWARE_DIR=$HOME_DIR/../
ANA_DIR=$SOFTWARE_DIR/analysis/
COMMON_DIR=$HOME_DIR/common/

# basic command
CMD=$SOFTWARE_DIR/runTest

##### hameg and back bias

# PSU file descriptor
PSU_DEV=${PSU_DEV-'/dev/ttyHAMEG0'}
# maximum supply current to the boards
PSU_CURR1=1.0    # positive
PSU_CURR2=0.15   #
PSU_CURR3=0.020  # vbb

PSU_VOLT1=5.0
PSU_VOLT2=0.0
PSU_VOLT3=0.0

# initialise all power supplies
$HOME_DIR/common/hameg2030.py $PSU_DEV 7 2>&1
sleep 5
$HOME_DIR/common/hameg2030.py $PSU_DEV 0 $PSU_CURR1 $PSU_CURR2 $PSU_CURR3 2>&1
$HOME_DIR/common/hameg2030.py $PSU_DEV 10 2 ${V_LIST[0]} 2>&1
$HOME_DIR/common/hameg2030.py $PSU_DEV 5 2>&1
sleep 5

# measure the current
$HOME_DIR/common/hameg2030.py $PSU_DEV 4 2>&1
if [ ${PIPESTATUS[0]} -eq 1 ]
then
    echo "back-bias current too high, stopping measurement"  2>&1
    $HOME_DIR/common/hameg2030.py $PSU_DEV 7 2>&1
    exit 1
fi

$HOME_DIR/common/hameg2030.py $PSU_DEV 5 2>&1


##### program fx3
#FX3_DIR=$SOFTWARE_DIR/../fx3
FX3_DIR=/home/freidt/alpide-software-fx3-tools/
cd $FX3_DIR
is_programmed=1
i_try=0
max_tries=5
while [ ! $is_programmed -eq 0 ]
do
    sleep 1
    echo "programming fx3..."
    ./download_fx3 -t RAM -i SlaveFifoSync.img
    is_programmed=$?

    let i_try=i_try+1
    if [ $i_try -ge $max_tries ]
    then
        echo "max number of tries reached, please reboot daqboard"
        exit 3
    fi
done
wait
sleep 2


###### set temperature of chiller
#cd $COMMON_DIR
#./huber.py 1 $TEMP
## check if it is reached
#temp_is_set=0
#while [ $temp_is_set -eq 0 ]
#do
#    sleep 10
#    ./huber.py 3
#    temp_is_set=$?
#done
## wait some time to let the chip accomodate
#sleep 60
#./huber.py 3

cd $HOME_DIR


#################################################################
### create folder structure and parameter lists
#################################################################
DATE_TIME=`date +%Y%m%d_%H%M%S`

# create folder structure and write list files
OUT_FOLDER=${CHIP_ID}_supplyVoltage_${DATE_TIME}
#DATA_DIR=$ANA_DIR/data/$OUT_FOLDER
DATA_DIR=$SOFTWARE_DIR/Data/$OUT_FOLDER
if [ ! -d "$DATA_DIR" ]
then
    mkdir $DATA_DIR
fi

LOG=$DATA_DIR/run.log
if [ -f "$LOG" ]
then
    rm $LOG
fi

# folder to save list/config files
SETTINGS_DIR=$DATA_DIR/settings
if [ ! -d "$SETTINGS_DIR" ]
then
    mkdir $SETTINGS_DIR
fi

CHIP_FILE=$SETTINGS_DIR/chip_name.txt
if [ -f "$CHIP_FILE" ]
then
    rm $CHIP_FILE
fi
echo "$CHIP_ID" >> $CHIP_FILE
IRRAD_FILE=$SETTINGS_DIR/irrad_level.txt
if [ -f "$IRRAD_FILE" ]
then
    rm $IRRAD_FILE
fi
echo "$IRRAD_LEVEL" >> $IRRAD_FILE

TEMP_FILE=$SETTINGS_DIR/temperature.txt
if [ -f "$TEMP_FILE" ]
then
    rm $TEMP_FILE
fi
echo "$TEMP" >> $TEMP_FILE

ITHR_FILE=$SETTINGS_DIR/ithr_list.txt
if [ -f "$ITHR_FILE" ]
then
    rm $ITHR_FILE
fi
for ITHR in "${ITHR_LIST[@]}"
do
    echo $ITHR >> $ITHR_FILE
done
IDB_FILE=$SETTINGS_DIR/idb_list.txt
if [ -f "$IDB_FILE" ]
then
    rm $IDB_FILE
fi
for IDB in "${IDB_LIST[@]}"
do
    echo $IDB >> $IDB_FILE
done
IRESET_FILE=$SETTINGS_DIR/ireset_list.txt
if [ -f "$IRESET_FILE" ]
then
    rm $IRESET_FILE
fi
for IRESET in "${IRESET_LIST[@]}"
do
    echo $IRESET >> $IRESET_FILE
done
VBB_FILE=$SETTINGS_DIR/vbb_list.txt
if [ -f "$VBB_FILE" ]
then
    rm $VBB_FILE
fi
V_FILE=$SETTINGS_DIR/v_list.txt
if [ -f "$V_FILE" ]
then
    rm $V_FILE
fi



for V in "${V_LIST[@]}"
do
    for ((i_vbb=0; i_vbb<$N_VBB; i_vbb++))
    do
        VBB=${VBB_LIST[${i_vbb}]}
        echo $VBB >> $VBB_FILE

        # create folder for each vbb value
        SETTINGS_SUBFOLDER="VBB-$VBB/V${V}"
        SETTINGS_SUBFOLDER=$DATA_DIR/${SETTINGS_SUBFOLDER}
        if [ ! -d "${SETTINGS_SUBFOLDER}" ]
        then
            mkdir -p ${SETTINGS_SUBFOLDER}
        fi
        VBB_SETTINGS_DIR=${SETTINGS_SUBFOLDER}/settings
        if [ ! -d "$VBB_SETTINGS_DIR" ]
        then
            mkdir $VBB_SETTINGS_DIR
        fi
        VBB_THRESHOLD_DIR=${SETTINGS_SUBFOLDER}/threshold
        if [ ! -d "$VBB_THRESHOLD_DIR" ]
        then
            mkdir $VBB_THRESHOLD_DIR
        fi
        VBB_FHR_DIR=${SETTINGS_SUBFOLDER}/fhr
        if [ ! -d "$VBB_FHR_DIR" ]
        then
            mkdir $VBB_FHR_DIR
        fi

        # write settings measured for each vbb value to settings files
        VCASN_FILE=$VBB_SETTINGS_DIR/vcasn_list.txt
        VCASN2_FILE=$VBB_SETTINGS_DIR/vcasn2_list.txt
        if [ -f "$VCASN_FILE" ]
        then
            rm $VCASN_FILE
        fi
        if [ -f "$VCASN2_FILE" ]
        then
            rm $VCASN2_FILE
        fi
        #for VCASN in "${VCASN_LIST[$i_vcasn]}"
        for ((i_vcasn=0; i_vcasn<$N_VCASN; i_vcasn++))
        do
            VCASN=${VCASN_LIST[${i_vbb},${i_vcasn}]}
            echo $VCASN >> $VCASN_FILE
            VCASN2=$(($VCASN+12))
            echo $VCASN2 >> $VCASN2_FILE
        done

        VCLIP_FILE=$VBB_SETTINGS_DIR/vclip_list.txt
        if [ -f "$VCLIP_FILE" ]
        then
            rm $VCLIP_FILE
        fi
        for ((i_vclip=0; i_vclip<$N_VCLIP; i_vclip++))
        do
            VCLIP=${VCLIP_LIST[${i_vbb},${i_vclip}]}
            echo $VCLIP >> $VCLIP_FILE
        done

        VRESETP_FILE=$VBB_SETTINGS_DIR/vresetp_list.txt
        if [ -f "$VRESETP_FILE" ]
        then
            rm $VRESETP_FILE
        fi
        for ((i_v=0; i_v<$N_VRESETP; i_v++))
        do
            VRESETP=${VRESETP_LIST[${i_vbb},${i_v}]}
            echo $VRESETP >> $VRESETP_FILE
        done

        VRESETD_FILE=$VBB_SETTINGS_DIR/vresetd_list.txt
        if [ -f "$VRESETD_FILE" ]
        then
            rm $VRESETD_FILE
        fi
        for ((i_v=0; i_v<$N_VRESETD; i_v++))
        do
            VRESETD=${VRESETD_LIST[${i_vbb},${i_v}]}
            echo $VRESETD >> $VRESETD_FILE
        done

    done
done

#################################################################
### loop over parameters
#################################################################
cd $SOFTWARE_DIR

VBB=0.0
VBB_OLD=0.0
IRESET=${IRESET_LIST[0]}
IDB=${IDB_LIST[0]}

for V in "${V_LIST[@]}"
do
    $HOME_DIR/common/hameg2030.py $PSU_DEV 10 2 ${V} 2>&1
    sleep 5
    $HOME_DIR/common/hameg2030.py $PSU_DEV 5 2>&1

    for ((i_vbb=0; i_vbb<$N_VBB; i_vbb++))
    do
        VBB_OLD=$VBB
        VBB=${VBB_LIST[${i_vbb}]}
        SETTINGS_SUBFOLDER=$DATA_DIR/$(printf "VBB-%0.1f/V%0.3f" ${VBB} ${V})
        if [ ! -d "${SETTINGS_SUBFOLDER}" ]
        then
            echo "VBB dir not found"
            exit 3
        fi

        echo "-------------------------------------------"
        echo "VBB $VBB" | tee -a $LOG

        cd $SOFTWARE_DIR
        #cd $HOME_DIR
        $HOME_DIR/common/hameg2030.py $PSU_DEV 3 $VBB_OLD $VBB 5 2>&1 | tee -a $LOG
        # measure the current
        $HOME_DIR/common/hameg2030.py $PSU_DEV 4 2>&1 | tee -a $LOG
        if [ ${PIPESTATUS[0]} -eq 1 ]
        then
            echo "back-bias current too high, stopping measurement"
            ./hameg2030.py $PSU_DEV 7 2>&1 | tee -a $LOG
            exit 1
        fi

        VRESETP=${VRESETP_LIST[${i_vbb},0]}
        VRESETD=${VRESETD_LIST[${i_vbb},0]}

        #for VCASN in "${VCASN_LIST[@]}"
        for ((i_vcasn=0; i_vcasn<$N_VCASN; i_vcasn++))
        do
            VCASN=${VCASN_LIST[${i_vbb},${i_vcasn}]}
            VCASN2=$(($VCASN+12))
            echo "-- VCASN $VCASN" | tee -a $LOG

            for ((i_vclip=0; i_vclip<$N_VCLIP; i_vclip++))
            do
                VCLIP=${VCLIP_LIST[${i_vbb},${i_vclip}]}
                echo "---- VCLIP $VCLIP" | tee -a $LOG

                for ITHR in "${ITHR_LIST[@]}"
                do
                    echo "------ ITHR $ITHR" | tee -a $LOG
                    # change config
                    cp Config_template.cfg Config.cfg
                    sed -i -e s/#VCASN/VCASN/g Config.cfg
                    sed -i -e 's/vcasnTmp/'${VCASN}'/g' Config.cfg
                    sed -i -e s/#ITHR/ITHR/g Config.cfg
                    sed -i -e 's/ithrTmp/'${ITHR}'/g' Config.cfg
                    sed -i -e s/#VCASN2/VCASN2/g Config.cfg
                    sed -i -e 's/vcasn2Tmp/'${VCASN2}'/g' Config.cfg
                    sed -i -e s/#VCLIP/VCLIP/g Config.cfg
                    sed -i -e 's/vclipTmp/'${VCLIP}'/g' Config.cfg
                    sed -i -e s/#VRESETP/VRESETP/g Config.cfg
                    sed -i -e 's/vresetpTmp/'${VRESETP}'/g' Config.cfg
                    sed -i -e s/#VRESETD/VRESETD/g Config.cfg
                    sed -i -e 's/vresetdTmp/'${VRESETD}'/g' Config.cfg
                    sed -i -e s/#IDB/IDB/g Config.cfg
                    sed -i -e 's/idbTmp/'${IDB}'/g' Config.cfg
                    sed -i -e s/#IRESET/IRESET/g Config.cfg
                    sed -i -e 's/iresetTmp/'${IRESET}'/g' Config.cfg

                    # threshold measurement
                    cd $SOFTWARE_DIR
                    #sleep 4
                    #./runTest THRESHOLD 164 0 50 2>&1 | tee -a $LOG
                    #./runTest THRESHOLD 82 0 70 2>&1 | tee -a $LOG
                    time timeout 1200 ./test_threshold 2>&1 | tee -a $LOG
                    wait
                    cd $SOFTWARE_DIR/Data
                    RUN_DATA_FILE=`ls -tr1 Threshold*.dat | tail -1`
                    RUN_CFG_FILE=`ls -tr1 ScanConfig*.cfg | tail -1`
                    echo "" >> $SOFTWARE_DIR/Data/$RUN_CFG_FILE
                    echo "CHIP $CHIP_ID" >> $SOFTWARE_DIR/Data/$RUN_CFG_FILE
                    echo "VBB  $VBB"     >> $SOFTWARE_DIR/Data/$RUN_CFG_FILE
                    echo "V    $V"       >> $SOFTWARE_DIR/Data/$RUN_CFG_FILE
                    # copy data files into folder structure
                    mv $SOFTWARE_DIR/Data/$RUN_CFG_FILE ${SETTINGS_SUBFOLDER}/threshold/ -v
                    mv $SOFTWARE_DIR/Data/$RUN_DATA_FILE ${SETTINGS_SUBFOLDER}/threshold/ -v
                    wait

                    sleep 10

                    ## fhr measurement
                    #cd $SOFTWARE_DIR
                    #timeout 240 ./test_noiseocc 2>&1 | tee -a $LOG
                    #wait
                    #cd $SOFTWARE_DIR/Data
                    #RUN_DATA_FILE=`ls -tr1 NoiseOccupancy*.dat | tail -1`
                    #RUN_CFG_FILE=`ls -tr1 ScanConfig*.cfg | tail -1`
                    #echo "" >> $SOFTWARE_DIR/Data/$RUN_CFG_FILE
                    #echo "CHIP $CHIP_ID" >> $SOFTWARE_DIR/Data/$RUN_CFG_FILE
                    #echo "VBB  $VBB" >> $SOFTWARE_DIR/Data/$RUN_CFG_FILE
                    ## copy data files into folder structure
                    #mv $SOFTWARE_DIR/Data/$RUN_CFG_FILE ${SETTINGS_SUBFOLDER}/fhr/
                    #mv $SOFTWARE_DIR/Data/$RUN_DATA_FILE ${SETTINGS_SUBFOLDER}/fhr/
                    #wait
                    #
                    #sleep 10
                    cd $SOFTWARE_DIR
                done
            done
        done
    done
done

# power off DAQ board
$HOME_DIR/common/hameg2030.py $PSU_DEV 7

# put chiller on stand-by mode
#cd $COMMON_DIR
#./huber.py 4

echo
echo
date
echo "done."

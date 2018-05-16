#!/bin/bash

# --- Get all the variables ---

SCRIPTSPATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
. $SCRIPTSPATH/EOStransfer.cfg

HINIBITFILE=$DBATTACHBASEPATH+"StopTransfer"
STARTDATE=`date`

# The remote EOS Path, with the service account name specification
DBATTACHREMOTEPATH=$DBATTACHSERVICEACCOUNT"@lxplus.cern.ch:/eos/project/a/alice-its/www"

echo " ------ ALICE-ITS FPC bench sync program - v.1.2 - A.Franco - INFN BARI Italy" 
echo "Start execution : $STARTDATE"
echo "Local path = $DBATTACHBASEPATH"
echo "Remote path = $DBATTACHREMOTEPATH" 
echo "Number of attempts = $DBATTACHATTEMPTS"

# --- tests if the transfer is hinibit ---
if [ -e $HINIBITFILE ]
then
	echo "$STARTDATE : The EOS file transfer is hinibit !"
	exit 0
fi

# --- performs the rsync, loop for more attempts ---
while [ $DBATTACHATTEMPTS -ne 0 ]; do

	# - do the sync	
	STOPDATE=`date`
	rsync -avuze ssh $DBATTACHBASEPATH $DBATTACHREMOTEPATH
	# - evaluates the result
	if [[ $? -gt 0 ]] 
	then
   		echo "$STOPDATE : Error to sync the remote repository $DBATTACHREMOTEPATH" >> $LOGFILENAME
   		echo "$STOPDATE : Error to sync the remote repository $DBATTACHREMOTEPATH"
   		DBATTACHATTEMPTS--
   		sleep 10
	else
   		echo "Remote repository sync done. ($STARTDATE -> $STOPDATE)" >> $LOGFILENAME
   		echo "Remote repository sync done at $STOPDATE !"
		echo " --------- "
   		exit 0 
	fi
done

STOPDATE=`date`
echo "$STOPDATE : Exit for error. Abort !"
echo " --------- "
exit 1



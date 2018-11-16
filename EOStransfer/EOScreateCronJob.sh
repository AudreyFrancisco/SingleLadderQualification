#!/bin/bash

echo " ------ ALICE-ITS EOS transfer cron job istallation script - v.1.2 - A.Franco - INFN BARI Italy" 

# Builds the command line
SCRIPTSPATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CRONCOMMAND=$SCRIPTSPATH"/EOStransfer.sh"
echo " The Cron command is = $CRONCOMMAND"

#  Writes out the current crontab
crontab -l > /tmp/actualCron
if [[ $? -gt 0 ]] 
then
	echo "Warning, is the first installation or some thing goes wrong into the cron job installation. Go ahead... [1]"
fi

if grep -F "$CRONCOMMAND" /tmp/actualCron
then
	echo " The Cron command is already installed. Exit ! [2]"
	RESULT=0
else
	echo " Insert new cron into cron file..."
	echo "10 * * * * $CRONCOMMAND &>/tmp/lastEOSCronExecution.log" >> /tmp/actualCron

	# Now installs the new cron file
	crontab /tmp/actualCron
	if [[ $? -gt 0 ]] 
	then
		echo "Error, some thing goes wrong into the cron job installation... [3]"
		RESULT=1
	else
		# Then the last check (redundant )
		crontab -l > /tmp/actualCron
		if grep -F "$CRONCOMMAND" /tmp/actualCron
		then
			echo " The Cron command is installed. Eureka ! [4]"
	    		RESULT=0
		else
			echo "Error, I don't find the cron job installed... [5]"
			RESULT=1
		fi
	fi
fi

echo "  ------ ALICE-ITS FPC bench cron job istallation script: Terminate"
rm /tmp/actualCron
exit $RESULT

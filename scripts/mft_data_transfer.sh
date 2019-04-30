#!/bin/bash

#: Title : mft_data_transfer.sh
#: Date : 2018-09-28
#: Author : "Fabrice Guilloux" <fabrice.guilloux@cea.fr>
#: Version : 2.0
#: Description : transfert the folder ./Data/hicXXX/ladder_func/ to lxplus7:/eos/project/m/mft/PRODUCTION/WP2_Ladder/HIC_Ladder/HIC_Ladder_XXX/
#: Options : None


## STOP at failure
set -e

## SET VARIABLES
### Get program name
progName=${0##*/}
### Current Date
currentDate=$(date +%Y-%m-%d)
currentTime=$(date +%Hh%Mm%Ss)
### Local
LC_ALL=C
boldText=$(tput bold)
normalText=$(tput sgr0)
### Regex
isFloat='^[-+]?[0-9]+\.?[0-9]*$'
isInt='^[0-9]+$'
isIP='^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$'
### Defaults values
userRsync="fguillou"
hicNumber=""
forceCreate="false"
testMode="false"

### printf template
commentsTemplate="%s "
usageOptionsTemplate="\t%-4s, %-15s %-15s %s\n"

## FUNCTIONS
## Usage
usage(){
    printf "\nUsage: %s [OPTIONS] hic_number                              \n" ${progName}
    printf "\n     hic_number [integer] is compulsory                     \n"
    printf "                                                              \n"
    printf "    Options :                                                 \n"
    printf "${usageOptionsTemplate}"   "-f" "--force-create"	"[true|false]" 	"Force directory creation on eos. Default ${forceCreate}"
    printf "${usageOptionsTemplate}"   "-t" "--test-mode"	"[true|false]" 	"Use mft/testHIC instead of mft/PRODUCTION. Default ${testMode}"
    printf "${usageOptionsTemplate}"   "-u" "--user"		"[cernid]" 	"User name for ssh copy. Default ${userRsync}"
    printf "                                                            \n"
    printf "        Example: ./mft_data_transfer.sh 71 --user fguillou  \n"
}
#### concatenate all the files from one folder
run_datatransfer()
{
	printf "Start : run_datatransfer() ... \n"
	if [ ! -d "${dataPath}" ]
	then
		printf "\t%15s does not exist.\n" ${dataPath}
		exit 1
	else
		if [ "${forceCreate}" == "true" ]
		then
			rsync -e ssh -avz --rsync-path="mkdir -p ${dataPathOnEos} && rsync" ${dataPath} ${userRsync}@lxplus7.cern.ch:${dataPathOnEos}
		else
			rsync -e ssh -avz ${dataPath} ${userRsync}@lxplus7.cern.ch:${dataPathOnEos}
		fi
	fi
	printf "Stop : run_datatransfer()\n"
exit 0
}


## Parse parameters
while [ $# -gt 0 ]
do
   case "$1" in
   	[0-9]*)
	   	if [[ $1 =~ ${isInt} ]]
   	   	then
			hicNumber=$1
			shift
 	   	fi
	   	;;
	"-f" | "--force-create")
		if [[ $2 =~ true|false ]]
		then
		   forceCreate=$2
		else
		   printf "\n\t === option %s : %s is not valid. Should be \"true\" or \"false\"\n" $1 $2
		   usage
		   exit 1
		fi
		shift
		shift
		;;
	"-t" | "--test-mode")
		if [[ $2 =~ true|false ]]
		then
		   testMode=$2
		else
		   printf "\n\t === option %s : %s is not valid. Should be \"true\" or \"false\"\n" $1 $2
		   usage
		   exit 1
		fi
		shift
		shift
		;;
	"-u" | "--user")
		userRsync=$2
		shift
		shift
		;;
	"-h" | "--help")
		usage
		exit 0
		;;
	*)

		printf "\n\t === Unknow option : %s\n" "$1"
		usage
		exit 1
		;;
   esac
done
if [ "${hicNumber}" == "" ]
then
	printf "HIC number requied. EXIT\n"
	exit 1
fi
dataPath=$( printf "./Data/%s/" "${hicNumber}")
if [ "${testMode}" == "true" ]
then
	dataPathOnEos=$( printf "/eos/project/m/mft/testHIC/HIC_Ladder/HIC_Ladder_%d/ladder_func/" "${hicNumber}")
else
	dataPathOnEos=$( printf "/eos/project/m/mft/PRODUCTION/WP2_Ladder/HIC_Ladder/HIC_Ladder_%d/ladder_func/" "${hicNumber}")
fi
run_datatransfer
exit 0

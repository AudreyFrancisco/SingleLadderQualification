#!/bin/bash
#
# Excute this script to prepare the running environment for the GUI using:
#
# source env.sh
# ./GUI

# setup environment / load functions
ROOT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd ) # determine where this script is located

export LD_LIBRARY_PATH=$(readlink -f ${ROOT_DIR}/../):$LD_LIBRARY_PATH # add the main folder with libalpide.so and libalpide_analysis.so

#For Mac users : comment the above line and uncomment the following one
#export LD_LIBRARY_PATH="$(greadlink -f ../):"$LD_LIBRARY_PATH && DYLD_LIBRARY_PATH="$(greadlink -f ../):"$DYLD_LIBRARY_PATH

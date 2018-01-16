#!/bin/bash

# change if your executable is named different
CLANG_FORMAT=clang-format

# add all the files and directories that may not be reformatted,
# relative to the project's root directory
IGNORE_SET=(
    analysis
    ui_
    moc_
)

####################################################################################

function join { local IFS="$1"; shift; echo "$*"; }
IGNORE_STRING=$(join \| "${IGNORE_SET[@]}")

SOURCES=$(find . | egrep -v ${IGNORE_STRING} | egrep "\.h$|\.hh$|\.c$|\.cc$|\.C$")

for FILE in $SOURCES
do
    ${CLANG_FORMAT} -i $FILE
done

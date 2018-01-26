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

SOURCES=$(find . | egrep -v ${IGNORE_STRING} | egrep "\.h$|\.hh$|\.c$|\.cc$|\.C$|\.cpp$")

os_version=$(lsb_release -d | grep "CentOS Linux release 7" | wc -l 2> /dev/null )
if [[ "$os_version" -ne 1 ]]
then
    "automatic formatting only available on CentOS Linux release 7"
    exit 3
fi

if [[ !"$(clang-format --version | sed -nre 's/^[^0-9]*(([0-9]+\.)*[0-9]+).*/\1/p' | tr -d '.' 2>/dev/null)" -gt 400 ]]
then
    "automatic formatting only available on if clang-format 4.0.0 or newer are available";
    exit 2
fi


echo "Formatting..."
for FILE in $SOURCES
do
    var=$(${CLANG_FORMAT} "$FILE" | diff "$FILE" - | wc -l)
    if [[ "$var" -ne 0 ]]
    then
        echo $FILE
        ${CLANG_FORMAT} -i $FILE
    fi
done
echo "done."

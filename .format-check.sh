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

EXIT_VAL=0

####################################################################################

function join { local IFS="$1"; shift; echo "$*"; }
IGNORE_STRING=$(join \| "${IGNORE_SET[@]}")

SOURCES=$(find . | egrep -v ${IGNORE_STRING} | egrep "\.h$|\.hh$|\.c$|\.cc$|\.C$|\.cpp$")

echo "Checking format with clang-format version:"
$CLANG_FORMAT --version

echo "Checking formatting..."
for FILE in $SOURCES
do
    var=$(${CLANG_FORMAT} "$FILE" | diff "$FILE" - | wc -l)
    if [[ "$var" -ne 0 ]]
    then
        echo "$(basename $FILE) does not respect the coding style:"
        ${CLANG_FORMAT} "$FILE" | diff -u "$FILE" -
        EXIT_VAL=1
    fi
done
if [[ "$EXIT_VAL" -eq 0 ]]
then
    echo "done."
else
    echo "finished, format errors fount!"
fi
exit $EXIT_VAL

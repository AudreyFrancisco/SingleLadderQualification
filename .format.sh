#!/bin/bash

# change if your executable is named different
CLANG_FORMAT=clang-format

# add all the files and directories that may not be reformatted,
# relative to the project's root directory
IGNORE_SET=(

    ui_
    moc_
)

####################################################################################

function join { local IFS="$1"; shift; echo "$*"; }
IGNORE_STRING=$(join \| "${IGNORE_SET[@]}")

SOURCES=$(find . | egrep -v ${IGNORE_STRING} | egrep "\.h$|\.hh$|\.c$|\.cc$|\.C$|\.cpp$")

if [ "$(uname)" == "Darwin" ]
then
    if [[ "$(clang-format --version)" != *4.0.1* ]]
    then
        echo "automatic formatting on MAC OS X only available if clang-format 4.0.1";
        exit 2
    fi
else
    CLANG_VERSION=$(clang-format --version | sed -nre 's/^[^0-9]*(([0-9]+\.)*[0-9]+).*/\1/p' | tr -d '.' 2>/dev/null)
    if [[ "${CLANG_VERSION}" -lt 400 ]] || [[ "${CLANG_VERSION}" -gt 401 ]]
    then
        echo "automatic formatting only available if clang-format 4.0.0 or 4.0.1 are available";
        exit 2
    fi
fi

echo "Formatting..."
for FILE in $SOURCES
do
    var=$(${CLANG_FORMAT} "$FILE" | diff "$FILE" - | wc -l)
    if [[ "$var" -ne 0 ]]
    then
        echo $FILE
        ${CLANG_FORMAT} -style=file -i $FILE
    fi
done
echo "done."

#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd ${DIR}

./HP_E3631A.py 2
./HP_E3631A.py 3

./hameg.py 3
./hameg.py 5

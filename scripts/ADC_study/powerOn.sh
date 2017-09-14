#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd ${DIR}

./HP_E3631A.py 0
./HP_E3631A.py 1
./HP_E3631A.py 2

./hameg.py 0
./hameg.py 1
./hameg.py 3

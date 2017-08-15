#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd ${DIR}

./huber.py 2 
./huber.py 4

./HP_E3631A.py 2
./HP_E3631A.py 3

./hameg.py 4
./hameg.py 6

#!/bin/bash -x

sed -i 's/ADDRESS 192.168.168.*/ADDRESS 192.168.168.'$1'/g' Config/Config_MFTLadder*

#!/bin/bash -x

sed -i 's/#ENABLED_'$1'/ENABLED_'$1'/g' Config/Config_MFTLadder*

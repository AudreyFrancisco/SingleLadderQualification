#!/bin/bash

echo " ------ ALICE-ITS EOS transfer istallation script - v.1.0 - A.Franco - INFN BARI Italy" 
echo " "
echo " Please authenticate Kerberos ticket "
echo " "
kinit 
echo " Now we build the EOStransfer.sh script "
echo " "
klist -l >/tmp/krblist
sudo ./EOSconfig.py
rm /tmp/krblist
echo " Finally setup the Cron Job task "
echo " "
./EOScreateCron.sh
echo " "
echo " Installation Done !"




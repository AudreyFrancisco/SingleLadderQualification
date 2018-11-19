#!/bin/bash

echo " ------ ALICE-ITS EOS transfer istallation script - v.1.0 - A.Franco - INFN BARI Italy" 
echo " "
read -p " Type the the Service Account Name for this site, followed by [ENTER]:" SERVICEACCOUNT
echo " "
echo " Please authenticate Kerberos ticket "
echo " "
kinit ${SERVICEACCOUNT}@CERN.CH 
echo " "
echo " Now we build the EOStransfer.sh script "
echo " "
klist -l >/tmp/krblist
sudo ./EOSconfig.py
rm /tmp/krblist
echo " Finally setup the Cron Job task "
echo " "
sudo ./EOSsetupSSHconfig.sh
./EOScreateCronJob.sh
echo " "
if [ ! -f /tmp/EOStransfer.log ] ; then 
  touch /tmp/EOStransfer.log
fi
rm -f /tmp/EOStransferlock.loc
echo " Installation Done !"


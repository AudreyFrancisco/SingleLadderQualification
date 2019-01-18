#!/bin/bash

echo " ------ ALICE-ITS EOS transfer ssh config istallation script - v.1.0 - A.Franco - INFN BARI Italy" 

# Builds the command line
SCRIPTSPATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SSHCONFIGFILE="/etc/ssh/ssh_config"
echo " Check and configure the SSH client config = $SSHCONFIGFILE"

if [ ! -f $SSHCONFIGFILE ];
then
    echo " ERROR file $SSHCONFIGFILE not found. Abort!"
    exit 1
fi

cat $SSHCONFIGFILE | grep "ALICE-ITS"
if [[ $? -gt 0 ]] 
then
    echo " No previous installation was done..."
    echo "# ALICE-ITS : Kerberos set up for DB activities on LXPLUS.CERN.CH" >> $SSHCONFIGFILE
    echo "Host lxplus*" >> $SSHCONFIGFILE
    echo "      GSSAPIAuthentication yes" >> $SSHCONFIGFILE
    echo "      GSSAPIDelegateCredentials yes" >> $SSHCONFIGFILE
    echo "      GSSAPITrustDNS yes" >> $SSHCONFIGFILE
    echo "# ---------------------" >> $SSHCONFIGFILE
fi
echo " Installation DONE !"
# ---------- EOF ------

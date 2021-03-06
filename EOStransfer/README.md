
====================================================

ALICE - ITS - DB EOS transfer utility

====================================================

	INFN - Sezione di Bari  - ITALY
	
	Date : 13 April  2018
	Author : Antonio Franco 
			 antonio.franco@ba.infn.it
	
	ver. 2.0.r1
----------------------------------------------------

	** HISTORY **
	
	09/07/2018 - Release v1.0.r1
	22/07/2018 - Release v2.0.r1 - Redesign of the sync with the DBParameters.dat semaphore file
  
----------------------------------------------------
	
    ** INSTALLATION  **
    
  Test version OS : CERN CentOS ver.7.3

  - Kerberos (if it isn't already installed)  
>    yum -y install ntp  
>    ntpdate ip-time-1.cern.ch  
>    systemctl start  ntpd.service  
>    systemctl enable ntpd.service  
>    yum -y install krb5-*  

  - Install packages:
>    sudo yum install git  
>    sudo yum install cronie  


----------------------------------------------------
		
    ** RUN  **

	Launch the configuration program: first it will ask for the Site Service Account Name,
	then its password in order to make the Kerberos authentication; in addition the password
	of the logged user (must be a sudoer) in order to create some directories under the '/var/'
	filesystem node:

>     [fap@localhost ~]cd ~/its-DB-EOStransfer/src
>     [fap@localhost src]$ ./install.sh
>     ------ ALICE-ITS EOS transfer istallation script - v.1.0 - A.Franco - INFN BARI Italy
>     Type the the Service Account Name for this site, followed by [ENTER]:aliceitsbari
>     Please authenticate Kerberos ticket 
>     Password for aliceitsbari@CERN.CH: 
>    
>     Now we build the EOStransfer.sh script 
>     [sudo] password for fap: 
>    
  Now the program that builds the EOStransfer.py program is executed. It will ask
  the 'Number of Attempts' to syncronize before an error will be raised (default 3),
  then the 'Type of Activity' will be done on the machine.

>     ******************************************************* 
>     *  ALICE ITS : EOS transfer configuration program     * 
>     *  ver. 2.0 - 28/08/2018    Auth : A.Franco INFN Bari * 
>     *                                                     * 
>     ******************************************************* 
>     
>     >>-> You are at Bari site. The related service account is  'aliceitsbari' 
>     The number of attempts to rsync : [3]:3
>    
>     ** Select the Activity :  menu list   **
>            1)       HicTests 
>            2)       fpc 
>            0)       Quit 
>             Select the choice :1
>     >>-> You select HicTests activity
>    
>     ** Select one or more Hic Test :  menu list   **
>            1)       IBEndurance 
>            2)       IBQualification 
>            3)       OBEndurance 
>            4)       OBFastPower 
>            5)       OBQualification 
>            6)       OBReception 
>            7)       OBImpedance 
>            0)       Done 
>             Select the choice :7
>     >>-> You select OBImpedance test 

  The next information is the path of Source Data Directory, this represents
  the base path node of the local files repository. All files and subfolders
  contained here will be syncronized on the EOS remote repositroy.
>    
>     Specify the position of the sources path []:/home/fap/Data

  Finally a 'Done' choiche (0) ends the program.
>    
>     ** Select one or more Hic Test :  menu list   **
>            1)       IBEndurance 
>            2)       IBQualification 
>            3)       OBEndurance 
>            4)       OBFastPower 
>            5)       OBQualification 
>            6)       OBReception 
>            7)       OBImpedance 
>            0)       Done 
>             Select the choice :0
>     WARNING : File /home/fap/its-DB-EOStransfer/src/EOStransfer.sh already exists. Overwrite !
>     >>-> Script file /home/fap/its-DB-EOStransfer/src/EOStransfer.sh was created !
>     EOS configuration program. Done !

  The last step of installation is the setup of the cronjob task. The command to synchronize the
  repositories (EOStransfer.sh) will be executed periodically every 10 minutes, if you want 
  change this period you must edit the crontab entry (please refer on the crontab manual).  

>     Finally setup the Cron Job task 
>     ------ ALICE-ITS EOS transfer cron job istallation script - v.1.0 - A.Franco - INFN BARI Italy
>     The Cron command is = /home/fap/its-DB-EOStransfer/src/EOStransfer.sh
>     */10 * * * * /home/fap/its-DB-EOStransfer/src/EOStransfer.sh &>/tmp/lastEOSCronExecution.log
>     The Cron command is already installed. Exit ! [2]
>      ------ ALICE-ITS FPC bench cron job istallation script: Terminate
>     Installation Done !

  At the end of installation in this folder will be created: 
    1) the script 'syncAll.sh' : this command make a COMPLETE synchronization of the local
       repository;
    2) the script 'autosyncOff.sh' : this command STOPs the automatic synchronization of the local
       repository, the command must be runned as superuser;
    3) the script 'autosyncOn.sh' : this command STARTs the automatic synchronization of the local
       repository, the command must be runned as superuser;
    4) the script 'EOStransfer.sh' : this command is launched by the cron daemon every 10 minutes;


  Note : two log files are produced:  
      1) the '/tmp/lastEOSCronExecution.log', it contains all the messages related to the last
         synchronization event.  
      2) the '/tmp/EOStransfer.log' that contains the list of all the executed syncronizations
         done.  
	
	
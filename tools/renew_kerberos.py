#!/usr/bin/env python

from datetime import datetime
from dateutil.parser import parse
import time
import random
import subprocess
from subprocess import Popen,PIPE
from getpass import getpass
import re
import os
import ConfigParser

config=ConfigParser.ConfigParser()
script_path=os.path.dirname(os.path.realpath(__file__))
config.read(script_path + '/setup.cfg')

USER=config.get('DEFAULT','login')

pwd=getpass('Enter password for user "%s": '%USER);

def check_kerberos():
  try:
    output = subprocess.check_output(["klist"])
    matches = re.match(r"(.*)renew until (.*?)$(.*)", output, re.DOTALL|re.M)
    if matches:
      renew_until = parse(matches.group(2))
      time_left = (renew_until - datetime.now()).total_seconds()
      
      return 1 if time_left > 3600*12 else 0
    return 0
  except:
    return 0

first=True
while 1:
  subprocess.call("date")
  print 'Trying to renew kerberos token...'
    
  # renew kerberos
  retcode = subprocess.call(["kinit", USER, "-R"])

  if first or (retcode != 0) or (not check_kerberos()):
    if not first:
      print 'Renewing failed. Trying to reinitialise token...'
    p=Popen(['kinit',USER,'-r','2000h'],stdin=PIPE,stdout=PIPE,shell=True)
    p.communicate(pwd+'\n')
    if p.returncode!=0:
      print 'ERROR: please check!'
      break
    first = False
  else:
    print 'done.'

  print 'Sleeping fo 2 min.'
  time.sleep(120)


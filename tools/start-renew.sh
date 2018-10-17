. ~/.bashrc
HISTFILE=
NOW=`date +%y%m%d`
echo -en "\033]0;Kerberos Token\a"
python ./renew_kerberos.py
echo
echo "!!! Restart script with ./start-renew.sh !!!"
echo

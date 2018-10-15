# Mosaic Network settings

There is three important kernel parameters which can be read out using:

```sh
cat /proc/sys/net/ipv4/ipfrag_high_thresh
cat /proc/sys/net/core/rmem_max
cat /proc/sys/net/ipv4/tcp_rmem
```

If the values are below:
```sh
net.ipv4.ipfrag_high_thresh = 1000000
net.core.rmem_max = 16777216
net.ipv4.tcp_rmem = 4096 87380 16777216
```
please fix them by adding the following lines have to be added to `/etc/sysctl.conf`:
```sh
net.ipv4.ipfrag_high_thresh = 1000000
net.core.rmem_max = 16777216
net.ipv4.tcp_rmem = 4096 87380 16777216
```
This settings can be activated by a reboot or invocation of `sysctl -p`.

After every reboot, furthermore, the following lines should be executed as root:
```sh
ip route add 192.168.168.250 advmss 32000 dev enp0s31f6
ip route add 192.168.168.251 advmss 32000 dev enp0s31f6
```
In this case, '192.168.168.250' and '192.168.168.251' are the IP addresses of the MOSAICs connected to the PC via the network interface 'enp0s31f6'. The IP addresses and interface name have to be adjusted according to your setup. The name of the network interface can be obtained using `ifconfig -a`. The ADVMSS setting can be verified using `netstat -rn`.

*Important* the advmss gets lost everytime the network connection is lost or the system is rebooted. Giuseppe has described [here](http://sunba2.ba.infn.it/MOSAIC/General/Documents/NetworkSettings.txt) how to make these settings permanent.

The MTU size of the network interface can stay at 1500, as the MOSAIC does not exceed this MTU size. It can be verified using `ifconfig -a` and set using `ifconfig <interface name> mtu 1500`.

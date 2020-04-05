brctl addbr br0
ip addr flush dev enx24f5a2f2599a
brctl addif br0 enx24f5a2f2599a
tunctl -t tap0 -u `whoami`
brctl addif br0 tap0
ifconfig enx24f5a2f2599a up
ifconfig tap0 up
ifconfig br0 up
brctl show
dhclient -v br0
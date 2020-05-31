brctl delif br0 tap0
tunctl -d tap0
brctl delif br0 enx24f5a2f2599a
ifconfig br0 down
brctl delbr br0
ifconfig enx24f5a2f2599a up
dhclient -v enx24f5a2f2599a

ip link add name br0 type bridge
ip addr add 172.20.0.1/16 dev br0
ip link set br0 up
dnsmasq --interface=br0 --bind-interfaces --dhcp-range=172.20.0.2,172.20.255.254

modprobe tun

# [[ ! -d /etc/qemu ]] && mkdir /etc/qemu
# echo allow br0 > /etc/qemu/bridge.conf

sysctl net.ipv4.ip_forward=1
sysctl net.ipv6.conf.default.forwarding=1
sysctl net.ipv6.conf.all.forwarding=1

iptables -t nat -A POSTROUTING -o wlp2s0 -j MASQUERADE
iptables -A FORWARD -m conntrack --ctstate RELATED,ESTABLISHED -j ACCEPT
iptables -A FORWARD -i tap0 -o wlp2s0 -j ACCEPT

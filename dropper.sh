sudo iptables -A INPUT -m statistic -p udp --mode random --probability 0.5 -j DROP


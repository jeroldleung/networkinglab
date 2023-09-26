# Checkpoint 4 Writeup

- **NetworkInterface**: is the connection between the network layer and the link layer that translate IP datagrams into Ethernet frames and vice versa. (In a real system, network interface typically have names like *eth0*, *eth1*, *wlan0*, etc.)

## Design of the NetworkInterface

The network interface has its own ip address and ethernet address. Generally, the network interface is the actual implementation of the **Address Resolution Protocol**, or **ARP**. It maintains a table that mapping the ip address and the ethernet address. Every time an ip datagram comes in, it encapsulate the ip datagram to the ethernet frame with a mapping ehternet address. If the mapping ethernet address not found in the table, it will broadcast an ARP request that asks "Who claims the following ip address? What's your ethernet address?" and waits for a response.

## Program Structure 

The ip address and ethernet address mapping table should record each entry's time of existence. Every entry are remembered for 30 seconds and being erased that have expired in the `tick` method.

The `send_datagram` method takes an ip datagram and the address of the next hop as arguments. It encapsulate the ip datagram to the ethernet frame with the corresponding next hop's ethernet address. If the ethernet address not found in the table, queue the ip datagram and send an ARP request to ask for it. To be notice that we don't want to flood the network with ARP requests. If the network interface already sent an ARP request about the same ip address in the last 5 second, don't send a second request, just wait for a reply to the first one. Again, queue the ip datagram until it learn the destination ethernet address.

The `recv_frame` method receive ethernet frame. If the inbound frame's type is ipv4, parse the payload as an ip datagram and return the resulting ip datagram to the caller. Otherwise, if the inbound frame's type is ARP, learn the ip address and the ethernet address from it. Furthermore, if it is an ARP request and asking for our ip address, send an appropriate ARP reply that telling our ethernet address. Since we got a new mapping, don't forget to send the queued ip datagrams which didn't know the target ethernet address before.


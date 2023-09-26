# Checkpoint 5 Writeup

- **IP router**: is to forward the datagrams that it gets according to the **routing table**.

## Design of the Router

A router has several network interfaces, and can receive Internet datagrams on any of them. The `Router` class keep track of a routing table and forward each datagram it receives to the correct next hop on the correct outgoing `NetworkInterface`.

## Program Structure

The `add_router` method adds a route to the routing table which contain information about the route prefex, prefix length, address of next hop and the interface index for each entry.

The `route` method needs to route each incoming datagram to the next hop, out the appropriate interface. It needs to implement the **longest-prefix match** logic to find the best route to follow. If no routes match, the router drops the datagram. When route matches, it needs to decrement the datagram's TTL, re-compute the checksum and send to the appropriate network interface. If the TTL was already zero or hits zero after the decrement, the router should drop the datagram.

## Implementation Challenges

I have bug in routing package on the same network. When I debug, I found the bug lies in network interface, the code should ignore frames that are not destined for the network interface, which means that only the target ethernet address is broadcast address or interface's own ethernet address is available for receiving, but I consider the ip address which will generate bugs.


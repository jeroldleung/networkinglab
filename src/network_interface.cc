#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  auto mapping = arp_table_.find( next_hop.ipv4_numeric() );
  EthernetFrame ef;

  if ( mapping != arp_table_.end() ) {
    // destination Ethernet address is already known
    ef.header = { mapping->second.first, ethernet_address_, EthernetHeader::TYPE_IPv4 };
    ef.payload = serialize( dgram );
  } else if ( mapping == arp_table_.end() || arp_sent_passage_ > 5000 ) {
    // destination Ethernet address is unknown
    // send an arp broadcast message if 5 seconds have passed
    ARPMessage arpmsg_request = {
      .opcode = ARPMessage::OPCODE_REQUEST,
      .sender_ethernet_address = ethernet_address_,
      .sender_ip_address = ip_address_.ipv4_numeric(),
      .target_ip_address = next_hop.ipv4_numeric(),
    };
    ef.header = { ETHERNET_BROADCAST, ethernet_address_, EthernetHeader::TYPE_ARP };
    ef.payload = serialize( arpmsg_request );
    arp_sent_passage_ = 0;
  } else {
    return;
  }

  ready_to_send_.push( ef );
}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  // IPv4
  InternetDatagram ipmsg;
  if ( frame.header.type == EthernetHeader::TYPE_IPv4 && parse( ipmsg, frame.payload ) ) {
    return ipmsg;
  }

  // arp
  ARPMessage arpmsg;
  if ( frame.header.type == EthernetHeader::TYPE_ARP && parse( arpmsg, frame.payload ) ) {
    arp_table_[arpmsg.sender_ip_address] = make_pair( arpmsg.sender_ethernet_address, 0 );
    auto iter = arp_table_.find( arpmsg.target_ip_address );
    // asking for our IP address
    if ( arpmsg.opcode == ARPMessage::OPCODE_REQUEST && iter != arp_table_.end() ) {
      EthernetFrame ef;
      ARPMessage arpmsg_reply = {
        .opcode = ARPMessage::OPCODE_REPLY,
        .sender_ethernet_address = ethernet_address_,
        .sender_ip_address = ip_address_.ipv4_numeric(),
        .target_ethernet_address = arpmsg.sender_ethernet_address,
        .target_ip_address = arpmsg.sender_ip_address,
      };
      ef.header = { arpmsg.sender_ethernet_address, ethernet_address_, EthernetHeader::TYPE_ARP };
      ef.payload = serialize( arpmsg_reply );
      ready_to_send_.push( ef );
    }
  }

  return {};
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  arp_sent_passage_ += ms_since_last_tick;

  for ( auto iter = arp_table_.begin(); iter != arp_table_.end(); ) {
    iter->second.second += ms_since_last_tick;
    if ( iter->second.second > 300000 ) {
      // IP-to-Ethernet mappings that have expired
      iter = arp_table_.erase( iter );
    } else {
      ++iter;
    }
  }
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if ( ready_to_send_.empty() )
    return {};

  EthernetFrame ef = ready_to_send_.front();
  ready_to_send_.pop();
  return ef;
}

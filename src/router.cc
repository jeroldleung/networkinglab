#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  routing_table_.emplace_back( route_prefix, prefix_length, next_hop, interface_num );
}

void Router::route()
{
  for ( auto& itf : interfaces_ ) {
    auto datagram = move( itf.maybe_receive() );
    if ( !datagram.has_value() )
      continue;
    // ttl was zero or hits zero after decrement, drop the datagram
    if ( datagram.value().header.ttl == 0 || --datagram.value().header.ttl == 0 )
      continue;

    // find the "longest-prefix match" entry
    vector<Entry>::const_iterator match_entry = routing_table_.end();
    for ( auto iter = routing_table_.begin(); iter != routing_table_.end(); ++iter ) {
      // match the route prefix
      if ( ( iter->route_prefix & datagram.value().header.dst ) == iter->route_prefix ) {
        // the longest-prefix
        if ( match_entry == routing_table_.end() || match_entry->prefix_length < iter->prefix_length ) {
          match_entry = iter;
        }
      }
    }

    // no match entry
    if ( match_entry == routing_table_.end() )
      continue;

    // recompute header checksum after decrement ttl
    datagram.value().header.compute_checksum();

    // found the match entry, route datagram to the target interface
    interface( match_entry->interface_num )
      .send_datagram( move( datagram.value() ),
                      match_entry->next_hop.value_or( Address::from_ipv4_numeric( datagram.value().header.dst ) ) );
  }
}

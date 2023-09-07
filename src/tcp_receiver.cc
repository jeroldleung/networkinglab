#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  if ( message.SYN ) {
    is_syned = true;
    zero_point = message.seqno;
  }

  if ( is_syned && message.FIN ) {
    is_fined = true;
  }

  reassembler.insert( message.SYN ? 0 : message.seqno.unwrap( zero_point, inbound_stream.bytes_pushed() ) - 1,
                      message.payload.release(),
                      message.FIN,
                      inbound_stream );
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  return TCPReceiverMessage {
    is_syned
      ? make_optional<Wrap32>( Wrap32::wrap( inbound_stream.bytes_pushed() + is_syned + is_fined, zero_point ) )
      : nullopt,
    static_cast<uint16_t>( min( inbound_stream.available_capacity(), ( 1UL << 16 ) - 1 ) ) };
}

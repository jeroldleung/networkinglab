#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  if ( message.SYN ) {
    is_syned = true;
    zero_point = message.seqno;
  }

  reassembler.insert( message.SYN ? 0 : message.seqno.unwrap( zero_point, inbound_stream.bytes_pushed() ) - 1,
                      message.payload.release(),
                      message.FIN,
                      inbound_stream );
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  const uint16_t window_size = min( inbound_stream.available_capacity(), ( 1UL << 16 ) - 1 );
  if ( !is_syned ) {
    return TCPReceiverMessage { nullopt, window_size };
  }
  return TCPReceiverMessage {
    Wrap32::wrap( inbound_stream.bytes_pushed() + is_syned + inbound_stream.is_closed(), zero_point ),
    window_size };
}

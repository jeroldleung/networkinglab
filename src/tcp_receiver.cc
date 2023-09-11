#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  if ( message.SYN ) {
    isn_ = message.seqno;
  }

  if ( !isn_.has_value() ) {
    return;
  }

  const uint64_t checkpoint = inbound_stream.bytes_pushed();
  const uint64_t absolute_seqno = message.seqno.unwrap( isn_.value(), checkpoint );
  const uint64_t first_index = message.SYN ? 0 : absolute_seqno - 1;

  reassembler.insert( first_index, message.payload.release(), message.FIN, inbound_stream );
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  TCPReceiverMessage msg;

  const auto win_sz = inbound_stream.available_capacity();
  msg.window_size = win_sz < UINT16_MAX ? win_sz : UINT16_MAX;

  if ( isn_.has_value() ) {
    const uint64_t absolute_seqno = inbound_stream.bytes_pushed() + 1 + inbound_stream.is_closed();
    msg.ackno = Wrap32::wrap( absolute_seqno, isn_.value() );
  }

  return msg;
}

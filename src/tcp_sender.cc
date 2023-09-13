#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return seqnos_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return consecutive_retransmissions_num_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  if ( ready_to_send_.empty() ) {
    return {};
  }

  outstanding_.push_back( move( ready_to_send_.front() ) );
  ready_to_send_.pop();

  return outstanding_.back();
}

// generate TCPSenderMessage and push to the ready_to_send_ queue
void TCPSender::push( Reader& outbound_stream )
{
  TCPSenderMessage sender_msg;

  if ( !is_syned ) {
    is_syned = true;
    sender_msg.seqno = isn_;
    sender_msg.SYN = true;
  }

  string data;
  auto payload_size = min( TCPConfig::MAX_PAYLOAD_SIZE, window_size_ );
  if ( outbound_stream.bytes_buffered() ) {
    payload_size = min( payload_size, outbound_stream.peek().size() );
    sender_msg.seqno = Wrap32::wrap( outbound_stream.bytes_popped() + is_syned, isn_ );
    read( outbound_stream, payload_size, data );
    sender_msg.payload = data;
    window_size_ -= data.size();
  }

  if ( outbound_stream.is_finished() && window_size_ > 0 ) {
    sender_msg.seqno = Wrap32::wrap( seqnos_sent_, isn_ );
    sender_msg.FIN = true;
    window_size_ -= 1;
  }

  if ( sender_msg.SYN || sender_msg.FIN || !sender_msg.payload.empty() ) {
    seqnos_sent_ += sender_msg.sequence_length();
    seqnos_in_flight_ += sender_msg.sequence_length();
    ready_to_send_.push( move( sender_msg ) );
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  return TCPSenderMessage { isn_ + seqnos_sent_ };
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // update the window size
  window_size_ = msg.window_size;

  if ( !msg.ackno.has_value() ) {
    return;
  }

  const auto absolute_ackno = msg.ackno.value().unwrap( isn_, seqnos_sent_ );

  // ignore ackno that beyond next seqno
  const auto absolute_seqno = ( isn_ + seqnos_sent_ ).unwrap( isn_, seqnos_sent_ );
  if ( absolute_ackno > absolute_seqno ) {
    return;
  }

  // remove the outstanding segment that has fully acknowledged
  for ( auto iter = outstanding_.begin(); iter != outstanding_.end(); ) {
    const auto absolute_seqend = iter->seqno.unwrap( isn_, seqnos_sent_ ) + iter->sequence_length();
    if ( absolute_seqend <= absolute_ackno ) {
      seqnos_in_flight_ -= iter->sequence_length();
      iter = outstanding_.erase( iter );
    } else {
      ++iter;
    }
  }

  // set RTO to initial value
  // set consecutive retransmissions number back to 0
  current_RTO_ms_ = initial_RTO_ms_;
  consecutive_retransmissions_num_ = 0;
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  time_passage_ += ms_since_last_tick;
  if ( time_passage_ < current_RTO_ms_ ) {
    return;
  }

  // retransmit
  auto earliest = outstanding_.begin();
  for ( auto iter = outstanding_.begin(); iter != outstanding_.end(); ++iter ) {
    const auto absolute_seqno = iter->seqno.unwrap( isn_, seqnos_sent_ );
    const auto earliest_seqno = earliest->seqno.unwrap( isn_, seqnos_sent_ );
    earliest = absolute_seqno < earliest_seqno ? iter : earliest;
  }
  ready_to_send_.push( move( *earliest ) );
  outstanding_.erase( earliest );

  // slows down retransmissions on lousy networks to avoid further gumming up the works
  time_passage_ = 0;
  current_RTO_ms_ *= 2;

  // increment consecutive retransmissions number
  consecutive_retransmissions_num_ += 1;
}

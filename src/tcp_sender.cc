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
  return seqnums_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return retransmissions_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  if ( ready_to_send_.empty() ) {
    return {};
  }

  TCPSenderMessage sender_msg = ready_to_send_.front();
  ready_to_send_.pop();

  return sender_msg;
}

void TCPSender::push( Reader& outbound_stream )
{
  const size_t available_win_sz = window_size_ == 0 ? 1 : window_size_;

  // generate as many TCPSenderMessages as possible
  while ( seqnums_in_flight_ < available_win_sz ) {
    TCPSenderMessage sender_msg;

    // synchronize
    if ( !syn_ ) {
      syn_ = sender_msg.SYN = true;
      seqnums_in_flight_ += 1;
    }

    sender_msg.seqno = Wrap32::wrap( seqnums_sent_, isn_ );

    // payload
    auto payload_size = min( TCPConfig::MAX_PAYLOAD_SIZE, available_win_sz - seqnums_in_flight_ );
    read( outbound_stream, payload_size, sender_msg.payload );
    seqnums_in_flight_ += sender_msg.payload.size();

    // finish
    if ( outbound_stream.is_finished() && seqnums_in_flight_ < available_win_sz && !fin_ ) {
      fin_ = sender_msg.FIN = true;
      seqnums_in_flight_ += 1;
    }

    if ( sender_msg.sequence_length() == 0 ) {
      break;
    }

    seqnums_sent_ += sender_msg.sequence_length();
    outstanding_.push( sender_msg );
    ready_to_send_.push( sender_msg );

    if ( sender_msg.FIN || outbound_stream.bytes_buffered() == 0 ) {
      break;
    }
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  return TCPSenderMessage { isn_ + seqnums_sent_ };
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // update the window size
  window_size_ = msg.window_size;

  if ( !msg.ackno.has_value() ) {
    return;
  }

  const auto absolute_ackno = msg.ackno.value().unwrap( isn_, seqnums_sent_ );

  // ignore ackno that beyond next seqno
  if ( absolute_ackno > seqnums_sent_ ) {
    return;
  }

  // remove the outstanding segment that has fully acknowledged
  while ( !outstanding_.empty() ) {
    const auto segment_end
      = outstanding_.front().seqno.unwrap( isn_, seqnums_sent_ ) + outstanding_.front().sequence_length();
    if ( segment_end <= absolute_ackno ) {
      seqnums_in_flight_ -= outstanding_.front().sequence_length();
      outstanding_.pop();
      timer.start();
    } else {
      break;
    }
  }

  // set RTO to initial value
  // set consecutive retransmissions number back to 0
  timer.set_rto( initial_RTO_ms_ );
  retransmissions_ = 0;
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  timer.tick( ms_since_last_tick );
  if ( !timer.is_expired() || outstanding_.empty() ) {
    return;
  }

  // retransmit
  ready_to_send_.push( outstanding_.front() );

  // slows down retransmissions on lousy networks to avoid further gumming up the works
  timer.start();
  if ( window_size_ != 0 ) {
    timer.double_rto();
  }

  // increment consecutive retransmissions number
  retransmissions_ += 1;
}

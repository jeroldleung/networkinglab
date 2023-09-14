#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <list>
#include <queue>

class RTOTimer
{
  uint64_t rto_;
  size_t time_passage_ = 0;

public:
  explicit RTOTimer( uint64_t initial_rto ) : rto_( initial_rto ) {}

  void start() { time_passage_ = 0; }
  void set_rto( const uint64_t current_rto ) { rto_ = current_rto; }
  void tick( const size_t ms ) { time_passage_ += ms; }
  bool is_expired() const { return time_passage_ >= rto_ ? true : false; }
  void double_rto() { rto_ *= 2; }
};

class TCPSender
{
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

  uint64_t consecutive_retransmissions_num_ = 0;

  bool is_syned_ = false;
  bool is_closed_ = false;
  bool sepcial_case_ = false;
  bool back_off_RTO_ = true;
  std::optional<uint64_t> window_size_ {};
  uint64_t available_window_size_ = 0;
  uint64_t seqnos_sent_ = 0;
  uint64_t seqnos_in_flight_ = 0;
  std::list<TCPSenderMessage> outstanding_ {};
  std::queue<TCPSenderMessage> ready_to_send_ {};

  RTOTimer timer { initial_RTO_ms_ };

public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};

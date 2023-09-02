#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  if ( is_last_substring ) {
    output.close();
  }

  uint64_t expected_index = output.bytes_pushed();

  // defensive check
  if ( data.empty() || first_index >= expected_index + output.available_capacity() ) {
    return;
  }

  // bytes that are the next bytes should be pushed to the stream immediately
  if ( expected_index >= first_index && expected_index < first_index + data.size() ) {
    output.push( std::move( data.erase( 0, expected_index - first_index ) ) );
    expected_index = output.bytes_pushed();
  }

  // earlier bytes remain unknown, then insert data to substrings
  if ( expected_index < first_index ) {
    bytes_substrings_ += data.size();
    substrings_.try_emplace( std::move( first_index ), std::move( data ) );
  }

  // earlier bytes become known, then push to the stream
  for ( auto search = substrings_.find( expected_index ); search != substrings_.end();
        search = substrings_.find( expected_index ) ) {
    bytes_substrings_ -= search->second.size();
    output.push( std::move( search->second ) );
    substrings_.erase( search );
    expected_index = output.bytes_pushed();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return bytes_substrings_;
}

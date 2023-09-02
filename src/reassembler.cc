#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  uint64_t expected_index = output.bytes_pushed();

  // defensive check
  if ( first_index >= expected_index + output.available_capacity() ) {
    return;
  }

  if ( data.empty() && is_last_substring ) {
    output.close();
    return;
  }

  // bytes that are the next bytes should be pushed to the stream immediately
  if ( expected_index >= first_index && expected_index < first_index + data.size() ) {
    output.push( std::move( data.erase( 0, expected_index - first_index ) ) );
    expected_index = output.bytes_pushed();
    if ( is_last_substring ) {
      output.close();
    }
  }

  // earlier bytes remain unknown, then insert data to substrings
  if ( expected_index < first_index ) {
    bytes_substrings_ += data.size();
    excerpts_.try_emplace( std::move( first_index ), substring { data, is_last_substring } );
  }

  // earlier bytes become known, then push to the stream
  auto search = excerpts_.find( expected_index );
  if ( search != excerpts_.end() ) {
    bytes_substrings_ -= search->second.data.size();
    insert( search->first, search->second.data, search->second.is_last_substring, output );
    excerpts_.erase( search );
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return bytes_substrings_;
}

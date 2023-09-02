#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  if ( is_last_substring ) {
    output.close();
  }

  // defensive check
  if ( data.empty() || output.available_capacity() == bytes_substrings_ ) {
    return;
  }

  // bytes that are the next bytes push to the stream immediately
  if ( output.bytes_pushed() < first_index + data.size() ) {
    output.push( std::move( data.erase( 0, output.bytes_pushed() - first_index ) ) );
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return bytes_substrings_;
}

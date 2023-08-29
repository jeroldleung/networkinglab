#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), buffer_(), nbytes_( 0 ), closed_( false ), error_( false ) {}

void Writer::push( string data )
{
  // Your code here.
  buffer_.push( data );
  nbytes_ += data.size();
}

void Writer::close()
{
  // Your code here.
  closed_ = true;
}

void Writer::set_error()
{
  // Your code here.
  error_ = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return { closed_ };
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return { capacity_ - nbytes_ };
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return {};
}

string_view Reader::peek() const
{
  // Your code here.
  return {};
}

bool Reader::is_finished() const
{
  // Your code here.
  return { closed_ && ( nbytes_ == 0 ) };
}

bool Reader::has_error() const
{
  // Your code here.
  return { error_ };
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  (void)len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return {};
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return {};
}

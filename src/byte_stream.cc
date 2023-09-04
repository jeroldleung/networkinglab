#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  // defensive check
  if ( data.empty() || available_capacity() == 0 ) {
    return;
  }

  // no enough capacity to buffer the entire string
  if ( available_capacity() < data.size() ) {
    data = data.substr( 0, available_capacity() );
  }

  // push to the stream buffer
  bytes_pushed_ += data.size();
  bytes_buffered_ += data.size();
  buffer_.push( std::move( data ) );
}

void Writer::close()
{
  is_closed_ = true;
}

void Writer::set_error()
{
  has_error_ = true;
}

bool Writer::is_closed() const
{
  return is_closed_;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - bytes_buffered_;
}

uint64_t Writer::bytes_pushed() const
{
  return bytes_pushed_;
}

string_view Reader::peek() const
{
  return string_view( buffer_.front() );
}

bool Reader::is_finished() const
{
  return is_closed_ && bytes_buffered_ == 0;
}

bool Reader::has_error() const
{
  return has_error_;
}

void Reader::pop( uint64_t len )
{
  // defensive check
  if ( len > bytes_buffered() ) {
    throw out_of_range( "pop: len is greater than bytes buffered." );
  }

  bytes_popped_ += len;
  bytes_buffered_ -= len;

  // pop the entire string out of stream
  while ( len >= buffer_.front().size() && !buffer_.empty() ) {
    len -= buffer_.front().size();
    buffer_.pop();
  }

  // pop several bytes of the front string
  if ( len > 0 ) {
    buffer_.front().erase( 0, len );
  }
}

uint64_t Reader::bytes_buffered() const
{
  return bytes_buffered_;
}

uint64_t Reader::bytes_popped() const
{
  return bytes_popped_;
}

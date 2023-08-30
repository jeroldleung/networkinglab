#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity )
  : capacity_( capacity )
  , buffer_()
  , nbytes_( 0 )
  , nbytes_pushed_( 0 )
  , nbytes_popped_( 0 )
  , is_closed_( false )
  , has_error_( false )
{}

void Writer::push( string data )
{
  // Your code here.
  if ( data.empty() || available_capacity() == 0 ) {
    return;
  }

  if ( available_capacity() < data.size() ) {
    data = data.substr( 0, available_capacity() );
  }

  nbytes_ += data.size();
  nbytes_pushed_ += data.size();
  buffer_.push( std::move( data ) );
}

void Writer::close()
{
  // Your code here.
  is_closed_ = true;
}

void Writer::set_error()
{
  // Your code here.
  has_error_ = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  return is_closed_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - nbytes_;
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return nbytes_pushed_;
}

string_view Reader::peek() const
{
  // Your code here.
  return string_view( buffer_.front() );
}

bool Reader::is_finished() const
{
  // Your code here.
  return is_closed_ && nbytes_ == 0;
}

bool Reader::has_error() const
{
  // Your code here.
  return has_error_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  if ( len > bytes_buffered() ) {
    throw out_of_range( "pop: len is greater than bytes buffered." );
  }

  nbytes_ -= len;
  nbytes_popped_ += len;

  while ( len >= buffer_.front().size() && !buffer_.empty() ) {
    len -= buffer_.front().size();
    buffer_.pop();
  }

  if ( len > 0 ) {
    buffer_.front().erase( 0, len );
  }
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return nbytes_;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return nbytes_popped_;
}

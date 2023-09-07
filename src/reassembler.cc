#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // defensive check
  if ( first_index >= output.bytes_pushed() + output.available_capacity() ) {
    return;
  }

  if ( is_last_substring ) {
    should_close_ = true;
  }

  // bytes that are the next bytes should be pushed to the stream immediately
  if ( output.bytes_pushed() >= first_index && output.bytes_pushed() < first_index + data.size() ) {
    output.push( move( data.erase( 0, output.bytes_pushed() - first_index ) ) );
  }

  // prevent out of range
  data.resize( min( data.size(), output.available_capacity() - first_index ) );

  // edge cases
  if ( !data.empty() && substrs_.empty() ) {
    nbytes_substrs_ += data.size();
    substrs_.emplace_front( first_index, move( data ) );
  }
  if ( !data.empty() && first_index > substrs_.back().starti_ ) {
    nbytes_substrs_ += data.size();
    substrs_.emplace_back( first_index, move( data ) );
  }

  for ( auto iter = substrs_.begin(); iter != substrs_.end(); ) {
    // insert to the substrs_ until earlier bytes become known
    if ( !data.empty() && first_index <= iter->starti_ ) {
      nbytes_substrs_ += data.size();
      iter = substrs_.insert( iter, { first_index, move( data ) } );
    }
    // overlap with the previous one
    auto prev = std::prev( iter );
    // some part overlapped
    if ( prev != substrs_.end() && iter->starti_ >= prev->starti_
         && iter->starti_ < prev->starti_ + prev->data_.size() ) {
      const int nbytes_overlapped = min( prev->starti_ + prev->data_.size() - iter->starti_, iter->data_.size() );
      nbytes_substrs_ -= nbytes_overlapped;
      iter->data_.erase( 0, nbytes_overlapped );
      iter->starti_ += nbytes_overlapped;
      // fully overlapped
      if ( iter->data_.empty() ) {
        iter = substrs_.erase( iter );
        continue;
      }
    }
    // no overlap
    ++iter;
  }

  // earlier bytes have reached, push the following expected bytes to stream
  for ( auto iter = substrs_.begin(); iter != substrs_.end(); ) {
    if ( output.bytes_pushed() >= iter->starti_ ) {
      nbytes_substrs_ -= iter->data_.size();
      output.push( move( iter->data_.erase( 0, output.bytes_pushed() - iter->starti_ ) ) );
      iter = substrs_.erase( iter );
    } else {
      ++iter;
    }
  }

  if ( should_close_ && nbytes_substrs_ == 0 ) {
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return nbytes_substrs_;
}

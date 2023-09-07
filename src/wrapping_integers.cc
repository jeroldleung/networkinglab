#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return zero_point + ( n & UINT32_MAX );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t absolute_seqno
    = ( static_cast<uint64_t>( raw_value_ ) + ( 1UL << 32 ) - zero_point.raw_value_ ) % ( 1UL << 32 );

  absolute_seqno += ( checkpoint >> 32 ) * ( 1UL << 32 );

  if ( absolute_seqno < checkpoint ) {
    absolute_seqno += ( 1UL << 32 );
  }

  if ( checkpoint != 0 && absolute_seqno - checkpoint > ( 1UL << 31 ) ) {
    absolute_seqno -= ( 1UL << 32 );
  }

  return absolute_seqno;
}

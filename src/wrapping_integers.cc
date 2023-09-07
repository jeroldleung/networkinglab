#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return zero_point + n;
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  constexpr uint64_t SL31 = 1UL << 31; // shift left 31 bits;
  constexpr uint64_t SL32 = 1UL << 32; // shift left 32 bits;

  const Wrap32 ckpt32 = wrap( checkpoint, zero_point );
  const uint64_t offset = raw_value_ - ckpt32.raw_value_;

  if ( offset <= SL31 || checkpoint + offset < SL32 ) {
    return checkpoint + offset;
  }

  return checkpoint + offset - SL32;
}

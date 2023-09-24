# Checkpoint 2 Writeup

- **TCPReceiver**: is the recipient of the TCP protocol. Its job is to receive the tcp sender message from the TCPSender, release data from the message, push data to the Reassembler, and reply a tcp receiver message for **acknowledgment** and **flow control** (using sliding window).
- **Wrap32**: is a type that represents a 32-bit unsigned integer, which starts at an arbitrary initial value and wraps back to zero when it reaches 2^32 - 1.

## Design of the Wrap32

Both sequence number and acknowledgment number in TCP header are 32-bit long. Suppose it just represent by a `uint32_t` variable, which means that a `uint32_t` sequence number or acknowlegment number can index 2^32 numbers of bytes. In a 100 gigabits/sec transmitting network, it takes only 1/3 second to reach 2^32 bytes, which means the sequence number will **overflow** if data size is bigger than 2^32 bytes.

For that reason, we need the 32-bit sequence number to **wrap around** for indexing the arbitrary long ByteStream.

## Program Structure

TCPReceiver `receive` message from the peer's sender, insert the message to the Reassembler and `send` a reply message to the peer's sender. There is one more thing to notices, 

- the **sequence number** start at random and contian the SYN flag and the FIN flag,
- the **absolute sequence number** start at zero and contian the SYN flag and the FIN flag,
- the **stream index** start at zero but not contian the SYN flag and the FIN flag.

The Wrap32 has a static `wrap` method that takes a absolute sequence number and an initial sequence number, wrap the absolute sequence number and return a Wrap32 object. The `unwrap` method take the initial sequence number and the checkpoint, unwrap the raw value which store in its Wrap32 object to the absolute sequence number that closest to the checkpoint, and return the absolute sequence number to the caller.

## Implementation Challenges

**Unwrap a sequence number to an absolute sequence number.** There are two candidates for the absolute sequence number, one is in front of the checkpoint, the other is behind the checkpoint. The key is to evaluate the offset that between the sequence number and the wrapped checkpoint. Suppose the absolute sequence number is in front of the checkpoint, which means that the offset is calculated by the sequence number minus the wrapped checkpoint. There are only two situations that the absolute sequence number is actually in front of the checkpoint:

- offset is less than and equal to 2^32 / 2 (front candidate is closer to the checkpoint)
- offset plus the wrapped checkpoint is less than 2^32 (candidate that behind the checkpoint is negative)

In those two cases, the absolute sequence number will be in front of the checkpoint and calculated by `checkpoint + offset`. Any other situations would be that the absolute sequence number is behind the checkpoint and calculated by `checkpoint - 2^32 + offset`.


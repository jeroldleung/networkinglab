# Checkpoint 1 Writeup

- **Reassembler**: is to reassemble data in sequence back into the original byte stream. The Reassembler share capacity with original byte stream, which means that is can be viewed as part of the original byte stream. Data possibly arrives out of order or overlapping, the Reassembler's job is to learn the next expected bytes and `insert` to the byte stream.

## Program Structure

-  I chose `list` for storing the discontinus data inside the Reassembler.
-  Bytes that beyond the stream's available capacity will be discarded at defensive checking.
-  Bytes are the next expected sequence will be pushed immediately to the stream.
-  Bytes that reach early will be inserted to the list in ascending order.
-  The overlapping substrings should also be detected and erased.
-  When earlier bytes have reached, then push the expected substrings that store in list to the stream.
-  Remember to close the stream when the last substring is reached and substrings in list are fully pushed to the stream.

## Implementation Challenges

**Detect and erase the overlapping substirngs.** The main logic of adding data (substrings) to the buffer in ascending order is according to the first index of the substring. There are two extra edge cases to handle, 

- the buffer is empty,
- the first index of current substring is the biggest one among the buffer.

In the first edge case, the substring just be `emplace_front` into the list while in the second edge case, the substring should be `emplace_back` into the list.

After adding the substring to the list, it need to handle the overlapping. It just need to compare to the previous substring in the buffer and erase the overlapped part because of the ascending order.


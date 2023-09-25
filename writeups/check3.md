# Checkpoint 3 Writeup

- **TCPSender**: is to read from a ByteStream which created and written to by some sender-side application, and turn the stream into a sequence of outgoing TCP segments.

## Responsibility of the TCPSender

- Keep track of the sliding window
- Fill the window if possible, by reading from the ByteStream, creating new TCP segments, and sending them. The sender should **keep sending segments** until either the window is full or the outbound ByteStream has nothing more to send.
- Keep track of which segments have been sent but not yet acknowledged by the receiver, which we call these "outstanding" segments.
- Re-send outstanding segments if enough time passes since they were sent, and they haven't been acknowledged yet.

## Program Structure

The TCPSender needs a helper class for recording the transmission time, which I call it `Timer`. The TCPSender is constructed with an given argument that tells it the initial value of the **retranssion timeout** (RTO). The Timer is responsible for recording the current RTO and time passage, and detecting whether the timer is expired or not.

The main methods of TCPSender are `push`, `receive` and `tick`. 

The `push` method generate as many TCP messages as possible. It keep track of the sequence numbers in flight, and make a comparation to the window size. It would have space to fill in the window if the sequence numbers in flight is smaller than the window size.

The `receive` method takes a argument `TCPReceiverMessage` which store the acknowledgment number and the window size. It updates the window size, removes the outstanding segment that has fully acknowledged according to the acknowledgment number that store in the message, set RTO to initial value, and reset the consecutive retransmissions number back to 0 (the TCPConnection will use this information to decide if the connection is hopeless, which means too many consecutive retransmissions in a row, and needs to be aborted).

The `tick` method need to retransmit an outstanding segment if the timer is expired. Don't forget to restart the timer and increment the consecutive retransmissions number after retransmitting. If the window size is nonzero, that means the network is bussy, we need to slow down retransmissions (double rto) avoid further gumming up the works. 

## Implementation Challenges

At first, I didn't know how to keep sending segments in `push` method, i.e., I didn't known how to set the while loop condition. Until I discussed with others, I found the key is to detect whether the window size is full or not. If the sequence numbers in flight is less than the window size, it means the window is not full, and keep generating TCPSenderMessages, otherwise, the window is full and jump out of the while loop. Translate it to pseudo code is 

```
while ( sequence numbers in flight < window size )
  keep generating TCPSenderMessages
```


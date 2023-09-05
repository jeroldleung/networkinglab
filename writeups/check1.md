Checkpoint 1 Writeup
====================

My name: [your name here]

My SUNet ID: [your sunetid here]

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This lab took me about [n] hours to do. I [did/did not] attend the lab session.

Program Structure and Design of the Reassembler:

1. `std::list` for internally buffer of the Reassembler.
2. Bytes that beyond the stream's available capacity will be discarded at defensive checking.
3. Bytes are the next expected sequence will be pushed immediately to the stream.
4. Bytes that reach early will be inserted to the list in ascending order.
5. The overlapping substrings should also be detected and erased at step 4.
6. When earlier bytes have reached, then push the expected substrings that store in list to the stream.
7. Remember to close the stream when the last substring is reached and substrings in list are fully pushed to the stream.

Implementation Challenges:
[Detect and erase the overlapping substirngs.]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

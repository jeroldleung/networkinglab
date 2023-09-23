# Checkpoint 0 Writeup

- **webget**: a simple network application using OS stream socket to fetch a web page.
- **ByteStream**: a abstraction of a reliable byte stream in communicating across the unreliable Internet.

## Program Structure

ByteStream is an object with Reader and Writer interfaces. Bytes can be written on the input side and can be read, in the same sequence, from the output side. The ByteStream has in-memory buffer and capacity for storing data, which we use `queue` for the buffer. The Writer `push` data to the buffer while the Reader `pop` data from the buffer.


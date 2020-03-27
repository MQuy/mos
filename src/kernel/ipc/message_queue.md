### Send

- get queue from hashmap
- copy buf from user space to kernel space (buf has to not contain any external pointers)
- check is there a receiver with the same requested type
  - if having, update message sender (id and thread's state to ready), buf and size -> remove it from queue
  - if not, create the message with sender, buf and size -> add it into queue
- if message type >= 0, call Receive with -type to wait to receive ack from a receiver

### Receive

- get queue from hashmap
- check is there a message with the same requested type
  - if having, remove it from queue
  - if not
    - create receiver with buf, size and type
    - add it to queue
    - sleep
    - wakeup (sender update receiver's thread state)
- if message type >= 0, send ack to sender with -type
- if buf != NULL, copy from receciver's buf to user buf

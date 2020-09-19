### Send

- get queue from hashmap
- copy buf from user space to kernel space (buf has to not contain any external pointers)
- if queue's messages is full
  - create the sender and add to queue's sender
  - sleep
- add message to queue's message
- if there is receiver
  - remove receiver from queue
  - wake up receiver's thread
- wakeup poll wait
- free that message

### Receive

- get queue from hashmap
- if queue is full -> get the first sender from queue's sender -> wake it up
- if there is a message in queue -> go to the last step
- if not
  - create and add receiver into queue
  - mark as waiting and sleep until there is a message on queue
  - remove receiver from queue
  - go to the last step
- copy to user space, remove message from queue and mark sender as ready

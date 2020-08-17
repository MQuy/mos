### Send

- get queue from hashmap
- copy buf from user space to kernel space (buf has to not contain any external pointers)
- create and add message into queue
- if there is receiver -> remove receiver from queue and mark as ready
- wait until that message has no link
- free that message

### Receive

- get queue from hashmap
- if there is a message in queue -> go to the last step
- if not
  - create and add receiver into queue
  - mark as waiting and sleep until there is a message on queue
  - remove receiver from queue
  - go to the last step
- copy to user space, remove message from queue and mark sender as ready

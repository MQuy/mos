[extern task_schedule]
[extern irq0]

[global irq_task_handler]
irq_task_handler:
  cli
  pusha

  push ds
  push es
  push fs
  push gs
  mov ax, 0x10
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  push esp
  cld
  call task_schedule
  add esp, 4

  pop gs
  pop fs
  pop es
  pop ds

  popa
  sti
  jmp irq0
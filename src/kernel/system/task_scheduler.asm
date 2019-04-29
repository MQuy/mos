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
  pop esp

  cmp eax, 0
  je irq_task_handler_end
  mov esp, eax
  jmp irq_task_handler_end

irq_task_handler_end:
  pop gs
  pop fs
  pop es
  pop ds

  sti
  popa
  jmp irq0
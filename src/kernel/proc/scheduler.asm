[global do_switch]
do_switch:
  cli
  pusha

  mov eax, [esp + (8 + 1) * 4] ; load current task's kernel stack address
  mov [eax], esp      ; save esp for current task's kernel stack

  mov eax, [esp + (8 + 2) * 4]     ; load next task's kernel stack to esp
  mov ebx, [esp + (8 + 3) * 4]     ; load next task's page directory
  mov esp, eax
  mov cr3, ebx     

  popa
  sti
  ret
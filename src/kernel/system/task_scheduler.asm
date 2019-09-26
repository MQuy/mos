[global do_switch]
do_switch:
  pusha

  mov [ebp + 8], esp      ; save esp for current task's kernel stack
  mov esp, [ebp + 12]     ; load next task's kernel stack to esp

  mov eax, [ebp + 16]     ; load next task's page directory
  mov cr3, eax     

  popa
  ret
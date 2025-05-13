#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //TODO();
  rtl_push(&cpu.eflags);

//PA4.3
  cpu.IF = 0;

  rtl_push(&cpu.cs);
  rtl_push(&ret_addr);

  decoding.jmp_eip = (vaddr_read(cpu.idtr.base + 8 * NO, 4) & 0xFFFF)|(vaddr_read(cpu.idtr.base + 4 + 8 * NO, 4) & 0xFFFF0000);
  decoding.is_jmp = 1;

}

void dev_raise_intr() {
  cpu.intr = 1;
}

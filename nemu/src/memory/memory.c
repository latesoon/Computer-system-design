#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

#define BEGIN(pte) (((uint32_t)pte) & (~((1 << 12) - 1)))
#define PDX(addr) ((((uint32_t)addr) >> 22) & ((1 << 10) - 1))
#define PTX(addr) ((((uint32_t)addr) >> 12) & ((1 << 10) - 1))
#define OFFSET(addr) (((uint32_t)addr) & ((1 << 12) - 1))

static bool use_paging(){
  return (cpu.cr0 & 0x1) && (cpu.cr0 & 0x8000);
}

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  if(is_mmio(addr) == -1)
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  else
    return mmio_read(addr,len,is_mmio(addr));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  if(is_mmio(addr) == -1)
    memcpy(guest_to_host(addr), &data, len);
  else
    mmio_write(addr,len,data,is_mmio(addr));
}

paddr_t page_translate(vaddr_t addr, bool write){
  PDE pde = (PDE)(paddr_read(cpu.cr3 + PDX(addr),4));
  assert(pde.present);

  PTE pte = (PTE)(paddr_read(BEGIN(pde.val) + PTX(addr),4));
  assert(pte.present);

  pde.accessed = 1;
  pte.accessed = 1;
  pte.dirty |= write;
  Log("VADDR:%x PADDR:%x",addr,BEGIN(pte.val) | OFFSET(addr));
  return BEGIN(pte.val) | OFFSET(addr);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  Log("Pageing use:%d\n",use_paging());
  if(!use_paging())
    return paddr_read(addr, len);

  paddr_t paddr = addr;
  if(BEGIN(addr) != BEGIN(addr+len-1))
    assert(0);
  else
    paddr = page_translate(addr, false);
  return paddr_read(paddr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  Log("Pageing use:%d\n",use_paging());
  if(!use_paging())
    return paddr_write(addr, len, data);

  paddr_t paddr = addr;
  if(BEGIN(addr) != BEGIN(addr+len-1))
    assert(0);
  else
    paddr = page_translate(addr, true);
  return paddr_write(paddr, len, data);
}

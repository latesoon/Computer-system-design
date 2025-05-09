#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

#define BEGIN(pte) (((uint32_t)pte) & ((1 << 12) - 1))


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
  return 1;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  paddr_t paddr = addr;
  if(BEGIN(addr) != BEGIN(addr+len-1)){
    assert(0);
  }
  else{
    //paddr = page_translate()
  }
  return paddr_read(paddr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  paddr_write(addr, len, data);
}

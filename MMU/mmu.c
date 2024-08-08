// ==============================================================================
/**
 * mmu.c
 */
// ==============================================================================



// ==============================================================================
// INCLUDES

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "mmu.h"
#include "vmsim.h"
// ==============================================================================



// ==============================================================================
// MACROS AND GLOBALS

/** 
 * The (real) address of the upper page table.  Initialized by a call
 * to `mmu_init()`.
 */
static vmsim_addr_t upper_pt_addr = 0;
// ==============================================================================



// ==============================================================================
void
mmu_init (vmsim_addr_t new_upper_pt_addr) {

  upper_pt_addr = new_upper_pt_addr;
  
}
// ==============================================================================



// ==============================================================================
vmsim_addr_t
mmu_translate (vmsim_addr_t sim_addr) {

  // REPLACE THIS WITH YOUR OWN, FULL IMPLEMENTATION OF THIS FUNCTION.
  // bits 31-22
  uint32_t upt_index = (sim_addr >> 22) & 0x3FF;
  // bits 21-12
  uint32_t lpt_index = (sim_addr >> 12) & 0x3FF;
  // bits 11-0
  uint32_t offset = sim_addr & 0xFFF;

  printf("\nSimulated address: 0x%x\n", sim_addr);
  printf("UPT Index: %u\n", upt_index);
  printf("LPT Index: %u\n", lpt_index);
  printf("Offset: %u\n", offset);

  uint32_t upt_entry; // create a variable for vmsim_read to read into 
  vmsim_read_real(&upt_entry, upper_pt_addr + upt_index * sizeof(uint32_t), sizeof(uint32_t));

  printf("UPT Entry Value: 0x%x\n", upt_entry);

  // case 1: no LPT to which the address's UPT entry leads to
  if (upt_entry == 0) {
	  printf("UPT entry leads to no LPT\n");
	  vmsim_map_fault(sim_addr); // map fault
	  printf("Mapping fault and trying translation again\n");
	  return mmu_translate(sim_addr); // try again after map fault
  }
  else {
	  uint32_t lpt_entry;
	  vmsim_read_real(&lpt_entry, upt_entry + lpt_index * sizeof(uint32_t), sizeof(uint32_t));

	  printf("LPT Entry value: 0x%x\n", lpt_entry);
	   
          // case 2: no real page  to which LPT's entry leads to
	  if (lpt_entry == 0) {
		  printf("LPT Entry leads to no real page");
		  vmsim_map_fault(sim_addr);
		  printf("Mapping fault and trying translation again\n");
		  return mmu_translate(sim_addr);
	  }
	  // case 3: translate simulated address to real address
	  else {
		  vmsim_addr_t real_addr = lpt_entry + offset;
		  printf("Translated real address: 0x%x\n", real_addr);
		  return real_addr;

         }
  }
}



// ==============================================================================

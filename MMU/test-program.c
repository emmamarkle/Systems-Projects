#include <stdio.h>
#include <stdlib.h>
#include "mmu.h"

int main() {
	size_t data_size = sizeof(unsigned char); // one byte of data
	vmsim_addr_t sim_addr = vmsim_alloc(data_size); // allocate simulated memory

	if (sim_addr == 0) { // allocated failure 
		printf("Error: failed to allocate simulated memory.");
		return 1;
	}

	unsigned char data_to_write = 0x77; // prepare single byte of data to write to sim memory
	printf("MAIN TEST: Simulated address before writing: 0x%x\n", sim_addr); // DEBUG

	vmsim_write(&data_to_write, sim_addr, data_size); // write data to simulated memory
	printf("MAIN TEST: Data written TO simulated memory: 0x%02X\n", data_to_write);

	unsigned char read_buffer[data_size];
	vmsim_read(read_buffer, sim_addr, data_size); // read data back from sim memory
	printf("MAIN TEST: Data read FROM simulated memory: 0x%02X\n", read_buffer[0]); // print data read from simulated memory



	vmsim_addr_t real_addr = mmu_translate(sim_addr);
	unsigned char data_from_real;
	vmsim_read_real(&data_from_real, real_addr, sizeof(unsigned char));
	printf("MAIN TEST: Data from translated real address: 0x%02X\n", data_from_real);



	vmsim_free(sim_addr); // free simulated memory 

	return 0;
}

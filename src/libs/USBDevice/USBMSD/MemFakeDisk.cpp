/*
 * MemFakeDisk.cpp
 *
 *  Created on: Apr 9, 2015
 *      Author: eai
 */

#include "MemFakeDisk.h"
#include "diskio.h"
#include "Kernel.h"
#include "StreamOutput.h"
#include "StreamOutputPool.h"
#include "malloc.h"

// Include fake disk data generated using build_fake_partition.sh
#include "fake_disk_data.h"

MemFakeDisk::MemFakeDisk() {
}

int MemFakeDisk::disk_initialize() {
	return 0;
};

char get_data(uint32_t i) {
	uint32_t addr_page = (i/16)*16;

	for (uint32_t j = 0; j < sizeof(addrs)/sizeof(int32_t); j++) {
		if (addr_page == addrs[j]){
			return data[j][i % 16];
		}
	}
	return 0;
}

int MemFakeDisk::disk_read(char * data, uint32_t block) {
	if (block < disk_sectors()) {
		for (uint32_t i = 0; i < 512; i++){
			data[i] = get_data(block*512 + i);
		}
		return RES_OK;
	} else {
		return RES_PARERR;
	}
}
int MemFakeDisk::disk_write(const char * data, uint32_t block) {
	return RES_WRPRT;
};

uint64_t MemFakeDisk::disk_size() {
	return disk_sectors() << 9;
};
uint32_t MemFakeDisk::disk_sectors() {
	return DISK_NUM_SECTORS;
};

//MemFakeDisk::~MemFakeDisk() {
//}


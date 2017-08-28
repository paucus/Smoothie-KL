/*
 * MemFakeDisk.h
 *
 *  Created on: Apr 9, 2015
 *      Author: eai
 */

#ifndef MemFakeDisk_H_
#define MemFakeDisk_H_

#include "disk.h"

class MemFakeDisk: public MSD_Disk {
public:
	MemFakeDisk();
//	virtual ~MemFakeDisk();

	virtual int disk_read(char * data, uint32_t block);

	virtual int disk_write(const char * data, uint32_t block);
	virtual int disk_initialize();
	virtual uint32_t disk_sectors();
	virtual uint64_t disk_size();
	virtual uint32_t disk_blocksize() { return (1<<9);};
	virtual int disk_status() { return 0; };
	virtual bool disk_canDMA() { return 0; };
	virtual int disk_sync() { return 0; };
	virtual bool busy() {return false; };
};

#endif /* MemFakeDisk_H_ */

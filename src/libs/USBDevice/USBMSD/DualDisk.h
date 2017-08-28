/*
 * DualDisk.h
 *
 *  Created on: Apr 10, 2015
 *      Author: eai
 */

#ifndef DUALDISK_H_
#define DUALDISK_H_

#include "disk.h"

class DualDisk: public MSD_Disk {
public:
	DualDisk(MSD_Disk* d1, MSD_Disk* d2);
//	virtual ~DualDisk();

	virtual int disk_initialize();

	virtual int disk_read(char * data, uint32_t block) { return curr_disk->disk_read(data, block); };
	virtual int disk_write(const char * data, uint32_t block) { return curr_disk->disk_write(data, block); };
	virtual uint32_t disk_sectors() { return curr_disk->disk_sectors(); };
	virtual uint64_t disk_size() { return curr_disk->disk_size(); };
	virtual uint32_t disk_blocksize() { return curr_disk->disk_blocksize(); };
	virtual int disk_status() { return curr_disk->disk_status(); };
	virtual bool disk_canDMA() { return curr_disk->disk_canDMA(); };
	virtual int disk_sync() { return curr_disk->disk_sync(); };
	virtual bool busy() { return curr_disk->busy(); };
private:
	MSD_Disk* curr_disk;
	MSD_Disk* d1;
	MSD_Disk* d2;
};


#endif /* DUALDISK_H_ */

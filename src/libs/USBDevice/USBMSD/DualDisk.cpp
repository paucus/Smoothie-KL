/*
 * DualDisk.cpp
 *
 *  Created on: Apr 10, 2015
 *      Author: eai
 */

#include "DualDisk.h"

DualDisk::DualDisk(MSD_Disk* d1, MSD_Disk* d2) {
	this->d1 = d1;
	this->d2 = d2;
	this->curr_disk = d1;
}

int DualDisk::disk_initialize(){
	int v = d1->disk_initialize();
	curr_disk = d1;
	if (v != 0) {
		v = d2->disk_initialize();
		curr_disk = d2;
	}
	return v;
}

//DualDisk::~DualDisk() {
//}


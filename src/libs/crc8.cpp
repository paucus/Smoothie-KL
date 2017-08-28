/*
 * crc8.cpp
 *
 *  Created on: Mar 11, 2016
 *      Author: eai
 */
#include "crc8.h"
#include <stdio.h>

// Source: https://en.wikipedia.org/wiki/Cyclic_redundancy_check
#define POLYNOME 0x07

uint8_t crc8_byte(uint8_t crc_so_far, uint8_t data_p) {
	unsigned int i;
	unsigned int data = data_p ^ crc_so_far;

	for (i = 0; i < 8; i++) {
		if ((data & 0x80) != 0) {
			data <<= 1;
			data ^= POLYNOME;
		} else {
			data <<= 1;
		}
	}
	return data;
}

uint8_t crc8(uint8_t crc_so_far, const uint8_t* data, unsigned int len) {
	for (unsigned int i = 0; i < len; i++) {
		crc_so_far = crc8_byte(crc_so_far, data[i]);
	}
	return crc_so_far;
}

int crc8_file(const char* fn) {
	uint8_t crc = 0;
	FILE* f = fopen(fn, "rb");
	if (!f) return -1;
	while (!feof(f)) {
		int c = fgetc(f);
		if (c >= 0) {
			crc = crc8_byte(crc, c);
		}
	}
	return crc;
}

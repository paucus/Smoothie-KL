/*
 * crc8.h
 *
 *  Created on: Mar 11, 2016
 *      Author: eai
 */

#ifndef CRC8_H_
#define CRC8_H_

#include <inttypes.h>

// Calculate crc8 of one byte using the previous crc.
uint8_t crc8_byte(uint8_t crc_so_far, uint8_t data_p);
// Calculate the crc8 of a piece of memory assuming initial crc=0.
uint8_t crc8(const uint8_t* data, unsigned int len);
// Calculate the crc8 of a whole file assuming initial crc=0.
// If something fails, returns -1.
int crc8_file(const char* f);


#endif /* CRC8_H_ */

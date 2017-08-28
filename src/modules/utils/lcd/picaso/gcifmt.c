/*
 * gcifmt.c
 *
 *  Created on: Dec 14, 2015
 *      Author: eai
 */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "gcifmt.h"

// Returns 0 if nothing failed.
int parse_gci_header(gcifmt_t * gci_data, const void* data) {
	const unsigned char* ucdata = data;
	// Parse bytes. Numbers are 16 bits little-endian.
	gci_data->width = ((unsigned int)ucdata[0]) | ((unsigned int)ucdata[1]) << 8;
	gci_data->height = ((unsigned int)ucdata[2]) | ((unsigned int)ucdata[3]) << 8;
	gci_data->colors = ucdata[4];
	gci_data->fps = ucdata[5];
	return 0;
}

#ifdef __cplusplus
}
#endif // __cplusplus

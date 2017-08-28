/*
 * gcifmt.h
 *
 *  Created on: Dec 14, 2015
 *      Author: eai
 */

#ifndef GCIFMT_H_
#define GCIFMT_H_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gcifmt {
	uint16_t width;
	uint16_t height;
	uint8_t colors;
	uint8_t fps;
} gcifmt_t;

// Parses the first 6 bytes of the data pointer and returns the gci parsed data through the
// gci_data parameter. Returns 0 if nothing failed.
int parse_gci_header(gcifmt_t * gci_data, const void* data);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* GCIFMT_H_ */

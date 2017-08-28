/*
 * kltar.c
 *
 *  Created on: Dec 11, 2015
 *      Author: eai
 */

#include "kltar.h"
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


#pragma pack(push, 1)
struct posix_header_extract
{                               /* byte offset */
  char name[100];               /*   0 */
  char ignore1[24];             /* 100 */
  char size[12];                /* 124 */
  char ignore2[20];             /* 136 */
  char typeflag;                /* 156 */
};
#pragma pack(pop)

#define round_to_sector(x) (((x + 511)/512) * 512)

static int read_extract(kltar_t* tar, const char* search_fname) {
	struct posix_header_extract header_extract;
	int read_len = fread(&header_extract, sizeof(header_extract), 1, tar->f);
	if (read_len != 1) {
		return KLTAR_FAILED_TO_READ_HEADER;
	}
	size_t name_len = strlen(header_extract.name);
	if (name_len == 0) {
		// Candidate to be an end of file
		// In order to be one, there must be two sectors with 0's
		char* ptr = (char*)(void*)&(header_extract);
		unsigned int i;
		for (i = 0; i < sizeof(struct posix_header_extract) && *ptr == '\0'; i++, ptr++){
			// loop
		}

		if (i == sizeof(struct posix_header_extract)) {
			// Candidate to be an end of file. At the risk of not being compliant with the
			// standard, let's skip this and verify the next sector.
			if (fseek(tar->f, 512 - sizeof(struct posix_header_extract), SEEK_CUR) != 0) {
				return KLTAR_FAILED_TO_READ_HEADER;
			}

			// read next sector and test the same
			int read_len = fread(&header_extract, sizeof(header_extract), 1, tar->f);
			if (read_len != 1) {
				return KLTAR_FAILED_TO_READ_HEADER;
			}

			// Read name, but don't check the len, as we will anyway check byte by byte if it's zero
			name_len = strlen(header_extract.name);
			ptr = (char*)(void*)&(header_extract);

			for (i = 0; i < sizeof(struct posix_header_extract) && *ptr == '\0'; i++, ptr++){
				// loop
			}

			if (i == sizeof(struct posix_header_extract)) {
				// All zeroes. Let's take it as an end of tar file.
				return KLTAR_EOF;
			}
		}
	}

	if (name_len > KLTAR_NAME_MAX_LEN) {
		// Invalid file name
		strncpy(tar->name, header_extract.name, KLTAR_NAME_MAX_LEN);
		tar->name[KLTAR_NAME_MAX_LEN] = '\0';
	} else {
		strcpy(tar->name, header_extract.name);
	}
	tar->size=strtol(header_extract.size, NULL, 8);
	tar->read=0;
	// Skip the rest of the file header
	if (fseek(tar->f, 512 - sizeof(struct posix_header_extract), SEEK_CUR) != 0) {
		return KLTAR_FAILED_TO_READ_HEADER;
	}
	if (search_fname != NULL && strcmp(search_fname, header_extract.name) == 0) {
		// Special case. When looking for a file in particular, compare the whole name.
		return KLTAR_FOUND;
	} else {
		return KLTAR_OK;
	}
}

int kltar_open(kltar_t* dest, const char* tar_fname) {
	dest->f = fopen(tar_fname, "rb");
	if (!dest->f) {
		return KLTAR_CANNOT_OPEN;
	}
	int res = read_extract(dest, NULL);
	if (res != KLTAR_OK) {
		fclose(dest->f);
		dest->f = NULL;
	}
	return res;
}

int kltar_next(kltar_t* tar) {
	// We know we are at the end of a file header. We must skip the rest of the file. Ignore what has been read so far.
	if (fseek(tar->f, round_to_sector(tar->size) - tar->read, SEEK_CUR) != 0) {
		// there should be at least this len.
		return KLTAR_FAILED_TO_READ;
	}

	return read_extract(tar, NULL);
}

int kltar_get_cur_name(kltar_t* tar, char** name) {
	*name = tar->name;
	return KLTAR_OK;
}
int kltar_get_cur_size(kltar_t* tar, unsigned long* size) {
	*size = tar->size;
	return KLTAR_OK;
}
int kltar_close(kltar_t* tar) {
	if (tar->f)
		fclose(tar->f);
	tar->f = NULL;
	return KLTAR_OK;
}

int kltar_cur_read(void* dest, size_t len, kltar_t* tar) {
	if (tar->size < tar->read + len) {
		// limit the length up to the size
		len = tar->size - tar->read;
	}

	int r = fread(dest, len, 1, tar->f);
	if (r != 1) {
		return KLTAR_FAILED_TO_READ;
	}

	tar->read += len;
	return len;
}

int kltar_cur_feof(kltar_t* tar) {
	return tar->size == tar->read;
}



// For internal usage
static int kltar_find_next(kltar_t* tar, const char* fname) {
	// We know we are at the end of a file header. We must skip the rest of the file. Ignore what has been read so far.
	if (fseek(tar->f, round_to_sector(tar->size) - tar->read, SEEK_CUR) != 0) {
		// there should be at least this len.
		return KLTAR_FAILED_TO_READ;
	}

	return read_extract(tar, fname);
}
int kltar_find_open(kltar_t* dest, const char* tar_fname, const char* fname) {
	dest->f = fopen(tar_fname, "rb");
	if (!dest->f) {
		return KLTAR_CANNOT_OPEN;
	}
	int res = read_extract(dest, fname);
	if (res == KLTAR_FOUND) {
		// First file is a match
		return KLTAR_OK;
	} else if (res != KLTAR_OK) {
		fclose(dest->f);
		dest->f = NULL;
		return res;
	}

	// If we reached this point, it means res == KLTAR_OK.
	// This means no error occurred, and the first file is not the one we are looking for.
	while ((res = kltar_find_next(dest, fname)) == KLTAR_OK) {
		// loop
	}

	// res == KLTAR_FOUND or res == error
	if (res == KLTAR_FOUND) {
		// file found. return control.
		return KLTAR_OK;
	} else {
		// something failed
		fclose(dest->f);
		dest->f = NULL;
		return res;
	}
}

#ifdef __cplusplus
}
#endif

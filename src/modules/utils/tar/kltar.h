/*
 * kltar.h
 *
 *  Created on: Dec 11, 2015
 *      Author: eai
 */

#ifndef KLTAR_H_
#define KLTAR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

// This library has been designed with memory constraints in mind.
// For this reason, many features are ignored in order to do just what we need to do: extract some
// files from a tar file.

typedef enum kltar_err {
	KLTAR_OK                    =  0,
	KLTAR_CANNOT_OPEN           = -1,
	KLTAR_FAILED_TO_READ        = -2,
	KLTAR_FAILED_TO_READ_HEADER = -3,
	KLTAR_EOF                   = -4,
	KLTAR_FOUND                 = -5, // for internal use only
} kltar_err;

// We are interested in keeping memory usage low, so, we can limit the length.
#define KLTAR_NAME_MAX_LEN 19

typedef enum kltar_entry_t {kltar_file, kltar_dir, kltar_other} kltar_entry_t;
typedef struct kltar {
	FILE* f;
	kltar_entry_t type;
	// If name starts with '\0', it means it's too long to be stored here.
	char name[KLTAR_NAME_MAX_LEN+1];
	unsigned long size;
	unsigned long read;
} kltar_t;

// Functions for listing mostly

// Opens the tar file and leaves the cursor at the first position.
int kltar_open(kltar_t* dest, const char* tar_fname);
// Closes the tar file.
int kltar_close(kltar_t* tar);
// Moves the cursor to the next file. If the end of tar file is reached, it returns KLTAR_EOF.
int kltar_next(kltar_t* tar);
// Returns the current file name through the name parameter. If the file is longer than
// KLTAR_NAME_MAX_LEN characters, it returns as much as it can store.
// If something failed, it returns an error code. Otherwise, it returns KLTAR_OK.
int kltar_get_cur_name(kltar_t* tar, char** name);
// Returns the current file size through the size parameter.
// If something failed, it returns an error code. Otherwise, it returns KLTAR_OK.
int kltar_get_cur_size(kltar_t* tar, unsigned long* size);
// Reads up to len bytes from the file where the cursor is located at. If nothing failed, it
// returns the number of bytes actually read. If something failed, it returns the error code.
// "0" means in this case that 0 bytes were read.
// To know when the end of file was reached, use kltar_cur_feof.
int kltar_cur_read(void* dest, size_t len, kltar_t* tar);
// Returns 1 if the end of file is reached. Otherwise, it return 0.
// In this case, the end of file refers to the end of the archived file, not the tar file itself.
// This function cannot fail, so, it won't return error codes.
int kltar_cur_feof(kltar_t* tar);
// Shortcut to find a specific file.
// It searches for the specified file and leaves the cursor in that position.
// If it fails it returns the error. The tar file will be automatically closed in that case, so,
// there is no need to call kltar_close. If it worked, it returns KLTAR_OK. In this case the file
// will have to be closed.
int kltar_find_open(kltar_t* dest, const char* tar_fname, const char* fname);

#ifdef __cplusplus
}
#endif

#endif /* KLTAR_H_ */

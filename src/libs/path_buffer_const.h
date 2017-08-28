/*
 * stdio_const.h
 *
 *  Created on: Jul 7, 2015
 *      Author: eai
 */

#ifndef PATH_BUFFER_CONST_H_
#define PATH_BUFFER_CONST_H_

#include <string>

// This constant is the size of path buffers in FATFileSystem.cpp.
#define PATH_BUFF_SIZE 100

// This constant considers the extra space needed to store the "X:/" at the beginning and the null character at the end.
#define MAX_PATH_LEN (PATH_BUFF_SIZE-4)

// This method lets you calculate whether some file name lengths are valid.
inline bool path_is_too_long(std::string& folder, const char* filename){
	return folder.size() + 1 /* the / char */ + strlen(filename) > MAX_PATH_LEN;
}


#endif /* STDIO_CONST_H_ */

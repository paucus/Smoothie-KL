/*
 * chunk.h
 *
 *  Created on: Jul 20, 2015
 *      Author: eai
 */

#ifndef CHUNK_H_
#define CHUNK_H_

typedef struct {
	unsigned long start;
	unsigned int length;
} chunk_t;

class chunk_iterator {
public:
	virtual ~chunk_iterator() {};
	virtual chunk_t operator *() = 0;
	virtual chunk_iterator& operator ++() = 0;
//	virtual chunk_iterator operator ++(int);
//	virtual bool operator == (const chunk_iterator& it);
	virtual bool has_more() = 0;
};

#endif /* CHUNK_H_ */

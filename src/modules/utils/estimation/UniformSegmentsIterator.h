/*
 * UniformSegmentsIterator.h
 *
 *  Created on: Jul 20, 2015
 *      Author: eai
 */

#ifndef UNIFORMSEGMENTSITERATOR_H_
#define UNIFORMSEGMENTSITERATOR_H_

#include "chunk.h"
#include <iterator>

class UniformSegmentsIterator : public chunk_iterator {
public:
        UniformSegmentsIterator(const UniformSegmentsIterator& it);
        UniformSegmentsIterator(unsigned long total_size, unsigned int number_of_segments, unsigned long chunks_length);
        virtual ~UniformSegmentsIterator();

        virtual chunk_t operator *();
        virtual UniformSegmentsIterator& operator ++();
        virtual UniformSegmentsIterator operator ++(int);
        virtual bool operator == (const UniformSegmentsIterator& it);
        virtual bool operator != (const UniformSegmentsIterator& it);

        virtual const UniformSegmentsIterator end();
        virtual bool has_more();

protected:
        // Internal constructor to set a specific i value
        UniformSegmentsIterator(unsigned long total_size, unsigned int number_of_segments, unsigned long chunks_length, unsigned long i);

        unsigned long total_size;
        unsigned int number_of_segments;
        unsigned long chunks_length;
        unsigned long i;
};

#endif /* UNIFORMSEGMENTSITERATOR_H_ */

/*
 * UniformSegmentsIterator.cpp
 *
 *  Created on: Jul 20, 2015
 *      Author: eai
 */

#include "UniformSegmentsIterator.h"

UniformSegmentsIterator::UniformSegmentsIterator(const UniformSegmentsIterator& it) : UniformSegmentsIterator(it.total_size, it.number_of_segments, it.chunks_length, it.i) {}
UniformSegmentsIterator::UniformSegmentsIterator(unsigned long total_size, unsigned int number_of_segments, unsigned long chunks_length, unsigned long i) : total_size(total_size), number_of_segments(number_of_segments), chunks_length(chunks_length), i(i) {}
UniformSegmentsIterator::UniformSegmentsIterator(unsigned long total_size, unsigned int number_of_segments, unsigned long chunks_length) : UniformSegmentsIterator(total_size, number_of_segments, chunks_length, 0) {}

UniformSegmentsIterator::~UniformSegmentsIterator() {
}

chunk_t UniformSegmentsIterator::operator *(){
    chunk_t c;
    // We know that i < number_of_segments, so we don't need to consider that case.
    // If total_size is big we must be careful when multiplying because we could overflow.
    if (total_size < (~((unsigned long)0)) / number_of_segments) {
        c.start = (i*total_size)/number_of_segments;
    } else {
        // a*B/c all integers, a < c << B: Max(abs(a*floor(B/c)-floor(a*B/c))) = c - 2, in this case
        // number_of_segments - 2
        c.start = i*(total_size/number_of_segments);
    }
    c.length = chunks_length;
    return c;
}
UniformSegmentsIterator& UniformSegmentsIterator::operator ++(){
    i++;
    return *this;
}
UniformSegmentsIterator UniformSegmentsIterator::operator ++(int){
    UniformSegmentsIterator tmp(*this);
    operator++();
    return tmp;
}

const UniformSegmentsIterator UniformSegmentsIterator::end() {
    return UniformSegmentsIterator(total_size, number_of_segments, chunks_length, number_of_segments);
}
bool UniformSegmentsIterator::has_more() {
    return i < number_of_segments;
}

bool UniformSegmentsIterator::operator != (const UniformSegmentsIterator& it) {
    return this->i != it.i;
}
bool UniformSegmentsIterator::operator == (const UniformSegmentsIterator& it) {
    return this->i == it.i;
}


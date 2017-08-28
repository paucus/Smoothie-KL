/*
 * TimeArray.h
 *
 *  Created on: Jul 20, 2015
 *      Author: eai
 */

#ifndef TIMEARRAY_H_
#define TIMEARRAY_H_

#define NUMBER_OF_SAMPLES 10
#define INDEX_TYPE unsigned char

class TimeArray {
public:
	TimeArray();
	virtual ~TimeArray();
	const float* get_time_array() const { return time_samples; };

	float operator[](int i) const { return time_samples[i]; };
	float& operator[](int i) {return time_samples[i]; };

	unsigned int get_time_array_number_of_elements() const { return NUMBER_OF_SAMPLES+1; };
	// the time array that is the result of the execution.
	// the first position is always 0 and the last one always the total time.
	// each segment represent a uniform amount of bytes processed.
	float time_samples[NUMBER_OF_SAMPLES+1];
};

#endif /* TIMEARRAY_H_ */

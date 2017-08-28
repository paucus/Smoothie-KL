/*
 * PrecalculationEstimation.h
 *
 *  Created on: Jul 15, 2015
 *      Author: eai
 */

#ifndef PRECALCULATIONESTIMATION_H_
#define PRECALCULATIONESTIMATION_H_

#include "RemainingTimeEstimation.h"
#include "TimeArray.h"

// reestimate every 30 seconds
#define ESTIMATION_INTERVAL 30

class PrecalculationEstimation : public RemainingTimeEstimation {
public:
	PrecalculationEstimation();
	virtual ~PrecalculationEstimation();

	unsigned long calculate_remaining_time(float extrusion_multiplier);
	void preprocess_file(const char* gcode);
protected:
	TimeArray time_array;

	// intermediate variables
	bool analysis_was_performed;
	// This attribute holds the calculated remaining time considering with these considerations:
	// It is agnostic of the speed multiplier. Any adjustment is made "using this value", but NOT in this value.
	// It is the remaining time at the time stored in current_estimation_timestamp.
	unsigned long current_remaining_time_est;
	unsigned long current_estimation_timestamp;
	unsigned long total_file_size;
};

#endif /* PRECALCULATIONESTIMATION_H_ */

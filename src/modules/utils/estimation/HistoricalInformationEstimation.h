/*
 * HistoricalInformationEstimation.h
 *
 *  Created on: Jul 15, 2015
 *      Author: eai
 */

#ifndef HISTORICALINFORMATIONESTIMATION_H_
#define HISTORICALINFORMATIONESTIMATION_H_

#include "RemainingTimeEstimation.h"

class HistoricalInformationEstimation: public RemainingTimeEstimation {
public:
	HistoricalInformationEstimation();
	virtual ~HistoricalInformationEstimation();

	unsigned long calculate_remaining_time(float extrusion_multiplier);
};

#endif /* HISTORICALINFORMATIONESTIMATION_H_ */

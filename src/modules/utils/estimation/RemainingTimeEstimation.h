/*
 * RemainingTimeEstimation.h
 *
 *  Created on: Jul 15, 2015
 *      Author: eai
 */

#ifndef REMAININGTIMEESTIMATION_H_
#define REMAININGTIMEESTIMATION_H_

#include "Kernel.h"
#include "checksumm.h"
#include "PublicData.h"
using std::string;
#include "PlayerPublicAccess.h"

#define INFINITE_TIME (~((unsigned long)0))

class RemainingTimeEstimation {
public:
	RemainingTimeEstimation();
	virtual ~RemainingTimeEstimation();

	virtual void preprocess_file(const char* gcode) {};
	virtual unsigned long calculate_remaining_time(float extrusion_multiplier) = 0;
};

class NullEstimation : public RemainingTimeEstimation {
	void preprocess_file(const char* gcode) {};
	unsigned long calculate_remaining_time(float extrusion_multiplier) { return 0; };
};

inline detailed_progress get_sd_card_playing_progress(){
	void* result;
	if (!PublicData::get_value(player_checksum, get_detailed_progress_checksum, &result)) {
		detailed_progress p;
		p.effective_secs = 0;
		p.file_size = 0;
		p.played_bytes = 0;
		return p;
	} else {
		return *(detailed_progress*)result;
	}
}

#endif /* REMAININGTIMEESTIMATION_H_ */

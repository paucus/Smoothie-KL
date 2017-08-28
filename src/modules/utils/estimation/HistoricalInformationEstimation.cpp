/*
 * HistoricalInformationEstimation.cpp
 *
 *  Created on: Jul 15, 2015
 *      Author: eai
 */

#include "HistoricalInformationEstimation.h"

HistoricalInformationEstimation::HistoricalInformationEstimation() {
}

HistoricalInformationEstimation::~HistoricalInformationEstimation() {
}


unsigned long HistoricalInformationEstimation::calculate_remaining_time(float extrusion_multiplier){
	detailed_progress p = get_sd_card_playing_progress();
	if (p.effective_secs == 0) {
		return 0;
	}
    unsigned long bytespersec = p.played_bytes / p.effective_secs;	// TODO Consider the division by 0
    if(bytespersec > 0) {
        return (unsigned long)(p.file_size - p.played_bytes) / bytespersec;
    } else {
        // We can't calculate. Set "infinite" so that **:**:** is displayed.
        return INFINITE_TIME;
    }
}

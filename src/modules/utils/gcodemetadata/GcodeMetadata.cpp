/*
 * GcodeMetadata.cpp
 *
 *  Created on: Feb 1, 2016
 *      Author: eai
 */

#include "GcodeMetadata.h"
#include "SerialMessage.h"
#include "SlicerCommentsGcodeAnalyzer.h"
#include "StreamOutput.h"
#include "utils.h"
using namespace std;
#include <string>

GcodeMetadata::GcodeMetadata() {
}

GcodeMetadata::~GcodeMetadata() {
}

void GcodeMetadata::on_module_loaded() {
	this->register_for_event(ON_CONSOLE_LINE_RECEIVED);
}

void GcodeMetadata::on_console_line_received(void * args) {
	SerialMessage* msg = static_cast<SerialMessage*>(args);

	char first_char = msg->message[0];
	if(strchr(";( \n\rGMTN", first_char) != NULL) return;

	string possible_command = msg->message;

	string cmd = shift_parameter(possible_command);

	if (strncasecmp(cmd.c_str(), "help", 4) == 0) {
		msg->stream->printf("info <file> - prints gcode comments metadata");
	} else if (strncasecmp(cmd.c_str(), "info", 4) == 0) {
		string file = absolute_from_relative(possible_command);

		SlicingInformation info;
		SlicerCommentsGcodeAnalyzer analyzer;
		if (analyzer.analyze(file.c_str(), &info) == ANALYZE_OK) {
			msg->stream->printf("Slicer: %s\n", slicer_name_to_string(info.slicer));
			if (info.layer_height.is_set) {
				msg->stream->printf("Layer Height: %d microns (%.2f mms)\n", info.layer_height.val, ((float)info.layer_height.val)/1000.0 );
			} else {
				msg->stream->printf("Layer Height: (NOT SET)\n");
			}
			if (info.print_time.is_set) {
				char time_str[] = "00:00:00";
				long hs; int mins, secs;
				convert_to_time_units(info.print_time.val, &hs, &mins, &secs);
				format_time(time_str, hs, mins, secs);
				msg->stream->printf("Print time: %s\n", time_str);
			} else {
				msg->stream->printf("Print time: (NOT SET)\n");
			}
			if (info.bed_temp.is_set) {
				msg->stream->printf("Bed temp: %d C\n", info.bed_temp.val);
			} else {
				msg->stream->printf("Bed temp: (NOT SET)\n");
			}
			if (info.extruder_temp.is_set) {
				msg->stream->printf("Extruder temp: %d C\n", info.extruder_temp.val);
			} else {
				msg->stream->printf("Extruder temp: (NOT SET)\n");
			}

			msg->stream->printf("Filament used: ");
			if (!info.filament_used_grams.is_set && !info.filament_used_meters.is_set && !info.filament_used_vol_cm3.is_set) {
				msg->stream->printf("(NOT SET)\n");
			} else {
				if (info.filament_used_grams.is_set) {
					msg->stream->printf("%.4f gr ", info.filament_used_grams.val);
				}
				if (info.filament_used_meters.is_set) {
					msg->stream->printf("%.4f mts ", info.filament_used_meters.val);
				}
				if (info.filament_used_vol_cm3.is_set) {
					msg->stream->printf("%.4f cm^3 ", info.filament_used_vol_cm3.val);
				}
				msg->stream->printf("\n");
			}
		} else {
			msg->stream->printf("There was an error processing file %s.\n", file.c_str());
		}
	}
}

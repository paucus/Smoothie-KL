#ifndef GCODE_UTILS
#define GCODE_UTILS

#include "Kernel.h"
#include "Gcode.h"
#include <stdio.h>
#include <stdarg.h>

#define SEND_GCODE_V_BUFF_SIZE 50
void send_gcode(const char* msg, StreamOutput* stream);
void send_gcode_v(const char* msgfmt, StreamOutput* stream, ...);
void send_robot_gcode_wait(bool relative, const char* msg, StreamOutput* stream);
void send_robot_gcode_wait_v(bool relative, const char* msgfmt, StreamOutput* stream, ...);
void send_gcode_vd(const char* msgfmt, int buffsize, StreamOutput* stream, ...);
void send_all_gcodes(char** gcodes, StreamOutput* stream);
void send_all_gcodes(const char** gcodes, StreamOutput* stream);
void extract_gcode_from_string(char** & dest, const char* gcode_lines);
bool is_playing_sd_card();

#endif // GCODE_UTILS

#include "GcodeUtils.h"
#include <string.h>
#include "modules/utils/player/PlayerPublicAccess.h"
#include "PublicData.h"
#include "checksumm.h"
#include "Conveyor.h"
#include "Robot.h"

void send_gcode(const char* msg, StreamOutput* stream) {
    Gcode *gcode = new Gcode(msg, stream);
    THEKERNEL->call_event(ON_GCODE_RECEIVED, gcode );
    delete gcode;
}
void send_gcode_v(const char* msgfmt, StreamOutput* stream, ...) {
    char buff[SEND_GCODE_V_BUFF_SIZE];
    va_list argptr;
    va_start(argptr, stream);
    vsnprintf(buff, SEND_GCODE_V_BUFF_SIZE - 1, msgfmt, argptr);
    va_end(argptr);
    send_gcode(buff, stream);
}
void send_gcode_vd(const char* msgfmt, int buffsize, StreamOutput* stream, ...) {
	char* buff = new char[buffsize];
	va_list argptr;
	va_start(argptr, stream);
	vsnprintf(buff, buffsize, msgfmt, argptr);
	va_end(argptr);
	send_gcode(buff, stream);
	delete[] buff;
}

void send_robot_gcode_wait(bool relative, const char* msg, StreamOutput* stream) {
    THEKERNEL->conveyor->wait_for_empty_queue();
    Gcode gcode(msg, stream);
    bool oldmode= THEKERNEL->robot->absolute_mode;
    THEKERNEL->robot->absolute_mode= !relative;
    THEKERNEL->robot->on_gcode_received(&gcode); // send to robot directly
    THEKERNEL->conveyor->wait_for_empty_queue();
    THEKERNEL->robot->absolute_mode= oldmode; // restore mode
}
void send_robot_gcode_wait_v(bool relative, const char* msgfmt, StreamOutput* stream, ...){
	char buff[SEND_GCODE_V_BUFF_SIZE];
	va_list argptr;
	va_start(argptr, stream);
	vsnprintf(buff, SEND_GCODE_V_BUFF_SIZE - 1, msgfmt, argptr);
	va_end(argptr);
	send_robot_gcode_wait(relative, buff, stream);
}



void send_all_gcodes(char** gcodes, StreamOutput* stream) {
    for ( ; *gcodes; gcodes++) {
        send_gcode((const char*)*gcodes, stream);
    }
}
void send_all_gcodes(const char** gcodes, StreamOutput* stream) {
    for ( ; *gcodes; gcodes++) {
        send_gcode(*gcodes, stream);
    }
}

static unsigned int count_commands(const char* gcode_lines) {
    const char* tmp = gcode_lines;
    const char* last_tmp = tmp;
    unsigned int count = 0;
    do {
        if (*tmp == '\r' || *tmp == '\n' || *tmp == ';' || *tmp == '\0') {
            if (tmp-last_tmp > 1)
                count++;

            if (*tmp == '\0') {
                return count;
            } else {
                last_tmp = tmp;
                last_tmp++;
            }
        }
        tmp++;
    } while (true);
}

void extract_gcode_from_string(char** & dest, const char* gcode_lines) {
    const char* tmp = gcode_lines;
    const char* last_tmp = tmp;

    unsigned int number_of_commands = count_commands(gcode_lines);
    dest = new char*[number_of_commands + 1];
    dest[number_of_commands] = nullptr;

    unsigned int i = 0;
    do {
        if (*tmp == '\r' || *tmp == '\n' || *tmp == ';' || *tmp == '\0') {
            if (tmp-last_tmp > 1) {
                dest[i] = new char[tmp-last_tmp+1];
                strncpy(dest[i], last_tmp, tmp-last_tmp);
                dest[i][tmp-last_tmp] = '\0';
                i++;
            }

            if (*tmp == '\0') {
                break;
            } else {
                last_tmp = tmp;
                last_tmp++;
            }
        }
        tmp++;
    } while (true);
}

bool is_playing_sd_card() {
    void *returned_data;
    bool ok = PublicData::get_value( player_checksum, is_playing_checksum, &returned_data );
    if (ok) {
        bool b = *static_cast<bool *>(returned_data);
        return b;
    } else {
        return false;
    }
}


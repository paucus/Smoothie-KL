#ifndef PLAYERPUBLICACCESS_H
#define PLAYERPUBLICACCESS_H

#define player_checksum           CHECKSUM("player")
#define is_playing_checksum       CHECKSUM("is_playing")
#define is_suspended_checksum     CHECKSUM("is_suspended")
#define abort_play_checksum       CHECKSUM("abort_play")
#define get_progress_checksum     CHECKSUM("progress")
// BEGIN MODIF time_remaining_calculation
#define get_detailed_progress_checksum CHECKSUM("get_detailed_progress")
// END MODIF time_remaining_calculation

struct pad_progress {
    unsigned int percent_complete;
    unsigned long elapsed_secs;
    string filename;
};
// BEGIN MODIF time_remaining_calculation
struct detailed_progress {
    unsigned long effective_secs;
    unsigned long played_bytes;
    unsigned long file_size;
};
// END MODIF time_remaining_calculation
#endif

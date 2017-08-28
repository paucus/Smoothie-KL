/*
 * LCD4DSelectModelScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DSELECTMODELSCREEN_H_
#define  LCD4DSELECTMODELSCREEN_H_

#include "LCD4DScreen.h"
#include "CancellableTask.h"

#define MAX_PWD_LEN 64

#define MAX_DEPTH_RECORDS 4
typedef struct dir_pos_record_s{
    unsigned char index;
    unsigned char record_pos;
    // there's no use in storing more than 256 positions as we only handle a few files.
    unsigned char position[MAX_DEPTH_RECORDS];

    dir_pos_record_s(){
        index = 0;
        record_pos = 0;
        for (unsigned int i = 0; i < MAX_DEPTH_RECORDS; i++)
            position[i] = 0;
    }
    void shift_left_records(){
        for (unsigned int i = 1; i < MAX_DEPTH_RECORDS; i++) {
            position[i-1] = position[i];
        }
    }
    void push(unsigned int i){
        if (index == MAX_DEPTH_RECORDS){
            // shift all records
            shift_left_records();
            record_pos++;
            position[MAX_DEPTH_RECORDS - 1] = i;
        } else {
            position[index] = i;
            index++;
        }
    }
    unsigned char pop(){
        if (index == 0) {
            if (record_pos > 0)
                record_pos--;
            return 0;       // we have already forgotten this value
        } else {
            return position[--index];
        }
    }
} dir_pos_record_t;

class LCD4DSelectModelScreen: public LCD4DScreen {
	public:
		LCD4DSelectModelScreen();
		int draw_screen();
		int process_click(int action, int button);
		void on_refresh();
	private:
		void show_current_file();
		void display_thumb(const char* path);
		void redraw_file_section(int dir_position);
		void draw_current_filename_and_preview(bool draw_preview);
		void cancel_preview_upload();
		void on_connection_change(connection_event_t* evt);
		// Changes the pwd to the given subdirectory.
		// If .. is given, it goes to the parent dir.
		// This function returns -1 if an error occurred. If dir is a subdirectory, on success it
		// returns 0. If dir is "..", then the position at the parent directory is returned. This
		// position is the number of files that must be skipped to show the exact position where
		// the screen was located before going into the subdirectory.
		int change_dir(const char* dir);
		// Path to working dir
		// The path is the absolute path ending with "/"
		char pwd[MAX_PWD_LEN];

		dir_pos_record_t dir_pos_record;
		int current_file_position;
		unsigned int number_of_files;
		CancellableTask* upload_img_task;
};

#endif /*  LCD4DSELECTMODELSCREEN_H_ */

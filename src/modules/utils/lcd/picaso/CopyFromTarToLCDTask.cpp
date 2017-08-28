/*
 * CopyFromTarToLCDTask.cpp
 *
 *  Created on: Dec 22, 2015
 *      Author: eai
 */

#include "CopyFromTarToLCDTask.h"
#include "LCD4DModule.h"

#define COPY_BUFF_SIZE 128
#define NUM_TRANSFERS 4

CopyFromTarToLCDTask::CopyFromTarToLCDTask(kltar_t* t, word display_filehandle, std::function<void()> on_finish_callback, std::function<void()> cancel_callback) : display_filehandle(display_filehandle), on_finish_callback(on_finish_callback), cancel_callback(cancel_callback) {
	this->t = new kltar;
	*this->t = *t;
}

CopyFromTarToLCDTask::~CopyFromTarToLCDTask() {
	delete this->t;
}

bool CopyFromTarToLCDTask::process() {
	char buff[COPY_BUFF_SIZE];
	for (int i = 0; i < NUM_TRANSFERS; i++) {
		if (kltar_cur_feof(t)) {
			kltar_close(t);
			LCD4DModule::lcd->file_Close(display_filehandle);

			// Operation finished. Execute the handler.
			on_finish_callback();
			return false;
		}

		int chread = kltar_cur_read(buff, COPY_BUFF_SIZE, t);
		if (chread > 0) {
			int chwriten = LCD4DModule::lcd->file_Write(chread, buff, display_filehandle);

			if (chread != chwriten) {
				kltar_close(t);
				LCD4DModule::lcd->file_Close(display_filehandle);
//				LCD4DModule::lcd->file_Erase((char*) new_name);
				return false;
			}
		}
	}

	return true;
}
void CopyFromTarToLCDTask::process_cancel() {
	kltar_close(t);
	LCD4DModule::lcd->file_Close(display_filehandle);
//	LCD4DModule::lcd->file_Erase((char*) new_name);
	cancel_callback();
}

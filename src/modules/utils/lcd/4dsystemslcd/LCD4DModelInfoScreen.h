/*
 * LCD4DModelInfoScreen.h
 *
 *  Created on: Feb 2, 2016
 *      Author: idlt
 */

#ifndef LCD4DMODELINFOSCREEN_H_
#define LCD4DMODELINFOSCREEN_H_

#include "LCD4DScreenWithBackButton.h"
#include "SlicingInformation.h"

class LCD4DModelInfoScreen: public LCD4DScreenWithBackButton {
public:
	LCD4DModelInfoScreen();
	virtual ~LCD4DModelInfoScreen();

	int draw_screen();
	int process_click(int action, int button);

	void set_model(const char* file, const char* fname);

private:
	void print_page(int page);
	int print_row(int row, int x, const char* text);
	int print_row(int row, int x, int num);
	int print_row(int row, int x, const char* fmt, float num);
	SlicingInformation info;
	int page;
	unsigned long fsize;
	time_t fdate;
	char* fname;
	// This variable could be necessary later if a preview is requested
	char* k3d_path;
};

#endif /* LCD4DMODELINFOSCREEN_H_ */

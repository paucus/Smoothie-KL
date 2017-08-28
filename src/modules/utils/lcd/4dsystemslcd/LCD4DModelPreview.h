/*
 * LCD4DModelPreview.h
 *
 *  Created on: Feb 2, 2016
 *      Author: idlt
 */

#ifndef LCD4DMODELPREVIEW_H_
#define LCD4DMODELPREVIEW_H_

#include "LCD4DScreenWithBackButton.h"
#include "CancellableTask.h"

class LCD4DModelPreview: public LCD4DScreenWithBackButton {
public:
	LCD4DModelPreview();
	virtual ~LCD4DModelPreview();

	int draw_screen();
	int process_click(int action, int button);

	void set_k3d_path(const char* k3d_path);

private:
	char* k3d_path;
	void display_preview();
	CancellableTask* upload_img_task;
};

#endif /* LCD4DMODELPREVIEW_H_ */

/*
 * LCD4DModelPreview.cpp
 *
 *  Created on: Feb 2, 2016
 *      Author: idlt
 */

#include "LCD4DModelPreview.h"

#define PREVIEW_FNAME "preview.gci"
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240
#define MIN_WIDTH DISPLAY_WIDTH/2
#define MIN_HEIGHT DISPLAY_HEIGHT/2

LCD4DModelPreview::LCD4DModelPreview() : upload_img_task(nullptr), k3d_path(nullptr) {
	// This screen consists only in a picture. Do not draw anything else.
	draw_temperatures_in_current_screen = false;
}

LCD4DModelPreview::~LCD4DModelPreview() {
	if (this->k3d_path) {
		delete [] this->k3d_path;
	}
}

void LCD4DModelPreview::display_preview() {
	if (!k3d_path) {
		return;
	}
	if (upload_img_task) {
		upload_img_task->cancel();
		upload_img_task = nullptr;
	}
	CancellableTask** t = &upload_img_task;
	upload_img_task = LCD4DModule::get_instance()->display_tar_img(k3d_path, PREVIEW_FNAME, DISPLAY_WIDTH/2, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, MIN_WIDTH, MIN_HEIGHT, ALIGN_IMG_CENTER_X, ALIGN_IMG_UNDER_Y, [t](){
		// Mark the task as finished in order not to cancel it by mistake.
		*t = nullptr;
	});
	if (!upload_img_task) {
		// Show a placeholder, the generic image
		lcd->img_Enable(handle, ibGenericFilePreview);
		lcd->img_SetWord(handle, ibGenericFilePreview, IMAGE_INDEX, 0);
		lcd->img_Show(handle, ibGenericFilePreview);
	}
}

void LCD4DModelPreview::set_k3d_path(const char* k3d_path) {
	if (this->k3d_path) {
		delete [] this->k3d_path;
	}
	this->k3d_path = strdup(k3d_path);
}

int LCD4DModelPreview::draw_screen() {
	LCD4DScreenWithBackButton::draw_screen();

	// Display a temporary "loading" icon
	lcd->img_Enable(handle, ibLoadingPreview);
	lcd->img_SetWord(handle, ibLoadingPreview, IMAGE_INDEX, 0);
	lcd->img_Show(handle, ibLoadingPreview);

	display_preview();
	return 0;
}
int LCD4DModelPreview::process_click(int action, int button) {
	if (upload_img_task){
		upload_img_task->cancel();
	}
	// No matter what is clicked, the picture is closed.
	this->go_to_previous_screen();
	return 0;
}

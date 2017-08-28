/*
 * LCD4DSelectModelScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DSelectModelScreen.h"
#include "LCD4DModule.h"

#include "LCD4DMessageScreen.h"
#include "LCD4DPrintingScreen.h"
#include "LCD4DMainScreen.h"
#include "LCD4DHeatingScreen.h"
#include "lcd_screens.h"
#include "PrinterMode.h"

#define MAX_LEN_TO_SHOW_IN_DISPLAY 50
#define THUMB_MAX_WIDTH 128
#define THUMB_MAX_HEIGHT 96
#define THUMB_MIN_WIDTH 90
#define THUMB_MIN_HEIGHT 75
#define DISPLAY_WIDTH 320
#define THUMB_YPOS 170
#define THUMB_FNAME "thumb.gci"

#define KL_METADATA_FILES_EXTENSION ".k3d"
#define MAX_SCREEN_X 320
#define FILE_NAME_Y_POS 20
// More lines would overlap with the preview
#define MAX_FILENAME_LINES 2

LCD4DSelectModelScreen::LCD4DSelectModelScreen() : upload_img_task(nullptr) {
	strcpy(pwd, "/");
	current_file_position = 0;
	number_of_files = 0;
}

void LCD4DSelectModelScreen::on_refresh() {
	LCD4DScreen::on_refresh();
}

static inline bool ends_with(const char* txt, char letter){
	const char* slash_ptr = strrchr(txt, letter);
	return slash_ptr && !*(slash_ptr + 1);
}

void LCD4DSelectModelScreen::display_thumb(const char* path) {
	if (upload_img_task) {
		upload_img_task->cancel();
		upload_img_task = nullptr;
	}
	CancellableTask** t = &upload_img_task;
	upload_img_task = LCD4DModule::get_instance()->display_tar_img(path, THUMB_FNAME, DISPLAY_WIDTH/ 2, THUMB_YPOS, THUMB_MAX_WIDTH, THUMB_MAX_HEIGHT, THUMB_MIN_WIDTH, THUMB_MIN_HEIGHT, ALIGN_IMG_CENTER_X, ALIGN_IMG_ABOVE_Y, [t](){
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

// To remove the extension
static size_t find_extension_pos(const char* fname) {
	const char* ext = strrchr(fname, '.');
	if (!ext) {
		// No ext, return strlen.
		return strlen(fname);
	} else {
		return (size_t)(ext - fname);
	}
}

void LCD4DSelectModelScreen::draw_current_filename_and_preview(bool draw_preview) {
	if (commandstream->has_file_list()) {
		lcd->img_ClearAttributes(handle, ibPreviousFile, I_TOUCH_DISABLE);
		lcd->img_Show(handle, ibPreviousFile);
		lcd->img_ClearAttributes(handle, ibNextFile, I_TOUCH_DISABLE);
		lcd->img_Show(handle, ibNextFile);

		const char* filename = commandstream->get_current_filename();
		bool is_dir = ends_with(filename, '/');
		// We build a temporary copy of the filename so that we can work in it and then destroy
		// it. It will be used for building the .k3d file path, add the "..." in case the
		// string is too long and remove the file extension.
		size_t buff_len = strlen(filename) + 5 + strlen(pwd) + strlen("/" PUBLIC_SD_MOUNT_DIR); // Reserve a bit more space for k3d ext.
		char* fpath = new char[buff_len];
		strcpy(fpath, "/" PUBLIC_SD_MOUNT_DIR);
		strcat(fpath, pwd);
		char* fname_cpy = &(fpath[strlen(pwd) + strlen("/" PUBLIC_SD_MOUNT_DIR)]);
		strcpy(fname_cpy, filename);
		size_t pos_replaced = 0;
		size_t len_replaced = 0;

		// Remove extension. If the name is too long, add "...".
		size_t len_without_ext = find_extension_pos(fname_cpy);
		if (len_without_ext > MAX_LEN_TO_SHOW_IN_DISPLAY) {
			// Too long. Truncate it and add "...".
			fname_cpy[MAX_LEN_TO_SHOW_IN_DISPLAY-2] = '.';
			fname_cpy[MAX_LEN_TO_SHOW_IN_DISPLAY-1] = '.';
			fname_cpy[MAX_LEN_TO_SHOW_IN_DISPLAY] = '.';
			fname_cpy[MAX_LEN_TO_SHOW_IN_DISPLAY+1] = '\0';
			pos_replaced = MAX_LEN_TO_SHOW_IN_DISPLAY - 2;
			len_replaced = 4;
		} else if (is_dir) {
			// Remove the final slash
			fname_cpy[len_without_ext-1] = '\0';
			pos_replaced = len_without_ext - 1;
			len_replaced = 1;
		} else {
			// just remove the file extension.
			fname_cpy[len_without_ext] = '\0';
			pos_replaced = len_without_ext;
			len_replaced = 1;
		}

		// We can only write in a width of at most MAX_SCREEN_Z - 2*10 because we have arrows
		// at both sides.
		screen_print(0, FILE_NAME_Y_POS, BIG, TEXT_CENTERED, WHITE, BLACK, MAX_SCREEN_X - 2*10, true, TEXT_CENTER_FROM_X, MAX_FILENAME_LINES, fname_cpy);

		// restore replaced chars in order to reuse the buffer
		if (len_replaced > 0) {
			for (int i = pos_replaced; i < pos_replaced+len_replaced; i++){
				fname_cpy[i] = filename[i];
			}
			len_replaced = 0;
		}


		// We always enable the touch of this image. Both if it is a folder or not, it works as the
		// area dedicated for the image preview.
//		lcd->img_ClearAttributes(handle, ibFolderPreview, I_TOUCH_DISABLE);

		if (draw_preview) {
			if (is_dir){
				lcd->img_ClearAttributes(handle, ibEnterFolder, I_TOUCH_DISABLE);
				lcd->img_SetAttributes(handle, ibDoPrint, I_TOUCH_DISABLE);
				lcd->img_Show(handle, ibEnterFolder);

				lcd->img_Enable(handle, ibFolderPreview);
				lcd->img_SetWord(handle, ibFolderPreview, IMAGE_INDEX, 0);
				lcd->img_Show(handle, ibFolderPreview);
			}else{
				lcd->img_ClearAttributes(handle, ibDoPrint, I_TOUCH_DISABLE);
				lcd->img_SetAttributes(handle, ibEnterFolder, I_TOUCH_DISABLE);
				lcd->img_Show(handle, ibDoPrint);

				// Check if there is a thumbnail file. Change the extension to .k3d.
				strcpy(&(fname_cpy[len_without_ext]), KL_METADATA_FILES_EXTENSION);
				FILE * thumb_file = fopen(fpath, "rb");
				if (thumb_file) {
					// File exists
					fclose(thumb_file);

					// Display a temporary "loading" icon
					lcd->img_Enable(handle, ibLoadingPreview);
					lcd->img_SetWord(handle, ibLoadingPreview, IMAGE_INDEX, 0);
					lcd->img_Show(handle, ibLoadingPreview);

					display_thumb(fpath);
				} else {
					// No thumbnail available. Draw a generic one.
					lcd->img_Enable(handle, ibGenericFilePreview);
					lcd->img_SetWord(handle, ibGenericFilePreview, IMAGE_INDEX, 0);
					lcd->img_Show(handle, ibGenericFilePreview);
				}
			}
		}

		if (!is_dir) {
			// Only files have "more info"
			lcd->img_ClearAttributes(handle, ibModelInfo, I_TOUCH_DISABLE);
			lcd->img_Enable(handle, ibModelInfo);
			lcd->img_SetWord(handle, ibModelInfo, IMAGE_INDEX, 0);
			lcd->img_Show(handle, ibModelInfo);
		} else {
			// Disable touch in "model info"
			lcd->img_SetAttributes(handle, ibModelInfo, I_TOUCH_DISABLE);
		}

		delete [] fpath;
	} else {
		lcd->img_SetAttributes(handle, ibDoPrint, I_TOUCH_DISABLE);
		lcd->img_Show(handle, ibDoPrint);
		lcd->img_SetAttributes(handle, ibPreviousFile, I_TOUCH_DISABLE);
		lcd->img_Show(handle, ibPreviousFile);
		lcd->img_SetAttributes(handle, ibNextFile, I_TOUCH_DISABLE);
		lcd->img_Show(handle, ibNextFile);

		screen_print(0, FILE_NAME_Y_POS, BIG, TEXT_CENTERED, BLACK,
				translate(NO_FILES_AVAILABLE_LABEL));
	}
}

void LCD4DSelectModelScreen::cancel_preview_upload() {
	if (upload_img_task) {
		upload_img_task->cancel();
		upload_img_task = nullptr;
	}
}

void LCD4DSelectModelScreen::show_current_file() {
	if (LCD4DModule::showing_screen == this) {

		//lets clear the air first!
		lcd->gfx_RectangleFilled(0, 0, 320, 172, WHITE);

		draw_current_filename_and_preview(true);

		lcd->img_Enable(handle, ibBack);
		lcd->img_SetWord(handle, ibBack, IMAGE_INDEX, 0);
		lcd->img_Show(handle, ibBack);
	}
}

void LCD4DSelectModelScreen::redraw_file_section(int dir_position) {
	send_gcode_v("M20 S1 P%s", commandstream, pwd);
	// skip files
	number_of_files = commandstream->get_number_of_files();
	current_file_position = dir_position;
	while(dir_position > 0){
		commandstream->get_next_filename();
		dir_position--;
	}
	show_current_file();
	on_refresh();
}
int LCD4DSelectModelScreen::draw_screen() {
	LCD4DScreen::draw_screen();

	// mount SD if not printing...
	send_gcode("M721", commandstream);

	lcd->img_Disable(handle, ALL);


	draw_lcd_images(ibBack, ibDoPrint, handle);

	redraw_file_section(current_file_position);

	return 0;
}

int LCD4DSelectModelScreen::change_dir(const char* dir) {
	if (strcmp(dir, "..") == 0) {
		// go to parent dir
		if (strcmp(pwd, "/") == 0){
			// do nothing, we are in the root
			return 0;
		} else {
			// Pop the position in the parent directory.
			unsigned char parent_dir_position = dir_pos_record.pop();

			// we always keep a / at the end, so, skip it and search the previous (that always
			// exists, because the pwd starts with one and we discarded the / path)
			int lastpos = strlen(pwd);
			lastpos--;
			// skip last /
			do {
				lastpos--;
			} while (pwd[lastpos] != '/');
			// don't forget to leave the last /
			pwd[lastpos+1] = '\0';

			return parent_dir_position;
		}
	} else {
		strncat(pwd, dir, MAX_PWD_LEN);
		int len = strnlen(pwd, MAX_PWD_LEN);
		if (len == MAX_PWD_LEN) {
			// path TOO long. Can't chdir.
			// don't change dir_pos_record
			return -1;
		} else {
			if (pwd[len-1] != '/') {
				// add / to correctly end the string
				if (len < MAX_PWD_LEN - 1) {
					// append /
					pwd[len] = '/';
					pwd[len+1] = '\0';

					dir_pos_record.push( current_file_position );
				} else {
					// we can't add the / char because it doesn't fit
					// don't change dir_pos_record
					return -1;
				}
			} else {
				dir_pos_record.push( current_file_position );
			}
		}
	}
	return 0;
}

int LCD4DSelectModelScreen::process_click(int status, int image) {
	if (status == TOUCH_PRESSED) {
		switch (image) {
			case ibPreviousFile:
				lcd->img_SetWord(handle, ibPreviousFile, IMAGE_INDEX, 1);
				lcd->img_Show(handle, ibPreviousFile);
				break;
			case ibNextFile:
				lcd->img_SetWord(handle, ibNextFile, IMAGE_INDEX, 1);
				lcd->img_Show(handle, ibNextFile);
				break;
			case ibDoPrint:
				lcd->img_SetWord(handle, ibDoPrint, IMAGE_INDEX, 1);
				lcd->img_Show(handle, ibDoPrint);
				break;
			case ibEnterFolder:
				lcd->img_SetWord(handle, ibEnterFolder, IMAGE_INDEX, 1);
				lcd->img_Show(handle, ibEnterFolder);
				break;
			case ibBack:
				lcd->img_SetWord(handle, ibBack, IMAGE_INDEX, 1);
				lcd->img_Show(handle, ibBack);
				break;
			case ibModelInfo:
				lcd->img_SetWord(handle, ibModelInfo, IMAGE_INDEX, 1);
				lcd->img_Show(handle, ibModelInfo);
				break;
			default:
				break;
		}
	} else if (status == TOUCH_RELEASED) {
		switch (image) {
			case ibPreviousFile:
				cancel_preview_upload();
				lcd->img_SetWord(handle, ibPreviousFile, IMAGE_INDEX, 0);
				lcd->img_Show(handle, ibPreviousFile);

				commandstream->get_previous_filename();
				current_file_position--;
				if (current_file_position < 0) {
					current_file_position += number_of_files;
				}
				show_current_file();
				break;
			case ibNextFile:
				cancel_preview_upload();
				lcd->img_SetWord(handle, ibNextFile, IMAGE_INDEX, 0);
				lcd->img_Show(handle, ibNextFile);

				commandstream->get_next_filename();
				current_file_position++;
				if (current_file_position >= number_of_files) {
					current_file_position -= number_of_files;
				}
				show_current_file();

				break;
			case ibDoPrint:
				cancel_preview_upload();
				lcd->img_SetWord(handle, ibDoPrint, IMAGE_INDEX, 0);
				lcd->img_Show(handle, ibDoPrint);

				// Show a temporary message saying that the G-Code file is being analyzed, so that
				// the user realizes that his command is being processed.
				lcd_screens.analyzing_file_screen->draw_screen();

				/* First try to open the file for printing... if not available, show message... */
				// remove the first slash to make it relative to the public SD following the standard of M23
				{
					const char* pwd_without_left_slash = &(pwd[1]);
					send_gcode_vd("M23 %s%s", 5 + strlen(commandstream->get_current_filename()) + strlen(pwd_without_left_slash), commandstream, pwd_without_left_slash, commandstream->get_current_filename());
				}

				if (!commandstream->file_was_selected()) {
					char temp[200];
					snprintf(temp, 200, translate(ERROR_SELECTING_FILE_FORMAT), commandstream->get_current_filename());
					lcd_screens.message_screen->set_message(temp);
					lcd_screens.message_screen->draw_screen();
				} else {
					// Draw heating screen. Once the temperature reaches target level it will send
					// M24, which will draw the printing screen.
					lcd_screens.heating_screen->draw_screen();

					if (target_bed_temperature != 0) {
						THEKERNEL->append_gcode_v_to_queue("M140 S%d", &StreamOutput::NullStream, (int)target_bed_temperature);
					}

					if ( target_hotend_temperature != 0) {
						THEKERNEL->append_gcode_v_to_queue("M109 S%d", &StreamOutput::NullStream, (int)target_hotend_temperature);
					}

					if (target_bed_temperature != 0) {
						THEKERNEL->append_gcode_v_to_queue("M190 S%d", &StreamOutput::NullStream, (int)target_bed_temperature);
					}

					THEKERNEL->append_gcode_to_queue("M24", &StreamOutput::NullStream);


					//we go to the printing screen, since the notification will arrive after temps are reached...
					//screens.printing_screen->draw_screen();
				}
				break;
				case ibEnterFolder:
					cancel_preview_upload();
					lcd->img_SetWord(handle, ibEnterFolder, IMAGE_INDEX, 0);
					lcd->img_Show(handle, ibEnterFolder);

					change_dir(commandstream->get_current_filename());
					redraw_file_section(0);

				break;
				case ibBack:
					cancel_preview_upload();
					lcd->img_SetWord(handle, ibBack, IMAGE_INDEX, 0);
					lcd->img_Show(handle, ibBack);

					if (strcmp(pwd, "/") != 0) {
						// not in the root, go back to the previous level
						int dir_position = change_dir("..");
						redraw_file_section(dir_position);
						break;
					}

					lcd_screens.main_screen->draw_screen();
				break;
				case ibModelInfo:
					lcd->img_SetWord(handle, ibModelInfo, IMAGE_INDEX, 0);
					lcd->img_Show(handle, ibModelInfo);

					cancel_preview_upload();

					{
						LCD4DModelInfoScreen& screen = *lcd_screens.model_info_screen;

						string buff;
						buff.reserve(strlen("/" PUBLIC_SD_MOUNT_DIR) + strlen(pwd) + strlen(commandstream->get_current_filename()) + 1 /* ending nullchar */);
						buff.append("/" PUBLIC_SD_MOUNT_DIR);
						buff.append(pwd);
						buff.append(commandstream->get_current_filename());

						screen.set_model(buff.c_str(), commandstream->get_current_filename());
						screen.draw_screen();
					}

				break;
				default:
					for (int i = ibBack; i <= ibNextFile; ++i) {
						if (i == ibEnterFolder) continue; // skip this. Otherwise any click will show this icon, even with files
						lcd->img_SetWord(handle, i, IMAGE_INDEX, 0);
						lcd->img_Show(handle, i);
					}
					// Redraw the filename because it will be overdrawn by previous and next buttons
					draw_current_filename_and_preview(false);
				break;
			}
		}
		return 0;
	}



void LCD4DSelectModelScreen::on_connection_change(connection_event_t* evt) {
	bool print_enabled = sd_print_allowed();
	if (!print_enabled) {
		// Print is not allowed. Go to main screen.
		lcd_screens.main_screen->draw_screen();
	}
}

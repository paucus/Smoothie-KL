/*
 * LCD4DModelInfoScreen.cpp
 *
 *  Created on: Feb 2, 2016
 *      Author: idlt
 */

#include "LCD4DModelInfoScreen.h"
#include "lcd_screens.h"
#include "font_width_table.h"
#include "SlicerCommentsGcodeAnalyzer.h"
#include "utils.h"

#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240
#define CLS_BORDER 36
#define CLS_HEIGHT 170
#define TEXT_POS_LEFT 40
#define TEXT_POS_TOP 10
#define TEXT_POS_SPACING (font_height_table[MEDIUM] + 3)
#define NUMBER_OF_PAGES 2

class LCDScreenPrint {
public:
	LCDScreenPrint(int x, int y, fontsize_t fsize) : x(x), y(y), fsize(fsize) {
	}
	LCDScreenPrint& print(const char* text) {
		LCD4DScreen::screen_print(x, y, fsize, TEXT_LEFT, text);
		x += LCD4DScreen::text_width(fsize, text);
		return *this;
	}
	LCDScreenPrint& print(int num) {
		char buff[9];
		sprintf(buff, "%d", num);
		LCD4DScreen::screen_print(x, y, fsize, TEXT_LEFT, buff);
		x += LCD4DScreen::text_width(fsize, buff);
		return *this;
	}
	LCDScreenPrint& print(const char* fmt, float num) {
		char buff[9];
		sprintf(buff, fmt, num);
		LCD4DScreen::screen_print(x, y, fsize, TEXT_LEFT, buff);
		x += LCD4DScreen::text_width(fsize, buff);
		return *this;
	}
	void set_pos(int x, int y){
		this->x = x;
		this->y = y;
	}
private:
	int x;
	int y;
	fontsize_t fsize;
};

LCD4DModelInfoScreen::LCD4DModelInfoScreen() : page(0), k3d_path(nullptr), fname(nullptr), fsize(0) {
}

LCD4DModelInfoScreen::~LCD4DModelInfoScreen() {
	if (k3d_path) {
		delete [] k3d_path;
	}
	if (fname) {
		delete [] fname;
	}
}

int LCD4DModelInfoScreen::draw_screen() {
	LCD4DScreenWithBackButton::draw_screen();

	// Clean the whole screen. If we return from LCD4DModelPreviewScreen,
	// there could be some remains of the preview.
	lcd->gfx_Rectangle(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, WHITE);

	lcd->img_Disable(handle, ALL);
	draw_lcd_images(imiPrevious, imiPreview, handle);

	page = 0;
	print_page(0);

	return 0;
}

void LCD4DModelInfoScreen::set_model(const char* file, const char* fname) {
	this->fname = strdup(fname);
	fsize = flen(file);

	// Get file date
	struct stat st;
	int result = stat(file, &st);
	if (result == 0) {
		fdate = st.st_mtime;
	} else {
		fdate = 0; // "unknown"
	}

	// Get Gcode metadata
	info = SlicingInformation(); // reset any previous info that the screen could have
	SlicerCommentsGcodeAnalyzer analyzer;
	analyzer.analyze(file, &info);

	// generate the k3d file path. We reserve 4 more bytes in case we need to concatenate .k3d.
	if (k3d_path) {
		delete[] k3d_path;
	}
	k3d_path = new char[strlen(file)];
	strcpy(k3d_path, file);
	// replace extension with k3d
	char* pos = strrchr(k3d_path, '.');
	if (!pos) {
		pos = strrchr(k3d_path, '\0');
		pos[0] = '.';
	}
	pos[1] = 'k';
	pos[2] = '3';
	pos[3] = 'd';
	pos[4] = '\0';
}

void LCD4DModelInfoScreen::print_page(int page) {
	lcd->gfx_RectangleFilled(CLS_BORDER, 0, DISPLAY_WIDTH - CLS_BORDER, CLS_HEIGHT, WHITE);

	switch (page) {
	case 0:
	{
		screen_print(TEXT_POS_LEFT, TEXT_POS_TOP, BIG, TEXT_LEFT, WHITE, BLACK, DISPLAY_WIDTH - 2*TEXT_POS_LEFT, true, fname);
		LCDScreenPrint pr(TEXT_POS_LEFT, 82, MEDIUM);
		pr.print(translate(MODEL_INFO_FILE_SIZE));
		if (fsize > 1<<30) {
			pr.print("%.2f", fsize / 1024.0 / 1024.0 / 1024.0);
			pr.print("GB");
		} else if (fsize > 1<<20) {
			pr.print("%.2f", fsize / 1024.0 / 1024.0);
			pr.print("MB");
		} else if (fsize > 1<<10) {
			pr.print("%.2f", fsize / 1024.0);
			pr.print("KB");
		} else {
			pr.print(fsize);
			pr.print("B");
		}

		pr.set_pos(TEXT_POS_LEFT, 82 + 1 * TEXT_POS_SPACING);
		pr.print(translate(MODEL_INFO_DATE));
		pr.set_pos(TEXT_POS_LEFT, 82 + 2 * TEXT_POS_SPACING);

		if (fdate != 0) {
			char str_date[] = "yyyy-mm-dd hh:mm:ss";
			struct tm mdate;
			localtime_r(&fdate, &mdate);
			strftime(str_date, sizeof(str_date), "%F %T", &mdate);
			pr.print(str_date);
		} else {
			pr.print("-");
		}
	}
	break;
	case 1:
	{
		LCDScreenPrint pr(TEXT_POS_LEFT, TEXT_POS_TOP + 0 * TEXT_POS_SPACING, MEDIUM);
		pr.print(translate(MODEL_INFO_SLICER));
		if (info.slicer == UNKNOWN_SLICER) {
			pr.print("-");
		} else {
			pr.print(slicer_name_to_string(info.slicer));
		}

		pr.set_pos(TEXT_POS_LEFT, TEXT_POS_TOP + 1 * TEXT_POS_SPACING);
		pr.print(translate(MODEL_INFO_LAYER_HEIGHT));
		if (info.layer_height.is_set) {
			pr.print("%.2f", info.layer_height / 1000.0).print("mm");
		} else {
			pr.print("-");
		}

		pr.set_pos(TEXT_POS_LEFT, TEXT_POS_TOP + 2 * TEXT_POS_SPACING);
		pr.print(translate(MODEL_INFO_PRINT_TIME));
		if (info.print_time.is_set) {
			char time_str[] = "00:00:00";
			long hs; int mins, secs;
			convert_to_time_units(info.print_time, &hs, &mins, &secs);
			format_time(time_str, hs, mins, secs);
			pr.print(time_str);
		} else {
			pr.print("-");
		}

		pr.set_pos(TEXT_POS_LEFT, TEXT_POS_TOP + 3 * TEXT_POS_SPACING);
		pr.print(translate(MODEL_INFO_EXTRUDER_TEMP));
		if (info.extruder_temp.is_set) {
			pr.print(info.extruder_temp);
			pr.print(" C");
		} else {
			pr.print("-");
		}
		pr.set_pos(TEXT_POS_LEFT, TEXT_POS_TOP + 4 * TEXT_POS_SPACING);
		pr.print(translate(MODEL_INFO_BED_TEMP));
		if (info.bed_temp.is_set) {
			pr.print(info.bed_temp);
			pr.print(" C");
		} else {
			pr.print("-");
		}

		pr.set_pos(TEXT_POS_LEFT, TEXT_POS_TOP + 5 * TEXT_POS_SPACING);
		pr.print(translate(MODEL_INFO_FILAMENT_USAGE));
		pr.set_pos(TEXT_POS_LEFT, TEXT_POS_TOP + 6 * TEXT_POS_SPACING);
		if (!info.filament_used_grams.is_set && !info.filament_used_meters.is_set && !info.filament_used_vol_cm3.is_set) {
			pr.print("-");
		} else {
			if (info.filament_used_grams.is_set) {
				pr.print("%.2f", info.filament_used_grams);
				pr.print("gr ");
			}
			if (info.filament_used_meters.is_set) {
				pr.print("%.2f", info.filament_used_meters);
				pr.print("mts ");
			}
			if (info.filament_used_vol_cm3.is_set) {
				pr.print("%.2f", info.filament_used_vol_cm3);
				pr.print("cm³");
			}
		}
	}
	break;
	}
}

int LCD4DModelInfoScreen::process_click(int action, int image) {
	if (action == TOUCH_PRESSED) {
		draw_lcd_images_touch(imiPrevious, imiPreview, handle, image);

	} else if (action == TOUCH_RELEASED) {
		switch (image) {
		case imiNext:
			if (page < NUMBER_OF_PAGES - 1) {
				page++;
				print_page(page);
			}
			draw_lcd_images(imiPrevious, imiPreview, handle);
			break;
		case imiPrevious:
			if (page > 0) {
				page--;
				print_page(page);
			}
			draw_lcd_images(imiPrevious, imiPreview, handle);
			break;
		case imiPreview:
			{
				LCD4DModelPreview& screen = *(lcd_screens.model_preview_screen);
				screen.set_k3d_path(k3d_path);
				lcd_screens.model_preview_screen->draw_screen();
			}
			break;
		case imiBack:
			go_to_previous_screen();
			break;
		default:
			draw_lcd_images(imiPrevious, imiPreview, handle);
		}
	}

	return 0;
}

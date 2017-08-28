/*
 * LCD4DScreen.cpp
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#include "LCD4DScreen.h"
#include "libs/PublicData.h"
#include "TemperatureControlPublicAccess.h"
#include <string.h>
#include "LCD4DModule.h"
#include "SwitchPublicAccess.h"
#include "font_width_table.h"
#include "lcd_screens.h"
#include "Kernel.h"
#include "Robot.h"
#include "ExtruderPublicAccess.h"
#include "utils.h"

Picaso_Serial_4DLib* LCD4DScreen::lcd = NULL;
language_t LCD4DScreen::lang = SPANISH;
word LCD4DScreen::handle = 0;
CommandParserStream* LCD4DScreen::commandstream = NULL;
word LCD4DScreen::small_font_handle = 0;
word LCD4DScreen::medium_font_handle = 0;
word LCD4DScreen::big_font_handle = 0;
float LCD4DScreen::target_bed_temperature = 0.0;
float LCD4DScreen::target_hotend_temperature = 0.0;
float LCD4DScreen::last_checked_bed_temperature = 0.0F;
float LCD4DScreen::last_checked_hotend_temp = 0.0F;

float LCD4DScreen::hot_level_temp = 50.0F;
bool LCD4DScreen::draw_notification_banner = false;

#define BG_COLOR GAINSBORO
#define MAX_SCREEN_X 320
#define min(a,b) (a<b?a:b)
#define BAD_READING_TEMP 10000

void LCD4DScreen::print_current_temperatures() {
	if (LCD4DModule::showing_screen == this && draw_temperatures_in_current_screen) {

		/* we print the temps, and update the temp buttons if needed...*/

		get_temperatures(&last_checked_bed_temperature,
						 &target_bed_temperature,
						 &last_checked_hotend_temp,
						 &target_hotend_temperature);

		char temp[15];
		if (draw_notification_banner) {
			lcd->gfx_RectangleFilled(0, 172, 320, 198, BG_COLOR);
			draw_notification_banner = false;
		}

		if (target_hotend_temperature != 0 || target_bed_temperature != 0){
			snprintf(temp, 12, "%s: ",translate(EXTRUDER));
			screen_print(42, 178, SMALL, TEXT_LEFT, BG_COLOR ,BLACK, false, temp);



			if (last_checked_hotend_temp > BAD_READING_TEMP) {
				// Invalid reading!
				screen_print(MAX_SCREEN_X - 95 - TEMP_WIDTH(SMALL), 178, SMALL, TEXT_RIGHT, BG_COLOR, RED, TEMP_WIDTH(SMALL), false, "ERR");
			} else {
				snprintf(temp, 10, "%dº",(int) last_checked_hotend_temp);
				screen_print(MAX_SCREEN_X - 95 - TEMP_WIDTH(SMALL), 178, SMALL, TEXT_RIGHT, BG_COLOR, (last_checked_hotend_temp >= hot_level_temp) ? RED : GREEN , TEMP_WIDTH(SMALL), false, temp);
			}
			snprintf(temp, 10, "/%dº",(int)target_hotend_temperature);
			screen_print(122, 178, SMALL, TEXT_LEFT, BG_COLOR, BLACK, CHAR_WIDTH(SMALL, '/') + TEMP_WIDTH(SMALL), false, temp);

			snprintf(temp, 15, "%s:",translate(PLATFORM_ACTION_NAME));
			screen_print(170, 178, SMALL, TEXT_LEFT, BG_COLOR, BLACK, false, temp);

			if (last_checked_bed_temperature > BAD_READING_TEMP) {
				// Invalid reading!
				screen_print(MAX_SCREEN_X - 234 - TEMP_WIDTH(SMALL), 178, SMALL, TEXT_RIGHT, BG_COLOR, RED, TEMP_WIDTH(SMALL), false, "ERR");
			} else {
				snprintf(temp, 10,  "%dº",(int) last_checked_bed_temperature);
				screen_print(MAX_SCREEN_X - 234 - TEMP_WIDTH(SMALL), 178, SMALL, TEXT_RIGHT, BG_COLOR, (last_checked_bed_temperature >= hot_level_temp) ? RED : GREEN, TEMP_WIDTH(SMALL), false, temp);
			}

			snprintf(temp, 10, "/%dº",(int) target_bed_temperature);
			screen_print(261, 178, SMALL, TEXT_LEFT, BG_COLOR, BLACK, CHAR_WIDTH(SMALL, '/') + TEMP_WIDTH(SMALL), false, temp);

		}else{
			snprintf(temp, 15, "                           ");
			screen_print(24, 178, SMALL, TEXT_LEFT, BG_COLOR, BLACK, false, temp);
			snprintf(temp, 12, "%s: ",translate(EXTRUDER));
			screen_print(80, 178, SMALL, TEXT_LEFT, BG_COLOR,BLACK, false, temp);
			if (last_checked_hotend_temp > BAD_READING_TEMP) {
				// Invalid reading!
				screen_print(128, 178, SMALL, TEXT_LEFT, BG_COLOR, RED, false, " ERR  ");
			} else {
				snprintf(temp, 10, " %dº  ",(int) last_checked_hotend_temp);
				screen_print(128, 178, SMALL, TEXT_LEFT, BG_COLOR, BLACK, false, temp);
			}

			snprintf(temp, 15, "                                   ");
			screen_print(255, 178, SMALL, TEXT_LEFT,BG_COLOR, BLACK, false, temp);
			snprintf(temp, 15, "%s:",translate(PLATFORM_ACTION_NAME));
			screen_print(170, 178, SMALL, TEXT_LEFT, BG_COLOR,BLACK, false, temp);

			if (last_checked_bed_temperature > BAD_READING_TEMP) {
				// Invalid reading!
				screen_print(234, 178, SMALL, TEXT_LEFT, BG_COLOR, RED, false, "ERR  ");
			} else {
				snprintf(temp, 10, "%dº   ",(int) last_checked_bed_temperature);
				screen_print(234, 178, SMALL, TEXT_LEFT, BG_COLOR, BLACK, false, temp);
			}

		}

	}
}

void LCD4DScreen::print_calibration_current_temperatures() {
	if (LCD4DModule::showing_screen == this && draw_temperatures_in_current_screen) {

		/* this method is used in the calibration screen,in order to print temperatures with one digit after the dot */

		get_temperatures(&last_checked_bed_temperature, &target_bed_temperature, &last_checked_hotend_temp,
				&target_hotend_temperature);

		char temp[90];
		snprintf(temp, 90, "     %s: %.1fº/%dº %.1fº/%dº     ", translate(CURRENT_TEMPERATURES_FORMAT),
				(float) last_checked_hotend_temp, (int) target_hotend_temperature, (float) last_checked_bed_temperature,
				(int) target_bed_temperature);

		screen_print(0, 178, SMALL, TEXT_CENTERED, BG_COLOR, BLACK, false, temp);

	}
}

void LCD4DScreen::get_current_speed(int* current_speed) {
	float cs = 6000.0 / THEKERNEL->robot->get_seconds_per_minute();
	*current_speed = cs;
}

void LCD4DScreen::get_current_flowrate(int* current_speed) {
	float extrmult = get_public_data_val<float>(extruder_checksum, extruder_multiplier_checksum, 0, 1.0);
	*current_speed = (int)(extrmult * 100.0);
}

void LCD4DScreen::set_temperatures(float bed_temp, float heater_temp) {
	target_bed_temperature = bed_temp;
	target_hotend_temperature = heater_temp;

	// with no third argument, it sets the target_temperature
	bool ok = PublicData::set_value(temperature_control_checksum, bed_checksum, &bed_temp);

	if (!ok) {
		THEKERNEL->streams->printf("Error: Temp controller Not Found\r\n");
	}

	// with no third argument, it sets the target_temperature
	ok = PublicData::set_value(temperature_control_checksum, hotend_checksum,
			&heater_temp);
	if (!ok) {
		THEKERNEL->streams->printf("Error: Temp controller Not Found\r\n");
	}
}
float LCD4DScreen::get_max_hotend_temperature() {
	// TODO Given that this value doesn't change, we could cache it. However, we just is it now
	// when changing temp from the display, so we don't really need to worry about it now.
	float max_temp;
	bool ok = PublicData::get_value(temperature_control_checksum, max_set_temp_checksum, hotend_checksum, &max_temp);
	if (!ok) {
		THEKERNEL->streams->printf("Error: Temp controller Not Found\r\n");
		return 0;
	} else {
		return max_temp;
	}
}
float LCD4DScreen::get_max_bed_temperature() {
	// TODO Given that this value doesn't change, we could cache it. However, we just is it now
	// when changing temp from the display, so we don't really need to worry about it now.
	float max_temp;
	bool ok = PublicData::get_value(temperature_control_checksum, max_set_temp_checksum, bed_checksum, &max_temp);
	if (!ok) {
		THEKERNEL->streams->printf("Error: Temp controller Not Found\r\n");
		return 0;
	} else {
		return max_temp;
	}
}

void LCD4DScreen::get_fan_status(bool* fan_on) {
	// get fan status
	struct pad_switch s;
	bool ok = PublicData::get_value(switch_checksum, fan_checksum, 0, &s);
	if (ok) {
		*fan_on = s.state;
	} else {
		// fan probably disabled
		*fan_on = false;
	}

}

void LCD4DScreen::get_temperatures(float* bed_temp, float* target_bed_temp, float* heater_temp,
	float* target_heater_temp) {
	struct pad_temperature temp;

	bool ok = PublicData::get_value( temperature_control_checksum, current_temperature_checksum,
			bed_checksum, &temp);

	if (ok) {
		*bed_temp = temp.current_temperature;
		*target_bed_temp = temp.target_temperature;
	} else {
		THEKERNEL->streams->printf("Error: Temp controller Not Found\r\n");
		*bed_temp = -1;
		*target_bed_temp = -1;
	}

	ok = PublicData::get_value( temperature_control_checksum, current_temperature_checksum,
			hotend_checksum, &temp);

	if (ok) {
		*heater_temp = temp.current_temperature;
		*target_heater_temp = temp.target_temperature;
	} else {
		THEKERNEL->streams->printf("Error: Temp controller Not Found\r\n");
		*heater_temp = -1;
		*target_heater_temp = -1;
	}

}

bool LCD4DScreen::init_screens(Picaso_Serial_4DLib* lc, word h, CommandParserStream *cs) {
	lcd = lc;
	commandstream = cs;
	handle = h;

	if (handle != 0) {
		small_font_handle = lcd->file_LoadImageControl("tahoma14.dat", "tahoma14.gci", 1);
		medium_font_handle = lcd->file_LoadImageControl("tahoma18.dat", "tahoma18.gci", 1);
		big_font_handle = lcd->file_LoadImageControl("tahoma24.dat", "tahoma24.gci", 1);

		if (handle && small_font_handle && medium_font_handle && big_font_handle) {
			return true;
		}
	}

	return false;
}

void LCD4DScreen::set_language(const char* language) {
	LCD4DScreen::lang = LCDTranslations::str_to_language_t(language);
	THEKERNEL->streams->printf("Now using language %s.\r\n", LCDTranslations::language_t_to_str(LCD4DScreen::lang));
}

void LCD4DScreen::set_language(language_t language) {
	LCD4DScreen::lang = language;
}

language_t LCD4DScreen::get_language() {
	return LCD4DScreen::lang;
}

void LCD4DScreen::screen_print(int x, int y, fontsize_t font, textalign_t align, const char* text) {
	screen_print(x, y, font, align, BLACK, text);
}

void LCD4DScreen::screen_print(int x, int y, fontsize_t font, textalign_t align, word FGcolor, const char* text) {
	screen_print(x, y, font, align, FGcolor, true, text);
}

void LCD4DScreen::screen_print(int x, int y, fontsize_t font, textalign_t align, word FGcolor, word BGwidth, const char* text) {
	screen_print(x, y, font, align, WHITE, FGcolor, BGwidth, true, TEXT_CENTER_FROM_X, 0, text);
}

void LCD4DScreen::screen_print(int x, int y, fontsize_t font, textalign_t align, word BGcolor, word FGcolor,
	bool breaklines, const char* text) {
	screen_print(x, y, font, align, BGcolor ,FGcolor, breaklines, TEXT_CENTER_FROM_X, text);
}

void LCD4DScreen::screen_print(int x, int y, fontsize_t font, textalign_t align, word BGcolor, word FGcolor,
	word BGwidth, bool breaklines, const char* text) {
	screen_print(x, y, font, align, BGcolor ,FGcolor, BGwidth, breaklines, TEXT_CENTER_FROM_X, 0, text);
}

void LCD4DScreen::screen_print(int x, int y, fontsize_t font, textalign_t align, word FGcolor, bool breaklines,
	const char* text) {
	screen_print(x, y, font, align, FGcolor, breaklines, TEXT_CENTER_FROM_X, text);
}

void LCD4DScreen::screen_print(int x, int y, fontsize_t font, textalign_t align, word FGcolor, bool breaklines,
	text_center_strategy_t center_strategy, const char* text) {
	screen_print(x, y, font, align, WHITE, FGcolor, breaklines, center_strategy, text);
}

/* This is an attempt to speed up the sizing of texts.
 * We use an array to save the charwidth, so we only check once per char per size...*/
word LCD4DScreen::get_char_width(fontsize_t font_size, const char ch) {
	if (ch >= FIRST_CHAR_TABLE && ch <= LAST_CHAR_TABLE){
		return font_width_table[font_size][ch - FIRST_CHAR_TABLE];
	} else if (ch == 'º') { // a very common char from extended ascii
		return degrees_width[font_size];
	} else {
		return lcd->charwidth((char) ch);
	}
}
word LCD4DScreen::get_char_height(fontsize_t font_size, const char ch) {
	return font_height_table[font_size];
}
word LCD4DScreen::text_width (fontsize_t font, const char* text) {
	word len = 0;
	const char *cp = text;

	while (*cp != 0) {
		len += get_char_width(font, (char) *cp);
		cp++;
	}
	return len;
}

/**
 * If text ends, returns the position of null char. So check that the pointed position is not \0!!!
 */
word LCD4DScreen::accum_until_width(const char* text, word max_line_width, fontsize_t font, word* line_width){
	word width = 0;
	word pos = 0;
	word last_word_end_position = 0;
	word width_until_last_full_word = 0;

	while (width < max_line_width) {
		if (text[pos] == '\0') {
			// Text ends here. Return the position of the null char so that it's easier
			// for the user to verify it ends here.
			*line_width = width;
			return pos;
		}
		if (text[pos] == '\n') {
			// Return char. Line ends here no matter the length.
			*line_width = width;
			return pos + 1;
		}

		if ((text[pos] == ' ' || text[pos] == '\t') && pos > 0) {
			last_word_end_position = pos;
			*line_width = width_until_last_full_word;
		}

		width += get_char_width(font, text[pos]);

		if (!(text[pos] == ' ' || text[pos] == '\t') && width < max_line_width){
			// This is the actual length so far.
			width_until_last_full_word = width;
		}

		pos++;
	}

	// Text is longer than a line.
	if (last_word_end_position > 0) {
		// More than one word entered the length of the line. Split at the end of that word.

		// Accumulate ending spaces so that next line doesn't start with a gap
		while (text[last_word_end_position] != '\0' && (text[last_word_end_position] == ' ' || text[last_word_end_position] == '\t')) {
			last_word_end_position++;
		}
		return last_word_end_position;
	} else {
		// There is a very long word. Split as far as we could reach of that word.
		// This is the last width captured before the end of the space.
		*line_width = width_until_last_full_word;
		return pos;
	}
}

/**
 * This function returns an array with the position where a text must be split
 * in order not to exceed the max displayable length.
 * This can be due to:
 * - Space or tab character
 * - A carriage return character
 * - A word that exceeds max length
 * This method doesn't modify the original text, but instead returns an array with the positions
 * where each line starts.
 * In order to print this text, you must print the text between each of the positions. When the
 * returned position points to '\0', this means that the text finished, and you mustn't keep
 * traversing the string, or a segmentation fault might occur.
 * The first array starts with 0 in order to ease the processing.
 * The line lengths are returned in the array parameter lines_length. The maximum char height is
 * returned in max_char_height.
 */
void LCD4DScreen::wordwrap(const char* text, word* lines_positions, word* lines_length, word* max_char_height, word max_num_lines, word max_line_width, fontsize_t font) {
	word cur_pos = 0;
	word i;
	*max_char_height = font_height_table[font];	// NOTE: Actually, we KNOW the height. So we set it. If this were to change, modify it to calculate it.
	for (i = 0; i < max_num_lines && (text[cur_pos] != '\0'); i++) {
		lines_positions[i] = cur_pos;
		lines_length[i] = 0;
		cur_pos += accum_until_width(&(text[cur_pos]), max_line_width, font, &(lines_length[i]));
	}
	lines_positions[i] = cur_pos;
}

void LCD4DScreen::screen_print(int x, int y, fontsize_t font, textalign_t align, word BGcolor, word FGcolor,
	bool breaklines, text_center_strategy_t center_strategy, const char* text) {
	screen_print(x, y, font, align, BGcolor, FGcolor, 0, breaklines, center_strategy, 0, text);
}

void LCD4DScreen::fill_area(word x, word width, word y, word height, word color) {
	fill_area_to(x, x+width, y, height, color);
}
void LCD4DScreen::fill_area_to(word x_min, word x_max, word y, word height, word color) {
	lcd->gfx_RectangleFilled(x_min, y, x_max - 1, y+height - 1, color);
}

/**
 * Lets you print text to the display, using 1 of the 3 custom fonts (and sizes), allowing for text alignment.
 *
 * This function is supposed to remove the guess work when positioning text on the display...
 *
 * @param x the x coordinate to start drawing. if TEXT_CENTERED, the text is centered from the passed x to the end of display.
 * 			If TEXT_RIGHT, x is used as padding on the right side.
 * 	@param y the y coordinate.
 * 	@param font the fontsize_t for small, medium or big.
 * 	@param align the textalign_t member defining text alignment.
 * 	@param BGcolor the color of the background of the text. needs to be 565 color (@see RGBto565), some are defined in Picaso_Const4D.h
 * 	@param FGcolor the color of the text. needs to be 565 color (@see RGBto565), some are defined in Picaso_Const4D.h
 * 	@param breaklines break long lines on spaces...
 * 	@param center_strategy how do we center? using x as right padding or center the text on the x point?...
 * 	@param text characters to be printed
 */
//FIXME: Remove text_center_strategy and create TEXT_CENTERED_AROUND_X, TEXT_CENTERED_FROM_X
void LCD4DScreen::screen_print(int x, int y, fontsize_t font, textalign_t align, word BGcolor, word FGcolor, word BGwidth,
	bool breaklines, text_center_strategy_t center_strategy, word text_max_lines, const char* text) {

	/* center around x implies breaklines and TEXT_CENTERED */
	if (center_strategy == TEXT_CENTER_AROUND_X) {
		breaklines = true;
		align = TEXT_CENTERED;
	}

	switch (font) {
		case SMALL:
			lcd->txt_FontID(small_font_handle);
			break;
		case MEDIUM:
			lcd->txt_FontID(medium_font_handle);
			break;
		case BIG:
			lcd->txt_FontID(big_font_handle);
			break;
		default:
			lcd->txt_FontID(0);
			break;
	}

	if (breaklines) {
		/* Implement this method to 'magically' break the lines ...*/
		word max_char_height = 0;
		int max_allowed_len;

		if (BGwidth > 0) {
			// If a width is specified, use it
			max_allowed_len = BGwidth;
		} else if (center_strategy == TEXT_CENTER_AROUND_X) {
			/* need to check maximum len in case we need to center around x */
			max_allowed_len = x > (MAX_SCREEN_X / 2) ? (MAX_SCREEN_X - x) * 2 : x * 2;
		} else {
			max_allowed_len = MAX_SCREEN_X - x;
		}

#define MAX_LINES_PER_SCREEN 10
		word num_lines[MAX_LINES_PER_SCREEN+1];
		word line_length[MAX_LINES_PER_SCREEN+1];
		wordwrap(text, num_lines, line_length, &max_char_height, MAX_LINES_PER_SCREEN, max_allowed_len, font);

		lcd->txt_BGcolour(BGcolor);
		lcd->txt_FGcolour(FGcolor);

		// Print up to text_max_lines if specified, or MAX_LINES_PER_SCREEN if 0
		word max_lines = text_max_lines > 0?min(text_max_lines, MAX_LINES_PER_SCREEN):MAX_LINES_PER_SCREEN;
		word i;
		for (i = 0; i < max_lines && text[num_lines[i]]; i++) {
			int corrected_x = x;
			if (align == TEXT_CENTERED) {
				if (center_strategy == TEXT_CENTER_AROUND_X) {
					corrected_x = x - (line_length[i] / 2);

					if (line_length[i] < BGwidth) {
						fill_area(x - BGwidth / 2, (BGwidth - line_length[i]) / 2, y, max_char_height, BGcolor);
						fill_area(x + line_length[i] / 2, (BGwidth - line_length[i]) / 2, y, max_char_height, BGcolor);
					}

				} else {
					// this is the default
					//if (center_strategy == TEXT_CENTER_FROM_X) {
					corrected_x = (MAX_SCREEN_X + x - line_length[i]) / 2;

					if (line_length[i] < BGwidth) {
						fill_area_to((MAX_SCREEN_X + x - BGwidth) / 2, corrected_x, y, max_char_height, BGcolor);
						fill_area_to(corrected_x + line_length[i], (MAX_SCREEN_X + x + BGwidth) / 2, y, max_char_height, BGcolor);
					}
				}
			} else if (align == TEXT_RIGHT) {
				corrected_x = (MAX_SCREEN_X - line_length[i] - x);
				if (line_length[i] < BGwidth){
					fill_area_to(corrected_x - (BGwidth - line_length[i]), corrected_x, y, max_char_height, BGcolor);
				}
			} else {
				// align left
				if (line_length[i] < BGwidth){
					fill_area(corrected_x + line_length[i], BGwidth - line_length[i], y, max_char_height, BGcolor);
				}
			}

			lcd->gfx_MoveTo(corrected_x, y);
			lcd->putstr(text, num_lines[i], num_lines[i+1]);

			y += (max_char_height * 1.2); // line separation...
		}
		// Clean up to text_max_lines at most. If 0 was specify, don't clean.
		max_lines = text_max_lines > 0?min(text_max_lines, MAX_LINES_PER_SCREEN):0;
		for (; i < max_lines; i++) {
			int corrected_x = x;
			if (align == TEXT_CENTERED && center_strategy == TEXT_CENTER_AROUND_X) {
				corrected_x = x - (BGwidth / 2);
			} else if (align == TEXT_CENTERED) {
				corrected_x = (MAX_SCREEN_X + x - BGwidth) / 2;
			} else if (align == TEXT_RIGHT) {
				corrected_x = (MAX_SCREEN_X - BGwidth - x);
			} else {
				// align left
			}
			fill_area(corrected_x, BGwidth, y, max_char_height, BGcolor);

			y += (max_char_height * 1.2); // line separation...
		}
	} else {
		word len = 0;
		if ((BGwidth > 0) || (align == TEXT_CENTERED || align == TEXT_RIGHT)) {
			//There's no lcd->strwidth so we count the chars... :(
			len = text_width(font, text);

			if ((align == TEXT_CENTERED) && (len > (MAX_SCREEN_X - x))) {
				// when the text is longer than screen width, we set x to 0 and show what we can... Need to do something better...
				x = 0;
			} else if (align == TEXT_CENTERED) {
				x = (MAX_SCREEN_X + x - len) / 2;
			} else if (align == TEXT_RIGHT) {
				x = (MAX_SCREEN_X - len - x);
			}
		}

		lcd->txt_BGcolour(BGcolor);
		lcd->txt_FGcolour(FGcolor);

		lcd->gfx_MoveTo(x, y);
		lcd->putstr(text);

		if (len < BGwidth) {
			word max_char_height = font_height_table[font];
			// Clean background only where nothing was written. This is equivalent to cleaning the
			// screen and then writing the text, but this causes a blinking that is bothersome.
			if ((align == TEXT_CENTERED) && (len > (MAX_SCREEN_X - x))) {
				// nothing to erase
			} else if (align == TEXT_CENTERED) {
				fill_area_to(x - (BGwidth - len) / 2, x, y, max_char_height, BGcolor);
				fill_area_to(x + len, x + (len + BGwidth) / 2, y, max_char_height, BGcolor);
			} else if (align == TEXT_RIGHT) {
				fill_area_to(x + len - BGwidth, x, y, max_char_height, BGcolor);
			} else {
				// left
				fill_area_to(x + len, x + BGwidth, y, max_char_height, BGcolor);
			}
		}
	}
}

int LCD4DScreen::draw_screen() {
	LCD4DModule::clear_screen_stack();	// No other screen depends on this one because it's an end screen

	// Do some garbage collection of unused screens
	lcd_clear_all_screens_except(this);

	/* Need to set ourselves as the showing screen, to get the click event!*/
	LCD4DModule::showing_screen = this;
	on_draw();
	return 0;
}

void LCD4DScreen::on_draw(){

	lcd->gfx_BGcolour(WHITE);
	lcd->gfx_Cls();
	draw_notification_banner = true;
	print_current_temperatures();
}

void LCD4DScreen::on_refresh() {
	print_current_temperatures();
}

void LCD4DScreen::on_periodic_tick(){

}

TmpPtr<LCD4DScreen> LCD4DScreen::get_tmp_ptr() {
	return TmpPtr<LCD4DScreen>(this, lcd_get_counter(this));
}

/*
 * LCD4DScreen.h
 *
 *  Created on: Feb 25, 2015
 *      Author: abialet
 */

#ifndef LCD4DSCREEN_H_
#define LCD4DSCREEN_H_

#include "CommandParserStream.h"
#include "GcodeUtils.h"
#include "ImagesConstants.h"
#include "LCD4DModuleConstants.h"
#include "LCD4DScreen.h"
#include "LCDTranslations.h"
#include "libs/Kernel.h"
#include "libs/StreamOutput.h"
#include "picaso/Picaso_Serial_4DLib.h"
#include "utils/Gcode.h"
#include "LCDUtils.h"
#include "TmpPtr.h"
#include "connection_event.h"

class LCD4DScreen {
	public:
		virtual ~LCD4DScreen(){};

		static bool init_screens(Picaso_Serial_4DLib* lcd, word h, CommandParserStream *commandstream);

		virtual int draw_screen();
		virtual int process_click(int action, int button) =0;

		/* FIXME This is a "temporary" hack.
		 * In some screens, we need to periodically update what's being shown... But we can't use timers to
		 * update the display because the on_idle function of the LCD4DModule needs to connect periodically
		 * to get touch events, and in doing so, we eventually get to a point where two different 'threads'
		 * try to access the serial connection concurrently, which is a BIG no no.
		 *
		 * What we do, is to call this method approximately every second (on the screen being shown only) to
		 * give the screen a chance to refresh. Methods called from this method need to be "cooperative",
		 * meaning that if the do a long operation, the firmware will be halted for as long as it takes...
		 *
		 *  */
		virtual void on_refresh();
		virtual void on_draw();
		virtual void on_periodic_tick();
		TmpPtr<LCD4DScreen> get_tmp_ptr();
		virtual void on_connection_change(connection_event_t* evt) {};

		static void screen_print(int x, int y, fontsize_t font, textalign_t align, const char* text);
		static void screen_print(int x, int y, fontsize_t font, textalign_t align, word FGcolor, const char* text);
		static void screen_print(int x, int y, fontsize_t font, textalign_t align, word FGcolor, word BGwidth, const char* text);
		static void screen_print(int x, int y, fontsize_t font, textalign_t align, word FGcolor, bool breaklines,
			const char* text);
		static void screen_print(int x, int y, fontsize_t font, textalign_t align, word FGcolor, bool breaklines,
			text_center_strategy_t center_strategy, const char* text);
		static void screen_print(int x, int y, fontsize_t font, textalign_t align, word BGcolor, word FGcolor,
			bool breaklines, text_center_strategy_t center_strategy, const char* text);
		static void screen_print(int x, int y, fontsize_t font, textalign_t align, word BGcolor, word FGcolor,
			bool breaklines, const char* text);
		static void screen_print(int x, int y, fontsize_t font, textalign_t align, word BGcolor, word FGcolor,
			word BGwidth, bool breaklines, const char* text);
		static void screen_print(int x, int y, fontsize_t font, textalign_t align, word BGcolor, word FGcolor,
			word BGwidth, bool breaklines, text_center_strategy_t center_strategy, word text_max_lines, const char* text);
		static word text_width (fontsize_t font, const char* text) ;

		static void set_temperatures(float bed_temp, float heater_temp);
		static void get_temperatures(float* bed_temp, float* target_bed_temp, float* heater_temp,
			float* target_heater_temp);
		static float get_max_hotend_temperature();
		static float get_max_bed_temperature();

		static void get_fan_status(bool* fan_on);
		static void get_current_speed(int* current_speed);
		static void get_current_flowrate(int* current_speed);

		static language_t get_language();
		static void set_language(language_t);
		static void set_language(const char*);

		static word small_font_handle;
		static word medium_font_handle;
		static word big_font_handle;
	protected:
		static Picaso_Serial_4DLib* lcd;
		static language_t lang;
		static word handle;
		static CommandParserStream *commandstream;

		static float target_bed_temperature;
		static float target_hotend_temperature;

		static float last_checked_bed_temperature;
		static float last_checked_hotend_temp;
		static float hot_level_temp;

		virtual void print_current_temperatures();
		virtual void print_calibration_current_temperatures();
		static void wordwrap(const char* text, word* lines_positions, word* lines_length, word* max_char_height, word max_num_lines, word max_display_width, fontsize_t font);
		static word accum_until_width(const char* text, word max_display_width, fontsize_t font, word* line_length);
		static void fill_area(word x, word width, word y, word height, word color);
		static void fill_area_to(word x_min, word x_max, word y, word height, word color);

		static bool draw_notification_banner;

		static word get_char_width(fontsize_t size, const char ch) ;
		static word get_char_height(fontsize_t size, const char ch) ;

		bool draw_temperatures_in_current_screen = true;
};

#endif /* LCD4DSCREEN_H_ */

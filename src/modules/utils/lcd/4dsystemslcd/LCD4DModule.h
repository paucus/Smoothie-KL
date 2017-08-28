#ifndef LCD4DMODULE_H_
#define LCD4DMODULE_H_

#include "CommandParserStream.h"
#include "GcodeUtils.h"
#include "LCD4DModuleConstants.h"
#include "LCDTranslations.h"
#include "libs/Kernel.h"
#include "libs/Module.h"
#include "libs/StreamOutput.h"
#include "picaso/Picaso_Serial_4DLib.h"
#include "picaso/SerialConsoleStream.h"
#include "utils/Gcode.h"
#include "version.h"
#include <string.h>
#include <deque>
#include <functional>
#include "CancellableTask.h"

#include "LCD4DScreen.h"
#include "LCD4DUpdateProgressScreenScreen.h"

#define translate(a) LCDTranslations::get_translation(LCD4DScreen::get_language(), a)
#define THELCD LCD4DModule::get_instance()
#define lcd_module_enable_checksum        CHECKSUM("lcd_module_enable")
#define lcd_module_energy_mode_checksum   CHECKSUM("lcd_module_energy_mode")

#define ALIGN_IMG_LEFT_X  -1
#define ALIGN_IMG_CENTER_X 0
#define ALIGN_IMG_RIGHT_X  1
#define ALIGN_IMG_ABOVE_Y -1
#define ALIGN_IMG_CENTER_Y 0
#define ALIGN_IMG_UNDER_Y  1

class LCD4DModule: public Module {
	public:
		LCD4DModule();
		virtual ~LCD4DModule();

//		void on_main_loop(void* argument);
		virtual void on_module_loaded();
		void on_second_tick(void *);
		void on_autolevel_status_change(void *argument);
		void on_cold_extrusion(void*argument);
		void on_idle(void* argument);
		void on_gcode_received(void *argument);
		void on_console_line_received( void *argument );
		void on_alert_triggered(void*argument);
		void on_out_of_filament(void *argument);
		void on_print_status_change(void *argument);
		void on_upgrade_process_begin(void *argument);
		void on_pid_autotune_event(void*);
		uint32_t on_periodic_tick(uint32_t dummy);
		void on_get_public_data(void *argument);
		void on_set_public_data(void *argument);
		void on_connection_change(void *);

		static LCD4DScreen* showing_screen;
		static deque<LCD4DScreen*> screens_stack;

		static LCD4DModule* get_instance();

		/* upgrade methods */
		bool copy_file_to_display_sd(const char* to_copy, const char* new_name, bool screen_refresh);
		bool copy_tar_file_to_display_sd(const char* tar, const char* to_copy, const char* new_name);

		// This method is an asynchronous version of copy_tar_file_to_display_sd.
		// It returns a cancellable task, which can be used to end the copy process.
		CancellableTask* start_copy_tar_file_to_display_sd_job(const char* tar, const char* to_copy, const char* new_name, std::function<void()> on_finish_callback, std::function<void()> on_cancel_callback);

		// Creates a file in the LCD SD card with the given name and using the given text as
		// content.
		bool dump_data_to_display_sd(char* text, const char* new_name);

		// This function starts an asynchronous operation to display in the LCD screen a picture
		// stored in a tar archive.
		// @param path: The tar file
		// @param imgname: The image to display (in gci format)
		// @param px, py: The x and y coordinates of a point to position the image.
		// @param max_width, max_height: The maximum allowable picture size
		// @param align_x, align_y: These params lets the user specify how the picture should be
		// drawn relative to px and py. Each can be -1, 0 or 1. You can use the constants
		// ALIGN_IMG_XXXXX to specify the alignment more easily.
		// @param task_close_callback: A callback called both if the operation is finished well or
		// cancelled. Its purpose is to destroy any resource reserved for this operation.
		CancellableTask* display_tar_img(const char* path, const char* imgname, unsigned int px, unsigned int py, unsigned int max_width, unsigned int max_height, unsigned int min_width, unsigned int min_height, int align_x, int align_y, std::function<void()> task_close_callback);

		void init_upgrade_process();
		void do_update_display();

		int display_do_init();
		static Picaso_Serial_4DLib* lcd;
		static SerialConsoleStream* stream;

		static void clear_screen_stack();
		static void push_screen_shown();
		static void push_screen(LCD4DScreen* screen);
		static LCD4DScreen* pop_screen_and_draw(LCD4DScreen* fallback);
		static LCD4DUpdateProgressScreenScreen* get_update_screen();
	private:
		void set_energy_mode(bool enable);
		word handle;
		bool energy_mode;
		CommandParserStream *commandstream;

		static LCD4DModule* instance;
};

#endif // LCD4DMODULE_H_


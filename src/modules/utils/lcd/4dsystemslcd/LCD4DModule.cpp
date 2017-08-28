#include "AutoLevelStatus.h"
#include "ImagesConstants.h"
#include "LCD4DModule.h"
#include "LCD4DUpdateProgressScreenScreen.h"
#include "PrintStatus.h"
#include "SerialNumber.h"
#include "StreamOutput.h"
#include "StreamOutputPool.h"
#include "Config.h"
#include "checksumm.h"
#include "utils.h"
#include "ConfigValue.h"
#include "SlowTicker.h"
#include "PublicDataRequest.h"

#include "LCD4DAdjustmentsScreen.h"
#include "LCD4DAutolevelingScreen.h"
#include "LCD4DCalibrationScreen.h"
#include "LCD4DCancelConfirmationScreen.h"
#include "LCD4DChangeFilamentScreen.h"
#include "LCD4DInformationScreen.h"
#include "LCD4DLanguageScreen.h"
#include "LCD4DMainScreen.h"
#include "LCD4DManualControlScreen.h"
#include "LCD4DMessageScreen.h"
#include "LCD4DPauseScreen.h"
#include "LCD4DPrepareScreen.h"
#include "LCD4DPrintDoneScreen.h"
#include "LCD4DPrintingScreen.h"
#include "LCD4DScreenSaverScreen.h"
#include "LCD4DSelectModelScreen.h"
#include "LCD4DPrepareCustomTempScreen.h"
#include "LCD4DCriticalErrorScreen.h"
#include "LCD4DHeatingScreen.h"
#include "LCD4DPIDAutotuneScreen.h"

#include "pid_autotune_event.h"
#include "font_width_table.h"
#include "lcd_screens.h"
#include "kltar.h"
#include "gcifmt.h"
#include "Tasks.h"
#include "CopyFromTarToLCDTask.h"
#include "Picaso_Serial_4DLib.h"
#include "alert_event.h"
#include "crc8.h"

#define DISPLAY_IMG_GCI "img.gci"
#define DISPLAY_IMG_DAT "img.dat"
// BEGIN MODIF update_firmware
#define ON_IDLE_PERIODIC_CALL_BYTE_PERIOD (1 << 19)
// END MODIF update_firmware
#define MIN_MILLIS_BETWEEN_CHECKS 100
#define MIN_MILLIS_BETWEEN_CHECKS_SLEEPING 1000
#define COPY_BUFF_SIZE 128
#define SECONDS_TO_SLEEP 300 // 5 minutes
#define WAKING_UP_NUMBER_OF_CLICKS_TO_IGNORE 5 // 500mseg / MIN_MILLIS_BETWEEN_CHECKS = 5

// long = 10 hours
#define LONG_TASK_TIMEOUT 36000000
#define DEFAULT_TIMEOUT 500

#define send_to_update_screen(...) if (display_inited) update_screen->printf_above_progress(__VA_ARGS__)

extern SerialConsoleBase* lcd_serial;

static int display_inited = 0;
static int display_objects_created = 0;
static int display_init_errors = 0;
static bool request_reset_flag = false;
static volatile unsigned int seconds_until_sleep = SECONDS_TO_SLEEP;
// After waking up, we must ignore the click that would happen due to touching the screen to wake it up.
// As the screen can detect false clicks, we must confirm that the screen has really been released, so
// this variable allow us to ignore N clicks (given by constant WAKING_UP_NUMBER_OF_CLICKS_TO_IGNORE).
static volatile unsigned int disable_actions_for_n_clicks = 0;
// flag to know if we can access the lcd or someone else is using it
static volatile bool lcd_busy = false;
static volatile bool lcd_sleeping = false;

static volatile bool needs_refresh = false;

LCD4DScreen* LCD4DModule::showing_screen = NULL;
deque<LCD4DScreen*> LCD4DModule::screens_stack;
LCD4DModule* LCD4DModule::instance = NULL;
Picaso_Serial_4DLib* LCD4DModule::lcd = NULL;
SerialConsoleStream* LCD4DModule::stream = NULL;

static LCD4DUpdateProgressScreenScreen* update_screen = NULL;

// In an attempt to keep code changes from edge, we decided to keep this way of handling M117.
#define panel_display_message_checksum CHECKSUM("display_message")
#define panel_checksum             CHECKSUM("panel")

LCD4DModule::LCD4DModule() : energy_mode (true) {
	instance = this;
	request_reset_flag = false;
}

LCD4DModule* LCD4DModule::get_instance() {
	return instance;
}

LCD4DModule::~LCD4DModule() {
	if (lcd)
		delete lcd;
	if (stream)
		delete stream;
}

static void lcd_error(int err, unsigned char c) {
	THEKERNEL->streams->printf("display error %d %d\r\n", err, (int)c);
//	if (err == Err4D_NAK) {
		request_reset_flag = true;
//	}
}

static void lcd_error_on_baud_change(int err, unsigned char c) {
	display_init_errors++;

	printf("%lu -> errors: %d. error code: %d (%s) while waiting for baudrate change\n", millis(), display_init_errors,
			err, err == 1 ? "timeout" : "nack");
}

void LCD4DModule::on_module_loaded() {
    if( !THEKERNEL->config->value( lcd_module_enable_checksum )->by_default(true)->as_bool() ){
        // as not needed free up resource
        delete this;
        return;
    }

    energy_mode = THEKERNEL->config->value( lcd_module_energy_mode_checksum )->by_default(true)->as_bool();

    this->register_for_event(ON_GCODE_RECEIVED);
    this->register_for_event(ON_CONSOLE_LINE_RECEIVED);
	this->register_for_event(ON_SECOND_TICK);
//	this->register_for_event(ON_MAIN_LOOP);
	this->register_for_event(ON_IDLE);

	/* Our status events */
	this->register_for_event(ON_PRINT_STATUS_CHANGE);
	this->register_for_event(ON_AUTOLEVEL_STATUS_CHANGE);
	this->register_for_event(ON_OUT_OF_FILAMENT);
	this->register_for_event(ON_COLD_EXTRUSION);
	this->register_for_event(ON_UPGRADE_PROCESS_BEGIN);
	this->register_for_event(ON_PID_AUTOTUNE_EVENT);
	this->register_for_event(ON_GET_PUBLIC_DATA);
	this->register_for_event(ON_SET_PUBLIC_DATA);
	this->register_for_event(ON_CONNECTION_CHANGE);
	this->register_for_event(ON_ALERT_TRIGGERED);

	commandstream = new CommandParserStream();

	stream = new SerialConsoleStream(lcd_serial);
	lcd = new Picaso_Serial_4DLib(stream, LCD_RST_PIN);

	display_do_init();
}


void LCD4DModule::on_second_tick(void *) {
	/* FIXME This is a "temporary" hack. See LCD4DScreen for a complete explanation... */
	// we want to call on_refresh() every second ...
	if (display_inited) {
		needs_refresh = true;
		if (energy_mode)
			seconds_until_sleep--;
	}

	if (seconds_until_sleep == 0) {
		lcd->gfx_Contrast(0);
		lcd_sleeping = true;
	}
}

void LCD4DModule::on_idle(void* argument) {
//void LCD4DModule::on_main_loop(void* argument) {
	static unsigned int lastcheck = millis();
	static unsigned int lastcheck2 = millis();

	if (request_reset_flag) {
		LCD4DModule::lcd->reset_display();
		while (lcd_serial->buffer_size() > 0) {
			lcd_serial->buffer_delete_tail();
		}
		LCD4DModule::stream->set_baudrate(LCD_INIT_BAUDRATE);
		TmpPtr<LCD4DScreen> curr_screen = LCD4DModule::showing_screen->get_tmp_ptr();
		this->display_do_init();
		curr_screen->draw_screen();
		request_reset_flag = false;
	}

	if (lcd_sleeping) {
		if ((millis() - lastcheck) > MIN_MILLIS_BETWEEN_CHECKS_SLEEPING) {
			lcd_busy = true;
			lastcheck = millis();

			int status = lcd->touch_Get(TOUCH_STATUS);

			if (status != NOTOUCH) {
				lcd->gfx_Contrast(1);
				/* We do not want to process this touch, only wake from sleep */
//				word image = lcd->img_Touched(handle, ALL);
//				showing_screen->process_click(status, image);
				disable_actions_for_n_clicks = WAKING_UP_NUMBER_OF_CLICKS_TO_IGNORE;
				lcd_sleeping = false;
				seconds_until_sleep = SECONDS_TO_SLEEP;
			}
			lcd_busy = false;
		}
	} else {
		if (needs_refresh && !lcd_busy) {
			lcd_busy = true;
			showing_screen->on_refresh();
			lcd_busy = false;

			needs_refresh = false;
		}

		if (display_inited && (millis() - lastcheck) > MIN_MILLIS_BETWEEN_CHECKS && !lcd_busy) {
			lcd_busy = true;
			lastcheck = millis();

			int status = lcd->touch_Get(TOUCH_STATUS);

			if (disable_actions_for_n_clicks == 0) {	// check if touch is still disabled as a consequence of having just woken up
				// The screen hasn't just woken up, so, let it work normally.
				if (status != NOTOUCH) {
					word image = lcd->img_Touched(handle, ALL);
					showing_screen->process_click(status, image);
					seconds_until_sleep = SECONDS_TO_SLEEP;
				}
			} else {
				// The screen is still waking up. The user hasn't released the screen yet,
				// so, ignore all clicks until it releases the screen for a number of clicks
				// given by constant WAKING_UP_NUMBER_OF_CLICKS_TO_IGNORE. To do that, check
				// if the user is still pressing. Otherwise, reset the
				// disable_actions_for_n_clicks flag yo WAKING_UP_NUMBER_OF_CLICKS_TO_IGNORE.
				if (status == NOTOUCH) {
					// once disable_actions_for_n_clicks becomes 0, touch is reenabled.
					disable_actions_for_n_clicks--;
				} else {
					disable_actions_for_n_clicks = WAKING_UP_NUMBER_OF_CLICKS_TO_IGNORE;
				}
			}

			lcd_busy = false;
		}

		if (display_inited && (millis() - lastcheck2) > 25 && !lcd_busy) {
			lcd_busy = true;
			lastcheck2 = millis();

			if(showing_screen) {
				this->showing_screen->on_periodic_tick();
			}

			lcd_busy = false;

		}

	}
	/* FIXME: If display wasn't inited, but we have a touch, we can assume that the user inserted a new sd card,
	 * and wants to init the display with it... Do the init process again... */
}

static int get_file_info(const char* path, time_t* t, off_t* size) {
	struct stat st;
	int result = stat(path, &st);
	if (result != 0) {
		*t = 0;
		*size = 0;
		return result;       // invalid date, return the default date
	}
	*t = st.st_mtime;
	*size = st.st_size;
	return 0;
}

bool LCD4DModule::copy_file_to_display_sd(const char* to_copy, const char* new_name, bool do_screen_notif) {
	off_t file_size;
	time_t time_m;

	int res = get_file_info(to_copy, &time_m, &file_size);
	if (res != 0){
		if (do_screen_notif) update_screen->printf("Failed to get stats from file %s. Aborting.\r\n", to_copy);
		return false;
	}

	if (display_inited)
		if (do_screen_notif) update_screen->start_progress_bar();

	FILE* ffrom = fopen(to_copy, "r");
	if (!ffrom) {
		if (display_inited)
			if (do_screen_notif) update_screen->printf("Source file does not exist: %s. Aborting.\r\n", to_copy);
		return false;
	}

	/* We need to erase existing file or opening for write will fail.
	 * from 4D manual: If a file is opened for write mode 'w' , and the file already exists,
	 * the operation will fail. Unlike C and some other languages where the file will be erased
	 * ready for re-writing when opened for writing, 4DGL offers a simple level of protection that
	 * ensures that a file must be purposely erased before being re-written.*/
	if (lcd->file_Exists(new_name)) {
		lcd->file_Erase(new_name);
	}

	word display_filehandle = lcd->file_Open(new_name, 'w');
	if (display_filehandle < 1) {
		if (display_inited)
			if (do_screen_notif) update_screen->printf("Error opening file handle on display: %d. Aborting.\r\n", display_filehandle);

		fclose(ffrom);
		return false;
	}

	long copied = 0;
	char buff[COPY_BUFF_SIZE];
	while (!feof(ffrom)) {
		int chread = fread(buff, 1, COPY_BUFF_SIZE, ffrom);
		int chwriten = lcd->file_Write(chread, buff, display_filehandle);

		if (chread != chwriten) {
			if (display_inited)
				if (do_screen_notif) update_screen->printf("Read and copied sizes do not match. Read: %d Copied: %d. Aborting.\r\n", chread,
						chwriten);

			fclose(ffrom);
			lcd->file_Close(display_filehandle);
			lcd->file_Erase(new_name); // try not to leave bad files on display...
			return false;
		}
		copied += chwriten;

		if (copied % ON_IDLE_PERIODIC_CALL_BYTE_PERIOD == 0) {	// every 512KB call on_idle event
			THEKERNEL->call_event(ON_IDLE);
		}

		/* We call on_idle so everyone has a chance to do what they do while we are copying...*/
//		THEKERNEL->call_event(ON_IDLE);

		if (do_screen_notif) update_screen->update_progress(file_size != 0 ? (double) copied / file_size : 0.0);
	}

	fclose(ffrom);
	lcd->file_Close(display_filehandle);

	if (display_inited)
		if (do_screen_notif) update_screen->printf("File %s copied. Size: %ld KB.\r\n", new_name, (copied / 1024));

	if (display_inited)
		if (do_screen_notif) update_screen->stop_progress_bar();

	return true;

}

// BEGIN MODIF tar
CancellableTask* LCD4DModule::start_copy_tar_file_to_display_sd_job(const char* tar_file, const char* to_copy, const char* new_name, std::function<void()> on_finish_callback, std::function<void()> cancel_callback) {
	kltar_t t;
	int r = kltar_find_open(&t, tar_file, to_copy);
	if (r != KLTAR_OK) {
		return nullptr;
	} else {
		// The file was found in the tar archive.
		// Copy the file in the same way as copy_file_to_display_sd
		// We need to erase existing file or opening for write will fail.
		if (lcd->file_Exists(new_name)) {
			lcd->file_Erase(new_name);
		}

		word display_filehandle = lcd->file_Open(new_name, 'w');
		if (display_filehandle < 1) {
			kltar_close(&t);
			return nullptr;
		}

		// From this point on, the process differs from copy_tar_file_to_display_sd. Now this must
		// be performed in chuncks.
		CancellableTask* copy_task = new CopyFromTarToLCDTask(&t, display_filehandle, on_finish_callback, cancel_callback);
		Tasks::instance->schedule(copy_task);

		return copy_task;
	}
}
bool LCD4DModule::copy_tar_file_to_display_sd(const char* tar_file, const char* to_copy, const char* new_name) {
	kltar_t t;
	int r = kltar_find_open(&t, tar_file, to_copy);
	if (r != KLTAR_OK) {
		return false;
	} else {
		// The file was found in the tar archive.
		// Copy the file in the same way as copy_file_to_display_sd
		// We need to erase existing file or opening for write will fail.
		if (lcd->file_Exists( new_name)) {
			lcd->file_Erase(new_name);
		}

		word display_filehandle = lcd->file_Open(new_name, 'w');
		if (display_filehandle < 1) {
			kltar_close(&t);
			return false;
		}

		int copied = 0;
		while (!kltar_cur_feof(&t)) {
			char buff[COPY_BUFF_SIZE];
			int chread = kltar_cur_read(buff, COPY_BUFF_SIZE, &t);
			if (chread > 0) {
				int chwriten = lcd->file_Write(chread, buff, display_filehandle);

				if (chread != chwriten) {
					kltar_close(&t);
					lcd->file_Close(display_filehandle);
					lcd->file_Erase( new_name); // try not to leave bad files on display...
					return false;
				}
				copied += chwriten;

				if (copied % ON_IDLE_PERIODIC_CALL_BYTE_PERIOD == 0) {	// every 512KB call on_idle event
					THEKERNEL->call_event(ON_IDLE);
				}
			}
		}

		lcd->file_Close(display_filehandle);
		kltar_close(&t);
	}
	return true;
}
bool LCD4DModule::dump_data_to_display_sd(char* text, const char* new_name) {
	// Copy the file in the same way as copy_file_to_display_sd
	// We need to erase existing file or opening for write will fail.
	if (lcd->file_Exists( new_name)) {
		lcd->file_Erase( new_name);
	}

	word display_filehandle = lcd->file_Open(new_name, 'w');
	if (display_filehandle < 1) {
		return false;
	}

	int chread = strlen(text);
	int chwriten = lcd->file_Write(chread, text, display_filehandle);

	if (chread != chwriten) {
		lcd->file_Close(display_filehandle);
		lcd->file_Erase(new_name); // try not to leave bad files on display...
		return false;
	}

	lcd->file_Close(display_filehandle);
	return true;
}
// END MODIF tar

static int to_picaso_baudrate(int freq) {
	switch (freq){
		case 9600: return BAUD_9600;
		case 19200: return BAUD_19200;
		case 56000: return BAUD_56000;
		case 115200: return BAUD_115200;
		case 128000: return BAUD_128000;
		case 256000: return BAUD_256000;
		default: return BAUD_9600;
	}
}

int LCD4DModule::display_do_init() {
#define retries 3

	/* We init with a "special" callback until we change the baud rate, we will replace it after
	 * Also, use a smaller timeout, since we will be trying to get an ACK back repeatedly */
	lcd->TimeLimit4D = 500;
	lcd->Callback4D = lcd_error_on_baud_change;
	/* First, print version info and change the speed at which we communicate with the display.
	 * To do that, we need to send the command at 9600 (which is what's flashed on the display,
	 * and get the response after a while with the new speed... */
	lcd->gfx_Set(SCREEN_MODE, LANDSCAPE_R);

	lcd->putstr("Kikai Labs. Version: ");
	lcd->putstr(__GITVERSIONSTRING__);
	lcd->putstr("\nInitializing...\n");

	lcd->setbaudWait(to_picaso_baudrate(LCD_BAUDRATE));

	/* Now, check that we actually get an ack... And keep trying until we can do it... We use
	 * charwidth since it has to return a size != 0 */

	word res;
	do {
		res = lcd->charwidth('A');
		// sleep some time?
		printf("res: %d\n", res);
	} while (res == 0 && display_init_errors < retries);

	if (res == 0) {
		/* we never got an ACK, return?...*/
		return -1;
	}

	/* After we leave the loop, we already changed the baudrate, and can actually use the display :)
	 * Set back the timeout and the callback... And business as usual! */
	lcd->TimeLimit4D = 2000;
	lcd->Callback4D = lcd_error;

	lcd->gfx_Cls();

	/* FIXME: If we cant init the display, show an image with the method described
	 * at: http://www.4dsystems.com.au/downloads/Application-Notes/4D-AN-00137_R_1_0.pdf */
	int i = 0, j = 0;

	i = lcd->file_Mount();

	if (!i) {
		lcd->putstr("Please insert card\n");

		while ((!i) && (j++ < retries)) {
			lcd->putstr(". ");

			i = lcd->file_Mount();

			delay(1000);
		}
	}

	lcd->gfx_BGcolour(WHITE);
	lcd->gfx_Cls();

	if (!i) {
		display_inited = 0;

		lcd->txt_BGcolour(WHITE);
		lcd->txt_FGcolour(BLACK);

		/* FIXME: Also here, show image with blit method */
		char message[40] = "Display can not be inited.";

		lcd->gfx_MoveTo(10, 50);
		lcd->putstr(message);
	} else {
		handle = lcd->file_LoadImageControl("DisplayE.dat", "DisplayE.gci", 1);

		if (LCD4DScreen::init_screens(lcd, handle, commandstream)) {
			display_inited = 1;

			lcd->TimeLimit4D = DEFAULT_TIMEOUT;

			if (!display_objects_created) {
				lcd_initialize_counters();

				// If any screen must be instantiated when this module is created, do it here
				display_objects_created = 1;
			}
			showing_screen = lcd_screens.main_screen;
			showing_screen->draw_screen();

		} else {
			//TODO Translate string...
			lcd->gfx_MoveTo(10, 50);
			lcd->putstr("Display can't be inited.Can't read needed files from sd.\n");
		}

		lcd->touch_Set(TOUCH_ENABLE);
	}

	return display_inited;
}

static bool exists_file(const char* file) {
	FILE* f;
	if ((f = fopen(file, "r")) != NULL) {
		fclose(f);
	}
	return f != NULL;
}

void LCD4DModule::init_upgrade_process() {
	if (update_screen == NULL && display_inited)
			update_screen = new LCD4DUpdateProgressScreenScreen();
}

void LCD4DModule::do_update_display() {
	/* Disconnect the serial port because the copy takes too long and it hangs the host OS (at least on Mac OS X)*/
//	usbserial.disconnect();
	if (update_screen == NULL && display_inited)
		update_screen = new LCD4DUpdateProgressScreenScreen();

	if (display_inited) {
		if (showing_screen != update_screen)
			update_screen->draw_screen();

		update_screen->printf("Starting display files copy!.\n");
	}

	/* Files names MUST be 8.3 format for the display */
	const char* proposed_files[] = {
			"DisplayE.dat",
			"DisplayE.gci",
			"tahoma14.gci",
			"tahoma14.dat",
			"tahoma18.gci",
			"tahoma18.dat",
			"tahoma24.gci",
			"tahoma24.dat" };
#define NO_OF_FILES (sizeof(proposed_files)/sizeof(const char*))

	for (unsigned int i = 0; i < NO_OF_FILES; i++) {
		char name[30];
		snprintf(name, sizeof(name), "/" PUBLIC_SD_MOUNT_DIR "/display/%s", proposed_files[i]);
		if (exists_file(name)) {
			if (display_inited)
				update_screen->printf_above_progress(translate(UPDATE_COPYING_FORMAT), i + 1, NO_OF_FILES);

			/* FIXME: There's no file_Rename() method on the lcd, so we copy the file overwriting the old ones.
			 * At some point, we may want to write to temporary files and rename after we have copied the new ones.*/
			/* We need to use a temp filename, and we cant append .tmp directly since we have to use 8.3 filenames */

			if (display_inited)
				update_screen->printf("Copying '%s' to '%s'!.\n", name, proposed_files[i]);

			unsigned long start = millis();

			bool copied = this->copy_file_to_display_sd(name, proposed_files[i], true);

			if (display_inited) {
				if (copied) {
					update_screen->printf("Copy successful! Took %lu ms.\n", millis() - start);
				} else {
					update_screen->printf("Copy failed!.\n");
				}
			}

			/* if we can't copy one of the files, try to revert what we did so far and leave. If we can't revert (erase copied files)
			 * is not a big deal... these files will be overwritten on the next upgrade... */
			if (!copied) {
				if (display_inited) {
					update_screen->printf_above_progress(translate(UPDATE_FAILED_TEXT));
					update_screen->printf("Display update was not successful.\n");
				}
				return;
			} else {
				/* file was copied, delete it... */
				int s = remove(name);
				if (s != 0) {
					if (display_inited) {
						update_screen->printf("Could not delete %s.\n", name);
					}
				}
			}
		}
	}

	if (display_inited) {
		update_screen->printf_above_progress(translate(UPDATE_SUCCESS_TEXT));
		update_screen->printf("Display update was successful.\n");
	}
}

int RGBto565(int r, int g, int b) {
	int result = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
	return result;
}

inline static void print_single_char_width_ranges(char c0, char ce, Gcode*gcode, Picaso_Serial_4DLib* lcd){
	gcode->stream->printf("\r\nRange '%c'-'%c': {", c0, ce);
	for (char c = c0; c <= ce; c++) gcode->stream->printf("%d%c", lcd->charwidth(c), c==ce?'}':',');
}
static word to_font_size(fontsize_t font){
	switch (font) {
		case SMALL:
			return LCD4DScreen::small_font_handle;
		case MEDIUM:
			return LCD4DScreen::medium_font_handle;
		case BIG:
			return LCD4DScreen::big_font_handle;
	}
	return 0;
}
static void print_all_char_widths(fontsize_t f, Gcode* gcode, Picaso_Serial_4DLib* lcd) {
	lcd->txt_FontID(to_font_size(f));
	print_single_char_width_ranges(FIRST_CHAR_TABLE, LAST_CHAR_TABLE, gcode, lcd); // includes all alphabet chars, numbers, and glyphs
	gcode->stream->printf("\r\nDegree: %d", lcd->charwidth('º'));
	gcode->stream->printf("\r\nCubic: %d", lcd->charwidth('³'));
}
static void print_height(fontsize_t f, Gcode* gcode, Picaso_Serial_4DLib* lcd) {
	lcd->txt_FontID(to_font_size(f));
	gcode->stream->printf("\r\nHeight: %d\r\n", lcd->charheight('A'));
}
void LCD4DModule::on_gcode_received(void *argument) {
	Gcode* gcode = static_cast<Gcode*>(argument);

	// M117 now is handled in GcodeDispatch.
//	if (gcode->has_m && gcode->m == 117) {
//		/* Show message screen ... */
//		lcd_screens.message_screen->set_message(gcode->get_command());
//		lcd_screens.message_screen->draw_screen();
//	}
	if (gcode->has_m && gcode->m == 778) {
		lcd->reset_display();
		display_do_init();
		THEKERNEL->streams->printf("display reset!.\r\n");
	} else if (gcode->has_m && gcode->m == 798) {
		THEKERNEL->streams->printf("Using %s for display.\r\n", LCDTranslations::language_t_to_str(LCD4DScreen::get_language()));
	} else if (gcode->has_m && gcode->m == 799) {
		/* We use this to set the display's language */
		// We use gcode->get_command()+1 because the first character is a space and we don't want it
		LCD4DScreen::set_language(gcode->get_command()+1);
	} else if (gcode->has_m && gcode->m == 800) {	// Print font widths
		gcode->stream->printf("\r\nSMALL");
		print_all_char_widths(SMALL, gcode, lcd);
		print_height(SMALL, gcode, lcd);
		gcode->stream->printf("\r\nMEDIUM");
		print_all_char_widths(MEDIUM, gcode, lcd);
		print_height(MEDIUM, gcode, lcd);
		gcode->stream->printf("\r\nBIG");
		print_all_char_widths(BIG, gcode, lcd);
		print_height(BIG, gcode, lcd);
	} else if (gcode->has_m && gcode->m == 806) {
		// M806 E1/0
		// energy efficient mode. When enabled, the LCD will periodically turn off.
		if (gcode->has_letter('E')) {
			set_energy_mode(gcode->get_int('E'));
		}
		gcode->stream->printf("LCD Energy Mode: %s\n", this->energy_mode?"ON":"OFF");
	} else if (gcode->has_m && (gcode->m == 500 || gcode->m == 503)) {
		gcode->stream->printf("M806 E%d\r\n", this->energy_mode);
	} else if (gcode->has_m && (gcode->m == 510 || gcode->m == 513)) {
		gcode->stream->printf("M799 %s\r\n", LCDTranslations::language_t_to_str(LCD4DScreen::get_language()));
	}
}
void LCD4DModule::on_get_public_data(void *argument){
	PublicDataRequest *pdr = static_cast<PublicDataRequest *>(argument);
	if (pdr->starts_with(lcd_module_energy_mode_checksum)) {
		pdr->set_data_ptr(&energy_mode);
		pdr->set_taken();
	}
}
void LCD4DModule::set_energy_mode(bool enable) {
	if (this->energy_mode == enable) {
		// Nothing changed
		return;
	}
	this->energy_mode = enable;
	if (lcd_sleeping && energy_mode) {
		// restart screen
		lcd->gfx_Contrast(1);
		lcd_sleeping = false;
	}
	seconds_until_sleep = SECONDS_TO_SLEEP;
}
void LCD4DModule::on_set_public_data(void *argument){
	PublicDataRequest *pdr = static_cast<PublicDataRequest *>(argument);

	if (pdr->starts_with(lcd_module_energy_mode_checksum)) {
		set_energy_mode(*((bool*)pdr->get_data_ptr()));
		pdr->set_taken();
		return;
	}

	if(!pdr->starts_with(panel_checksum)) return;

	if (pdr->second_element_is(panel_display_message_checksum)) {
		lcd_screens.message_screen->set_message(((string*)pdr->get_data_ptr())->c_str());
		lcd_screens.message_screen->draw_screen();
	}
}

void LCD4DModule::on_print_status_change(void *argument) {
	print_status_change_t e = *(static_cast<print_status_change_t*>(argument));

	//cp_idle, cp_printing, cp_paused
	switch (e.event) {
		case pe_begin:
			if (!lcd_screens.printing_screen.is_ptr(showing_screen)) {
				lcd_screens.printing_screen->draw_screen();
			}
			break;
		case pe_pause:
			if (e.print_source == PS_SD_CARD) {
				// Only show the pause screen if current print was sent from an SD card. We can't resume if it was printed from USB.
				lcd_screens.pause_screen->draw_screen();
			}
			break;
		case pe_resume:
			lcd_screens.printing_screen->draw_screen();
			break;
		case pe_end:
			lcd_screens.printing_done_screen->set_print_time(e.total_print_time);
			lcd_screens.printing_done_screen->draw_screen();
			break;
		default:
			break;
	}
}

void LCD4DModule::on_autolevel_status_change(void *argument) {
	autolevel_status_change_t e = *(static_cast<autolevel_status_change_t*>(argument));

	static TmpPtr<LCD4DScreen> lastscreen;

	if (e.event == al_begin) {
		lastscreen = showing_screen->get_tmp_ptr();
		lcd_screens.autoleveling_screen->draw_screen();
	} else if (e.event == al_end) {
		lastscreen->draw_screen();
	} else {
		/* Failed! */
		// FIXME: What do we do? when does it fail? Is there anything to do?
	}
}

void LCD4DModule::on_out_of_filament(void *argument) {
	THEKERNEL->streams->printf("END OF FILAMENT!!!\r\n");

	lcd_screens.change_filament_screen->was_called_from_eof_event = true;
	lcd_screens.change_filament_screen->draw_screen();
}

void LCD4DModule::on_cold_extrusion(void*argument) {
	lcd_screens.message_screen->warn_cold_extrusion();
}

void LCD4DModule::on_alert_triggered(void*argument) {
	alert_event_t* alert = static_cast<alert_event_t*>(argument);
	if (alert->reason == alert_mintemp) {
		lcd_screens.message_screen->warn_mintemp_triggered(alert->mintemp_event.source);
	} else if (alert->reason == alert_thermistor_out) {
		lcd_screens.message_screen->warn_thermistor_out();
	} else {
		char buff[] = "on_alert_triggered event = -2000111222.";
		sprintf(buff, "on_alert_triggered event = %d.", (int)alert->reason);
		lcd_screens.message_screen->warn_unknown_error(buff);
	}
}

void LCD4DModule::on_upgrade_process_begin(void *argument) {
	lcd_busy = true;
	if (display_inited) {
		if (update_screen == NULL)
			update_screen = new LCD4DUpdateProgressScreenScreen();

		if (showing_screen != update_screen)
			update_screen->draw_screen();
	}
}

LCD4DUpdateProgressScreenScreen* LCD4DModule::get_update_screen() {
	return update_screen;
}

void LCD4DModule::clear_screen_stack(){
	screens_stack.clear();
}
void LCD4DModule::push_screen_shown(){
	push_screen(LCD4DModule::showing_screen);
}
void LCD4DModule::push_screen(LCD4DScreen* screen){
	screens_stack.push_back(screen);
}
LCD4DScreen* LCD4DModule::pop_screen_and_draw(LCD4DScreen* fallback){
	if (!screens_stack.empty()) {
		LCD4DScreen* next = screens_stack.back();
		screens_stack.pop_back();
		next->draw_screen();
		return next;
	} else {
		return fallback;
	}
}

void LCD4DModule::on_console_line_received( void *argument ){
    SerialMessage new_message = *static_cast<SerialMessage *>(argument);

    char first_char = new_message.message[0];
    if(strchr(";( \n\rGMTN", first_char) != NULL) return;

    string possible_command = new_message.message;

    string cmd = shift_parameter(possible_command);

    if (strncasecmp(cmd.c_str(), "lcd-cp", 6) == 0) {
        string from = absolute_from_relative(shift_parameter( possible_command ));
        string to = shift_parameter(possible_command);

        long start = millis();
        bool copied = copy_file_to_display_sd(from.c_str(), to.c_str(), true);

        if (copied) {
            new_message.stream->printf("Copy successful! Took %lu ms.\n", millis() - start);
        } else {
            new_message.stream->printf("Copy failed!.\n");
        }
	// BEGIN MODIF tar
    } else if (strncasecmp(cmd.c_str(), "lcd-tar-cp", 10) == 0) {
        string tar = absolute_from_relative(shift_parameter( possible_command ));
        string from = shift_parameter( possible_command );
        string to = shift_parameter(possible_command);

        long start = millis();
        bool copied = copy_tar_file_to_display_sd(tar.c_str(), from.c_str(), to.c_str());

        if (copied) {
            new_message.stream->printf("Copy successful! Took %lu ms.\n", millis() - start);
        } else {
            new_message.stream->printf("Copy failed!.\n");
        }
	// END MODIF tar
    } else if (strncasecmp(cmd.c_str(), "help", 4) == 0) {
        new_message.stream->printf("lcd-cp file1 file2 - copy file to lcd\r\n");
        // BEGIN MODIF tar
        new_message.stream->printf("lcd-tar-cp filetar file1 file2 - copy file from tar to lcd\r\n");
        // END MODIF tar
	// BEGIN MODIF copy_in_lcd
	} else if (strncasecmp(cmd.c_str(), "lcdsd-cp", 8) == 0) {
		// Good reference
		// https://github.com/4dsystems/Picaso-Serial-C-Library/blob/master/C/Samples/BIGDEMO.C
		string from = shift_parameter( possible_command );
		string to = shift_parameter(possible_command);

		new_message.stream->printf("copying internally file in SD %s to %s\n", from.c_str(), to.c_str());
		word src_file_h = lcd->writeString(0, (char*)from.c_str());
		word dst_file_h = lcd->writeString(src_file_h, (char*)to.c_str());
		word args[] = {src_file_h, dst_file_h};
		lcd->TimeLimit4D = LONG_TASK_TIMEOUT;
		word ret = lcd->file_Exec("FILECOPY.4FN", 2, args);
		lcd->TimeLimit4D = DEFAULT_TIMEOUT;

	} else if (strncasecmp(cmd.c_str(), "checksum", 8) == 0) {
		string file = possible_command;

		new_message.stream->printf("Checksum %s\n", file.c_str());
		word src_file_h = lcd->writeString(0, (char*)file.c_str());
		word args[] = {src_file_h};
		lcd->TimeLimit4D = LONG_TASK_TIMEOUT;
		word res = lcd->file_Exec("CHECKSUM.4FN", 1, args);
		lcd->TimeLimit4D = DEFAULT_TIMEOUT;
		new_message.stream->printf("Result %d\n", res);
	} else if (strncasecmp(cmd.c_str(), "loccrc", 6) == 0) {
		string file = possible_command;

		new_message.stream->printf("Locally Checksum %s\n", file.c_str());
		uint8_t crc = crc8_file(file.c_str());
		new_message.stream->printf("Result %d\n", crc);

	// END MODIF copy_in_lcd
    }
}

void LCD4DModule::on_pid_autotune_event(void* argument) {
	if (!display_inited) {
		// Display not initialized. That means that probably we won't be able to process the request.
		return;
	}
	pid_autotune_event_t* event = static_cast<pid_autotune_event_t*>(argument);

	if (event->event == pid_autotune_begin){
		lcd_screens.pid_autotune_screen->reset_wizard(event->pool_index, event->name_checksum, event->temp);
		lcd_screens.pid_autotune_screen->draw_screen();
	} else if (event->event == pid_autotune_abort){
		// Process finished
		lcd_screens.main_screen->draw_screen();
	} else if (event->event == pid_autotune_end){
		// Process finished
		lcd_screens.pid_autotune_screen->on_finish_autotune();
	} else { //if (event->event == pid_autotune_tested){
		lcd_screens.pid_autotune_screen->on_pid_tested(event->peak_temp, event->temp, event->time_to_stabilize, event->time_to_reach_temp);
	}
}

void LCD4DModule::on_connection_change(void* argument) {
	connection_event_t* evt = static_cast<connection_event_t*>(argument);
	if (this->showing_screen) {
		this->showing_screen->on_connection_change(evt);
	}
}


static bool gci_dimensions_in_tar_match(const char* tar_path, const char* gci_file, unsigned int max_width, unsigned int max_height, unsigned int min_width, unsigned int min_height, unsigned int* img_width, unsigned int* img_height) {
	kltar_t t;
	int r = kltar_find_open(&t, tar_path, gci_file);
	if (r == KLTAR_OK) {
		unsigned char data[6];
		// File found.
		if (kltar_cur_read(&data, 6, &t) != 6) {
			// Something failed. Just in case, return false to cancel the operation.
			return false;
		}
		kltar_close(&t);

		gcifmt_t gcit;
		parse_gci_header(&gcit, data);
		*img_width = gcit.width;
		*img_height = gcit.height;
		// Only 16 bits images are allowed, so, validate colors and fps too.
		return gcit.width <= max_width && gcit.width >= min_width && gcit.height <= max_height && gcit.height >= min_height && gcit.colors == 16 && gcit.fps == 0;
	} else {
		// Something failed. Just in case, return false to cancel the operation.
		return false;
	}
}

CancellableTask* LCD4DModule::display_tar_img(const char* path, const char* imgname, unsigned int px, unsigned int py, unsigned int max_width, unsigned int max_height, unsigned int min_width, unsigned int min_height, int align_x, int align_y, std::function<void()> task_close_callback) {
	// Validate image format and dimensions
	unsigned int img_width;
	unsigned int img_height;
	if (!gci_dimensions_in_tar_match(path, imgname, max_width, max_height, min_width, min_height, &img_width, &img_height)){
		// Invalid image!
		return nullptr;
	}

	// TODO Omit this in the future if already copied (confirm no one overrid it)
	char data[] = "\"cur_img\" 0000 0000 00 00\n";
	static bool dat_file_uploaded = false; // Only upload it once
	if (!dat_file_uploaded) {
		bool copied = LCD4DModule::dump_data_to_display_sd(data, DISPLAY_IMG_DAT);
		if (!copied) {
			return nullptr;
		}
		dat_file_uploaded = true;
	}

	CancellableTask* task = start_copy_tar_file_to_display_sd_job(path, imgname, DISPLAY_IMG_GCI, [img_width, img_height, align_x, align_y, px, py, task_close_callback](){
		word handle = lcd->file_LoadImageControl(DISPLAY_IMG_DAT, DISPLAY_IMG_GCI, 1);
		lcd->img_SetWord(handle, 0, IMAGE_WIDTH, img_width);
		lcd->img_SetWord(handle, 0, IMAGE_HEIGHT, img_height);
		lcd->img_SetWord(handle, 0, IMAGE_XPOS, align_x>0?px:align_x<0?px-img_width:(px-img_width/2));
		lcd->img_SetWord(handle, 0, IMAGE_YPOS, align_y>0?py:align_y<0?py-img_height:(py-img_height/2));
		lcd->img_SetWord(handle, 0, IMAGE_INDEX, 0);

		lcd->img_ClearAttributes(handle, 0, TOUCH_DISABLE);
		lcd->img_Show(handle, 0);
		lcd->mem_Free(handle);

		task_close_callback();
	}, [task_close_callback]() {
		task_close_callback();
	});

	return task;
}


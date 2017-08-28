/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#include "libs/Kernel.h"

#include "modules/tools/laser/Laser.h"
#include "modules/tools/spindle/Spindle.h"
#include "modules/tools/extruder/ExtruderMaker.h"
#include "modules/tools/temperaturecontrol/TemperatureControlPool.h"
#include "modules/tools/endstops/Endstops.h"
// BEGIN MODIF filament
#include "modules/tools/endoffilament/EndOfFilament.h"
#include "modules/tools/endoffilament/PrinterParking.h"
// #include "modules/tools/endoffilament/RepetierHostNotification.h"
#include "modules/tools/filamenttemps/FilamentTemps.h"
// END MODIF filament
// BEGIN MODIF external sdcard
#include "modules/communication/SDCardInsert.h"
// END MODIF external sdcard
// BEGIN MODIF serial number
#include "modules/utils/serialnumber/SerialNumber.h"
// END MODIF serial number
// BEGIN MODIF status report
#ifndef NO_TOOLS_STATUSREPORT
#include "modules/tools/statusreport/StatusReport.h"
#endif // NO_TOOLS_STATUSREPORT
// END MODIF status report
// BEGIN MODIF client_print
#include "modules/utils/clientprint/Uptime.h"
#include "modules/utils/clientprint/ClientPrint.h"
#include "SerialConsole.h"
// END MODIF client_print
// BEGIN MODIF show_print
#include "Robot.h"
#include "modules/utils/showprint/ShowPrint.h"
// END MODIF show_print
// BEGIN MODIF notify_sdcard_not_present
#include "CriticalErrorNotification.h"
// END MODIF notify_sdcard_not_present
// BEGIN MODIF pid_autotune_screen
#ifndef NO_UTILS_PIDTEST
#include "PIDTest.h"
#endif // NO_UTILS_PIDTEST
// END MODIF pid_autotune_screen
// BEGIN MODIF fast_beginning
#include "SlowTicker.h"
// END MODIF fast_beginning
// BEGIN MODIF printer_mode
#include "PrinterMode.h"
// END MODIF printer_mode
// BEGIN MODIF tasks
#include "Tasks.h"
// END MODIF tasks
// BEGIN MODIF turn_fan_off
#include "modules/utils/misc/MiscModule.h"
// END MODIF turn_fan_off
// BEGIN MODIF safety_alert
#include "modules/tools/safetyalert/SafetyAlert.h"
// END MODIF safety_alert
#include "modules/tools/zprobe/ZProbe.h"
#include "modules/tools/scaracal/SCARAcal.h"
#include "modules/tools/switch/SwitchPool.h"
#include "modules/tools/temperatureswitch/TemperatureSwitch.h"
#include "modules/tools/drillingcycles/Drillingcycles.h"
#include "FilamentDetector.h"

#include "modules/robot/Conveyor.h"
#include "modules/utils/simpleshell/SimpleShell.h"
#include "modules/utils/configurator/Configurator.h"
#include "modules/utils/currentcontrol/CurrentControl.h"
#include "modules/utils/player/Player.h"
#include "modules/utils/pausebutton/PauseButton.h"
#include "modules/utils/PlayLed/PlayLed.h"
#include "modules/utils/panel/Panel.h"
#include "libs/Network/uip/Network.h"
// BEGIN MODIF proxy
#include "modules/utils/netutils/HttpProxyMod.h"
// END MODIF proxy
// BEGIN MODIF gcode_metadata
//#include "modules/utils/gcodemetadata/GcodeMetadata.h"
// END MODIF gcode_metadata
#include "Config.h"
#include "checksumm.h"
#include "ConfigValue.h"
#include "StepTicker.h"

// #include "libs/ChaNFSSD/SDFileSystem.h"
#include "libs/nuts_bolts.h"
#include "libs/utils.h"

// Debug
#include "libs/SerialMessage.h"

#include "libs/USBDevice/USB.h"
#include "libs/USBDevice/USBMSD/USBMSD.h"
#include "libs/USBDevice/USBMSD/SDCard.h"
#include "libs/USBDevice/USBSerial/USBSerial.h"
#include "libs/USBDevice/DFU.h"
#include "libs/SDFAT.h"
#include "StreamOutputPool.h"
#include "ToolManager.h"

#include "libs/Watchdog.h"

#include "version.h"
// BEGIN MODIF firmware version
#include "FirmwareVersion.h"
// END MODIF firmware version
#include "system_LPC17xx.h"

#include "mbed.h"

#define second_usb_serial_enable_checksum  CHECKSUM("second_usb_serial_enable")
#define disable_msd_checksum  CHECKSUM("msd_disable")
#define dfu_enable_checksum  CHECKSUM("dfu_enable")
// BEGIN MODIF lcd
#include "modules/utils/lcd/4dsystemslcd/LCD4DModule.h"
SerialConsoleBase* lcd_serial;

#if !LCD_DISPLAY_SERIAL
#include "NonexecutableSerialConsole.h"
#endif //LCD_DISPLAY_SERIAL
// END MODIF lcd

// Watchdog wd(5000000, WDT_MRI);

// USB Stuff
SDCard sd  __attribute__ ((section ("AHBSRAM0"))) (P0_9, P0_8, P0_7, P0_6);      // this selects SPI1 as the sdcard as it is on Smoothieboard
//SDCard sd(P0_18, P0_17, P0_15, P0_16);  // this selects SPI0 as the sdcard
//SDCard sd(P0_18, P0_17, P0_15, P2_8);  // this selects SPI0 as the sdcard witrh a different sd select

// BEGIN MODIF external sdcard
#if SPLIT_CONFIG_AND_PUBLIC_SD
#include "MemFakeDisk.h"
#include "DualDisk.h"

    // sd2 uses SPI1, located next to the ARM micro controller in the smoothie board.
    // Visit http://chibidibidiwah.wdfiles.com/local--files/smoothieboard/smoothie_wiring_diagram.png
    // to know how each pin must be connected.
    SDCard real_sd2 __attribute__ ((section ("AHBSRAM0"))) (P0_18, P0_17, P0_15, P0_16);
    MemFakeDisk fake_sd2 __attribute__ ((section ("AHBSRAM0")));
    DualDisk sd2 __attribute__ ((section ("AHBSRAM0")))(&real_sd2, &fake_sd2);
#endif
// END MODIF external sdcard

USB u __attribute__ ((section ("AHBSRAM0")));
USBSerial usbserial __attribute__ ((section ("AHBSRAM0"))) (&u);
#ifndef DISABLEMSD
// BEGIN MODIF external sdcard
#if SPLIT_CONFIG_AND_PUBLIC_SD
    // publish sd2 instead of sd (don't give access to the configuration file)
    USBMSD msc __attribute__ ((section ("AHBSRAM0"))) (&u, &sd2);
#else
    USBMSD msc __attribute__ ((section ("AHBSRAM0"))) (&u, &sd);
#endif
// END MODIF external sdcard
#else
USBMSD *msc= NULL;
#endif

// BEGIN MODIF external sdcard
SDFAT mounter __attribute__ ((section ("AHBSRAM0"))) (CONFIG_SD_MOUNT_DIR, &sd);
#if SPLIT_CONFIG_AND_PUBLIC_SD
    // This second SD card will be mounted at /sd2. However, mounting it is deferred, because this
    // card is removable.
    // Instead of mounting it here, this file system will be mounted in these cases:
    // * In function init(), if and only if the SD card was correctly initialized.
    // * If the user runs the command M21 and the SD card is present (see SDCardInsert.cpp).
    // When the SD card file system is not mounted, mounter_sd2 = nullptr.
    // This file system is unmounted when calling M22 only (see SDCardInsert.cpp).
    SDFAT* mounter_sd2 = nullptr;
#endif
// END MODIF external sdcard

GPIO leds[5] = {
    GPIO(P1_18),
    GPIO(P1_19),
    GPIO(P1_20),
    GPIO(P1_21),
    GPIO(P4_28)
};

// debug pins, only used if defined in src/makefile
#ifdef STEPTICKER_DEBUG_PIN
GPIO stepticker_debug_pin(STEPTICKER_DEBUG_PIN);
#endif

void init() {

    // Default pins to low status
    for (int i = 0; i < 5; i++){
        leds[i].output();
        leds[i]= 0;
    }

#ifdef STEPTICKER_DEBUG_PIN
    stepticker_debug_pin.output();
    stepticker_debug_pin= 0;
#endif

    Kernel* kernel = new Kernel();

    kernel->streams->printf("Smoothie Running @%ldMHz\r\n", SystemCoreClock / 1000000);
    Version version;
    kernel->streams->printf("  Build version %s, Build date %s\r\n", version.get_build(), version.get_build_date());

    bool sdok= (sd.disk_initialize() == 0);
    if(!sdok) kernel->streams->printf("SDCard is disabled\r\n");

#ifdef DISABLEMSD
    // attempt to be able to disable msd in config
    if(sdok && !kernel->config->value( disable_msd_checksum )->by_default(false)->as_bool()){
        // HACK to zero the memory USBMSD uses as it and its objects seem to not initialize properly in the ctor
        size_t n= sizeof(USBMSD);
        void *v = AHB0.alloc(n);
        memset(v, 0, n); // clear the allocated memory
        // BEGIN MODIF external sdcard
        #if SPLIT_CONFIG_AND_PUBLIC_SD
            msc= new(v) USBMSD(&u, &sd); // allocate object using zeroed memory
        #else
            msc= new(v) USBMSD(&u, &sd2); // allocate object using zeroed memory
        #endif
        // END MODIF external sdcard
    }else{
        msc= NULL;
        kernel->streams->printf("MSD is disabled\r\n");
    }
#endif

    // BEGIN MODIF external sdcard
    #if SPLIT_CONFIG_AND_PUBLIC_SD
        bool sd2ok= (sd2.disk_initialize() == 0);
        if (!sd2ok) {
            kernel->streams->printf("  Failed to initialize sd card 2.");
        } else {
            // create and register fat file system
            mounter_sd2 = new SDFAT(PUBLIC_SD_MOUNT_DIR, &sd2);
        }
    #endif
    // END MODIF external sdcard

    // Create and add main modules
    // BEGIN MODIF client_print
    if (!kernel->robot->dimensions_are_valid()) {
    	kernel->add_module( CriticalErrorNotification::instance_invalid_dimensions_error());
    }
    kernel->add_module( new UptimeImpl() );
    kernel->add_module( new ClientPrint() );
    // END MODIF client_print
    kernel->add_module( new SimpleShell() );
    kernel->add_module( new Configurator() );
    kernel->add_module( new CurrentControl() );
// BEGIN MODIF no_pause_button
#ifndef NO_UTILS_PAUSEBUTTON
    kernel->add_module( new PauseButton() );
#endif // NO_UTILS_PAUSEBUTTON
// END MODIF no_pause_button
    kernel->add_module( new PlayLed() );
    // BEGIN MODIF autolevel
    Endstops* e = new Endstops();
    kernel->add_module( e );
    // END MODIF autolevel
    // BEGIN MODIF filament
    kernel->add_module( new EndOfFilament() );
// #ifndef NO_TOOLS_REPETIERHOSTNOTIFICATION
//     kernel->add_module( new RepetierHostNotification() );
// #endif // NO_TOOLS_REPETIERHOSTNOTIFICATION
    kernel->add_module( new FilamentTemps());
    // END MODIF filament
    kernel->add_module( new Player() );
    // BEGIN MODIF fat
    kernel->add_module( new SDCardInsert() );
    // END MODIF fat
    // BEGIN MODIF serial number
    kernel->add_module( new SerialNumber() );
    // END MODIF serial number
    // BEGIN MODIF status report
#ifndef NO_TOOLS_STATUSREPORT
    // Most of the time this module is not necessary and uses more than 2kb of
    // flash, so, disabling it is a wise decision. It could be useful for
    // diagnosing problems.
    kernel->add_module( new StatusReport() );
#endif // NO_TOOLS_STATUSREPORT
    // END MODIF status report
    // BEGIN MODIF firmware version
    kernel->add_module( new FirmwareVersion() );
    // END MODIF firmware version
    // BEGIN MODIF emergency_stop
//    kernel->add_module( new EmergencyStop() );
    // END MODIF emergency_stop
    // BEGIN MODIF notify_sdcard_not_present
#ifndef NO_UTILS_LCD
    if (!sdok){
        kernel->add_module( CriticalErrorNotification::instance_missing_sd_card_error() );
    }
#endif // NO_UTILS_LCD
    // END MODIF notify_sdcard_not_present
    // BEGIN MODIF pid_autotune_screen
#ifndef NO_UTILS_PIDTEST
    kernel->add_module(new PIDTest());
#endif// NO_UTILS_PIDTEST
    // END MODIF pid_autotune_screen
    // BEGIN MODIF printer_mode
    kernel->add_module(new PrinterMode());
    // END MODIF printer_mode
    // BEGIN MODIF tasks
    kernel->add_module(new Tasks());
    // END MODIF tasks
    // BEGIN MODIF turn_fan_off
    kernel->add_module(new MiscModule());
    // END MODIF turn_fan_off


    // these modules can be completely disabled in the Makefile by adding to EXCLUDE_MODULES
    #ifndef NO_TOOLS_SWITCH
    SwitchPool *sp= new SwitchPool();
    sp->load_tools();
    delete sp;
    #endif
    #ifndef NO_TOOLS_EXTRUDER
    // NOTE this must be done first before Temperature control so ToolManager can handle Tn before temperaturecontrol module does
    ExtruderMaker *em= new ExtruderMaker();
    em->load_tools();
    delete em;
    #endif
    #ifndef NO_TOOLS_TEMPERATURECONTROL
    // Note order is important here must be after extruder so Tn as a parameter will get executed first
    TemperatureControlPool *tp= new TemperatureControlPool();
    tp->load_tools();
    delete tp;
    #endif
    #ifndef NO_TOOLS_LASER
    kernel->add_module( new Laser() );
    #endif
    #ifndef NO_TOOLS_SPINDLE
    kernel->add_module( new Spindle() );
    #endif
    #ifndef NO_UTILS_PANEL
    kernel->add_module( new Panel() );
    #endif
    /*#ifndef NO_TOOLS_TOUCHPROBE
    kernel->add_module( new Touchprobe() );
    #endif*/
    #ifndef NO_TOOLS_ZPROBE
    kernel->add_module( new ZProbe() );
    #endif
    #ifndef NO_TOOLS_SCARACAL
    kernel->add_module( new SCARAcal() );
    #endif
    #ifndef NONETWORK
    kernel->add_module( new Network() );
    // BEGIN MODIF proxy
    kernel->add_module( new HttpProxyMod() );
    // END MODIF proxy
    #endif
    // BEGIN MODIF gcode_metadata
//    kernel->add_module( new GcodeMetadata() );
    // END MODIF gcode_metadata
    #ifndef NO_TOOLS_TEMPERATURESWITCH
    // Must be loaded after TemperatureControl
    kernel->add_module( new TemperatureSwitch() );
    #endif
    #ifndef NO_TOOLS_DRILLINGCYCLES
    kernel->add_module( new Drillingcycles() );
    #endif
    #ifndef NO_TOOLS_FILAMENTDETECTOR
    kernel->add_module( new FilamentDetector() );
    #endif
    // BEGIN MODIF lcd
#ifndef NO_UTILS_LCD
#if LCD_DISPLAY_SERIAL
    lcd_serial = THEKERNEL->serial;
#else
    lcd_serial = new NonexecutableSerialConsole(LCD_TX_PIN, LCD_RX_PIN, LCD_INIT_BAUDRATE);
    kernel->add_module(lcd_serial);
#endif  // LCD_DISPLAY_SERIAL
    kernel->add_module( new LCD4DModule() );
#endif // NO_UTILS_LCD
    // END MODIF lcd
    // BEGIN MODIF filament
    // Printer parking is added after LCD4DModule, so that on_out_of_filament event is processed later.
    kernel->add_module( new PrinterParking() );
    // END MODIF filament
    // BEGIN MODIF show_print
    // Leaving ShowPrint before LCD4DModule or PrinterParking (I haven't verified which one yet)
    // causes it to stop working.
    kernel->add_module( new ShowPrint() );
    // END MODIF show_print
    // BEGIN MODIF safety_alert
	kernel->add_module(new SafetyAlert());
	// END MODIF safety_alert
    // Create and initialize USB stuff
    u.init();

#ifdef DISABLEMSD
    if(sdok && msc != NULL){
        kernel->add_module( msc );
    }
#else
    kernel->add_module( &msc );
#endif

    kernel->add_module( &usbserial );
    if( kernel->config->value( second_usb_serial_enable_checksum )->by_default(false)->as_bool() ){
        kernel->add_module( new(AHB0) USBSerial(&u) );
    }

    if( kernel->config->value( dfu_enable_checksum )->by_default(false)->as_bool() ){
        kernel->add_module( new(AHB0) DFU(&u));
    }
    kernel->add_module( &u );

    // BEGIN MODIF config_cache_preallocation
    printf("ConfigCache size: %d (current CONFIG_CACHE_PREALLOCATION=%d)\n", kernel->config->config_cache_size(), CONFIG_CACHE_PREALLOCATION);
    if (kernel->config->config_cache_size() > CONFIG_CACHE_PREALLOCATION) {
    	printf("WARNING: ConfigCache > CONFIG_CACHE_PREALLOCATION. Memory reallocation might occur as a consequence of this.\n");
    } else if (kernel->config->config_cache_size() < CONFIG_CACHE_PREALLOCATION / 2) {
    	printf("WARNING: Less than half of the CONFIG_CACHE_PREALLOCATION is used. It might be a waste of RAM.\n");
    }
    // END MODIF config_cache_preallocation
    // clear up the config cache to save some memory
    kernel->config->config_cache_clear();

    if(kernel->is_using_leds()) {
        // set some leds to indicate status... led0 init doe, led1 mainloop running, led2 idle loop running, led3 sdcard ok
        leds[0]= 1; // indicate we are done with init
        leds[3]= sdok?1:0; // 4th led inidicates sdcard is available (TODO maye should indicate config was found)
    }

    if(sdok) {
        // load config override file if present
        // NOTE only Mxxx commands that set values should be put in this file. The file is generated by M500
        FILE *fp= fopen(kernel->config_override_filename(), "r");
        if(fp != NULL) {
            char buf[132];
            kernel->streams->printf("Loading config override file: %s...\n", kernel->config_override_filename());
            while(fgets(buf, sizeof buf, fp) != NULL) {
                kernel->streams->printf("  %s", buf);
                if(buf[0] == ';') continue; // skip the comments
                struct SerialMessage message= {&(StreamOutput::NullStream), buf};
                kernel->call_event(ON_CONSOLE_LINE_RECEIVED, &message);
            }
            kernel->streams->printf("config override file executed\n");
            fclose(fp);
        }
        // BEGIN MODIF second_config_file
        fp= fopen(kernel->config_priv_filename(), "r");
        if(fp != NULL) {
            char buf[132];
            kernel->streams->printf("Loading config private override file: %s...\n", kernel->config_priv_filename());
            while(fgets(buf, sizeof buf, fp) != NULL) {
                kernel->streams->printf("  %s", buf);
                if(buf[0] == ';') continue; // skip the comments
                struct SerialMessage message= {&(StreamOutput::NullStream), buf};
                kernel->call_event(ON_CONSOLE_LINE_RECEIVED, &message);
            }
            kernel->streams->printf("config private override file executed\n");
            fclose(fp);
        }
        // END MODIF second_config_file
    }

    // BEGIN MODIF fast_beginning
    // Now reset the counter. At this moment the accumulated seconds so far will raise one event
    // each. This will make different parts of the firmware believe that just a few processor ticks
    // are really a lot of seconds.
    kernel->slow_ticker->reset_counter();
    // END MODIF fast_beginning
    THEKERNEL->step_ticker->start();
}

int main()
{
    init();

    uint16_t cnt= 0;
    // Main loop
    while(1){
        if(THEKERNEL->is_using_leds()) {
            // flash led 2 to show we are alive
            leds[1]= (cnt++ & 0x1000) ? 1 : 0;
        }
        THEKERNEL->call_event(ON_MAIN_LOOP);
        THEKERNEL->call_event(ON_IDLE);
        // BEGIN MODIF lcd
        THEKERNEL->process_display_gcode_queue();
        // END MODIF lcd
    }
}

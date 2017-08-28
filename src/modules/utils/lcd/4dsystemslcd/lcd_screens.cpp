/*
 * lcd_screens.cpp
 *
 *  Created on: Sep 3, 2015
 *      Author: eai
 */

#include "lcd_screens.h"
#include <cmsis.h>

screens_available_s lcd_screens;

#define NUMBER_OF_SCREENS (sizeof(screens_available_s)/sizeof(DltPtr<LCD4DScreen>))

static int lcd_counters[NUMBER_OF_SCREENS];
static int fake_counter; // for cases when screens are not found
static bool persistent[NUMBER_OF_SCREENS];

static int get_screen_index(LCD4DScreen* s) {
	DltPtr<LCD4DScreen>* screens_array = (DltPtr<LCD4DScreen>*)&lcd_screens;
	for (unsigned int i = 0; i < NUMBER_OF_SCREENS; i++){
		if (screens_array[i].is_ptr(s)) {
			return i;
		}
	}
	return -1;
}
template<class T>
static int get_screen_index(DltPtr<T>& s) {
	DltPtr<LCD4DScreen>* screens_array = (DltPtr<LCD4DScreen>*)&lcd_screens;
	for (unsigned int i = 0; i < NUMBER_OF_SCREENS; i++){
		if (&(screens_array[i]) == (DltPtr<LCD4DScreen>*)&s) {
			return i;
		}
	}
	return -1;
}
template<class T>
static void make_persistent(DltPtr<T>& s){
	int screen_index = get_screen_index(s);
	if (screen_index >= 0){
		persistent[screen_index] = true;
	} else {
		// FIXME this mustn't happen. If it happens, inform it!!
	}
}

void lcd_initialize_counters(){
	for (unsigned int i = 0; i < NUMBER_OF_SCREENS; i++){
		lcd_counters[i] = 0;
		persistent[i] = false;
	}
	// Now mark persistent windows as persistent
	// The main screen is used all the time, so deleting it is a waste of time.
	// Change_filament_screen can be called from a timer event, so we need to have it loaded so
	// that no dynamic memory operation is performed. As it is a LCD4DScreenWithBackButton, no
	// window will be destroyed while drawing it.
	make_persistent(lcd_screens.change_filament_screen);
	// The pwd attribute can be used even after the screen disappears, so, it's better to keep it
	// in memory.
	make_persistent(lcd_screens.select_model_screen);
	make_persistent(lcd_screens.main_screen);
}

void lcd_clear_all_screens_except(LCD4DScreen* s){
	// let's make some magic :-P

	// Fist of all, check we are not in an interrupt handler. In that case we won't be able to use
	// dynamic memory.
	if (__get_IPSR() != 0) return;

	// Then treat the struct as an array with len=sizeof(struct)/sizeof(ptr)
	// Cast the element as a class that has the same structure. Let's hope this will always work :-) .
	DltPtr<LCD4DScreen>* screens_array = (DltPtr<LCD4DScreen>*)&lcd_screens;
	for (unsigned int i = 0; i < NUMBER_OF_SCREENS; i++){
		if (!persistent[i] && lcd_counters[i] <= 0 && !screens_array[i].is_ptr(s)) {
			// Delete it if possible.
			screens_array[i].flush_obj();
		}
	}
}

int* lcd_get_counter(LCD4DScreen* s) {
	int index = get_screen_index(s);
	if (index >= 0) {
		return &(lcd_counters[index]);
	} else {
		// Oh oh, this is wrong. A screen that exists but is not referenced here.
		// Let's use a counter that will never make the code free this code.
		fake_counter = 0x0000CAFE; // mark it differently
		return &fake_counter;
	}
}

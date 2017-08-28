#include "smoothie_arduino.h"

#include "modules/utils/clientprint/Uptime.h"
#include "wait_api.h"
#include "sLPC17xx.h"

unsigned long millis() {
	__disable_irq();
	unsigned long ret = uptime_millis();
	__enable_irq();

    return ret;
}

void delay(unsigned long tts) {
	wait_ms(tts);
}


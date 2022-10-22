#ifndef TTY2PICO_MCU_H
#define TTY2PICO_MCU_H

#include "SdSpiDriverT2P.h"

static void noop(void);

typedef enum DateTimeFormat {
	DTF_UNIX = 0,
	DTF_HUMAN,
} DateTimeFormat;

class MCU
{
public:
	MCU(SdSpiDriverT2P *sdSpiDriver) : sdSpiDriver(sdSpiDriver) { }

	// Pause processes on other core(s)
	void pauseBackground(void);

	// Resume processes on other core(s)
	void resumeBackground(void);

	// Get the current CPU speed
	float getCpuSpeedMHz(void);

	// Get the current CPU temp
	float getCpuTemperature(void);

	// Get the SD card configuration
	SdSpiConfig getSdSpiConfig(void);

	// Get the time from the RTC
	const char *getTime(int format);

	// Get the SPI rate for the display
	float getSpiRateDisplayMHz();

	// Get the SPI rate for the SD card
	float getSpiRateSdMHz();

	// Set the time on the RTC
	void setTime(uint32_t timestamp);

	// Reset the MCU for a firmware update
	void resetForUpdate(void);

	// Run configuration for the MCU
	void setup(void);

private:
	SdSpiDriverT2P *sdSpiDriver;
};

#endif

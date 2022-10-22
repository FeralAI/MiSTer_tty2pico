#include "config.h"
#include "MCU.h"
#include "SdSpiDriverT2P.h"
#include "hardware/rtc.h"
#include "hardware/spi.h"
#include "hardware/vreg.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include <UnixTime.h>

static char datetimeString[37]; // Max date string could be 37 chars: Wednesday 17 November 01:59:49 2022

/*******************************************************************************
 * platform.h functions
 *******************************************************************************/

static inline __attribute__((always_inline)) void noop(void)
{
	tight_loop_contents();
}

float MCU::getCpuSpeedMHz(void)
{
	return clock_get_hz(clk_sys) / 1000000.0f;
}

float MCU::getCpuTemperature(void)
{
	return analogReadTemp();
}

SdSpiConfig MCU::getSdSpiConfig(void)
{
	return SdSpiConfig(SDCARD_CS_PIN, DEDICATED_SPI, SPI_FULL_SPEED, &sdSpiDriver);
}

const char *MCU::getTime(int format)
{
	if (!rtc_running())
		return "RTC Disabled";

	datetime_t datetime;
	rtc_get_datetime(&datetime);
	memset(datetimeString, 0, sizeof(datetimeString));

	switch (format)
	{
		case DTF_HUMAN:
			datetime_to_str(datetimeString, sizeof(datetimeString), &datetime);
			return static_cast<const char *>(datetimeString);

		default:
			UnixTime unixTime(0);
			unixTime.setDateTime(datetime.year, datetime.month, datetime.day, datetime.hour, datetime.min, datetime.sec);
			String unixTimeString(unixTime.getUnix());
			memcpy(datetimeString, unixTimeString.c_str(), unixTimeString.length());
			return static_cast<const char *>(datetimeString);
	}
}

float MCU::getSpiRateDisplayMHz()
{
	uint rate = spi_get_baudrate(getDisplaySpi());
	return rate / 1000000.0f;
}

float MCU::getSpiRateSdMHz()
{
	uint rate = spi_get_baudrate(getSdSpi());
	return rate / 1000000.0f;
}

void MCU::setTime(uint32_t timestamp)
{
	if (!rtc_running())
		rtc_init();

	// Set the date and time
	UnixTime unixTime(0); // No timezone offset
	unixTime.getDateTime(timestamp);
	datetime_t datetime = {
		.year  = static_cast<int16_t>(unixTime.year),
		.month = static_cast<int8_t>(unixTime.month),
		.day   = static_cast<int8_t>(unixTime.day),
		.dotw  = static_cast<int8_t>(unixTime.dayOfWeek),
		.hour  = static_cast<int8_t>(unixTime.hour),
		.min   = static_cast<int8_t>(unixTime.minute),
		.sec   = static_cast<int8_t>(unixTime.second),
	};
	rtc_set_datetime(&datetime);
	delayMicroseconds(64); // Need to delay for 3 RTC clock cycles which is 64us

	// Validate RTC
	if (rtc_get_datetime(&datetime))
	{
		memset(datetimeString, 0, sizeof(datetimeString));
		datetime_to_str(datetimeString, sizeof(datetimeString), &datetime);
		Serial.print("RTC date and time set to "); Serial.println(datetimeString);
	}
	else
	{
		Serial.print("Invalid timestamp: "); Serial.println(timestamp);
	}
}

void MCU::resetForUpdate(void)
{
	reset_usb_boot(0, 0);
}

void MCU::setup(void)
{
	rp2040.enableDoubleResetBootloader();

	int cpuMHz = 0;
	bool syncPClk = false;

	switch (config.overclockMode)
	{
		case TTY2PICO_OverclockMode::OVERCLOCKED:
			cpuMHz = 250;
			syncPClk = true;
			break;

		case TTY2PICO_OverclockMode::LUDICROUS_SPEED:
			cpuMHz = 266;
			syncPClk = true;
			break;
	}

	if (cpuMHz)
	{
		// Apply an overclock for about 2x performance and a voltage tweak to stablize most RP2040 boards.
		// If it's good enough for pixel-pushing in MicroPython, it's good enough for us :P
		// https://github.com/micropython/micropython/issues/8208
		vreg_set_voltage(VREG_VOLTAGE_1_20); // Set voltage to 1.2v
		delay(10); // Allow vreg time to stabilize
		set_sys_clock_khz(cpuMHz * 1000, true);
		Serial.println("CPU overclocked to "); Serial.print(cpuMHz); Serial.println("MHz");
	}

	if (syncPClk)
	{
		// Sync peripheral clock to CPU clock to get a huge boost to SPI performance, mostly for SD transfers
		uint32_t freq = clock_get_hz(clk_sys);
		clock_configure(clk_peri, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS, freq, freq);
		Serial.println("Peripheral bus overclock applied");
	}

	// Manually apply SPI frequencies so they're picked up and reported correctly, even if not overclocking
	spi_set_baudrate(getSdSpi(), SPI_FULL_SPEED);
	spi_set_baudrate(getDisplaySpi(), SPI_FREQUENCY);
	Serial.println("SPI baud rates set");

	Serial.println("Platform setup complete");
}

inline void MCU::pauseBackground(void)
{
	noInterrupts();
	rp2040.idleOtherCore();
}

inline void MCU::resumeBackground(void)
{
	rp2040.resumeOtherCore();
	interrupts();
}

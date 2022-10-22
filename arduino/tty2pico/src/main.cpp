/*******************************************************************************
 * tty2pico for Arduino
 *******************************************************************************/

// Configuration overrides - See "config.h" for more details
// #define WAIT_FOR_SERIAL 1
#define VERBOSE_OUTPUT 1
// #define SHOW_FPS 1
// #define USE_GIF_BUFFERING

#include "config.h"
#include <Arduino.h>
#include "TTY.h"
#include "SdSpiDriverT2P.h"
#include "MCU.h"
#include "Commands.h"
#include "storage.h"
#include "usbmsc.h"
#include "display.h"

TTY tty;
SdSpiDriverT2P sdSpiDriver;
MCU mcu(&sdSpiDriver);
Commander cmd(&mcu);

#define POLLING_LOOP_DELAY 500

void setup()
{
	/* NOTE: Most of these setup functions need to run in a particular order!!! */

	beginUsbMsc();    // Start up USB MSC interface, must be BEFORE the serial interface so CDC doesn't take over
	tty.setup();      // Bring up the serial interface
	setupStorage();   // Configure storage
	readyUsbMsc();    // Set USB MSC ready after storage is available
	mcu.setup();      // Apply platform-specific code for the MCU (tune bus speed, overclock, etc.)
	setupDisplay();   // Configure and enable the display
	cmd.setupQueue(); // Set up task queue

	setDirectory(config.imagePath); // Set the working image path
	showStartup();
}

void loop()
{
	static String command;
	static CommandData data;
	static uint32_t nextRead;
	static uint32_t now;

	now = millis();

	if (millis() - nextRead > 0)
	{
		command = tty.read();
		nextRead = millis() + POLLING_LOOP_DELAY; // Delay the next read for better performance

		if (command != "")
		{
			data = cmd.process(command);
			cmd.loopQueue();
		}
	}

	loopDisplay(now);

	delay(0); // Ever so slight performance gain...I think?
}

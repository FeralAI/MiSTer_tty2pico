#include "config.h"
#include "Commands.h"
#include "MCU.h"
#include "display.h"
#include "usbmsc.h"

using namespace std;

static String coreName;
static queue_t cmdQ;

void Commander::cmdBye(void)
{
	showMister();
}

void Commander::cmdCls(void)
{
	clearDisplay();
}

void Commander::cmdDisplayOff(void)
{
	digitalWrite(TFT_BL, LOW);
}

void Commander::cmdDisplayOn(void)
{
	digitalWrite(TFT_BL, HIGH);
}

void Commander::cmdEnableOTA()
{
	Serial.println("Restarting in firmware update mode");
	mcu.resetForUpdate();
}

void Commander::cmdGetTime(String command)
{
	int format = DTF_UNIX;
	if (command.indexOf(',') > 0)
	{
		String mode = command.substring(command.indexOf(',') + 1, command.indexOf(',') + 2);
		if (mode == "1")
			format = DTF_HUMAN;
	}

	const char *time = mcu.getTime(format);
	Serial.println(time);
}

void Commander::cmdGetSysInfo()
{
	string info = string("version:")  + string(TTY2PICO_VERSION_STRING)
	            + string("|board:")   + string(TTY2PICO_BOARD)
	            + string("|display:") + string(TTY2PICO_DISPLAY);

	Serial.println(info.c_str());
}

void Commander::cmdRotate(String command)
{
	if (command.indexOf(',') > -1)
	{
		String mode = command.substring(command.indexOf(',') + 1, command.indexOf(',') + 2);
		int rotationMode = -1;
		if (mode == "0")
			rotationMode = 0;
		else if (mode == "1")
			rotationMode = 2;
		else if (mode == "2")
			rotationMode = 1;
		else if (mode == "3")
			rotationMode = 3;

		if (rotationMode < 0)
		{
			Serial.print("Invalid rotation mode: "); Serial.println(mode);
			return;
		}

#ifdef TFT_ROTATION
		// Rotating needs to factor in the original rotation from TFT_ROTATION
		rotationMode = (rotationMode + TFT_ROTATION) % 4;
#endif
		tft.setRotation(rotationMode);
		config.tftRotation = rotationMode;
		saveConfig();
		Serial.print("Rotation changed to "); Serial.println(mode);
	}
	else Serial.println("No rotation command found");
}

void Commander::cmdSaver(String command)
{
	DisplayState lastDisplayState = getDisplayState();
	if (command.indexOf(',') > -1 && command.length() > 9)
	{
		String mode = command.substring(command.indexOf(',') + 1, command.indexOf(',') + 2);
		Serial.print("Screensaver mode changed to "); Serial.println(mode);
		bool enabled = mode != "0";
		if (enabled)
			setDisplayState(DISPLAY_SLIDESHOW);
		if (!enabled && lastDisplayState != DISPLAY_SLIDESHOW)
			showStartup(); // Reset to startup image for now, might want to restore state later
	}
	else
	{
		setDisplayState(DISPLAY_SLIDESHOW);
	}
}

void Commander::cmdSetCore(String command)
{
	if (command.startsWith(CMDCORE))
		coreName = command.substring(command.indexOf(',') + 1, command.length());
	else
		coreName = command;

	String path;
	bool found = false;
	for (int i = 0; i < imageExtensionCount; i++)
	{
		// Check for animated file(s) first
		path = config.imagePath + coreName + String(imageExtensions[i]);
#if VERBOSE_OUTPUT == 1
	Serial.print("Checking for file "); Serial.println(path);
#endif
		found = fileExists(path);
		if (found)
			break;
	}

	if (found)
	{
		Serial.print("Loading "); Serial.println(path.c_str());
		showImage(path);
	}
	else
	{
		Serial.print("Couldn't find core display file for "); Serial.println(coreName);
		showText(coreName);
	}
}

void Commander::cmdSetTime(String command)
{
	if (command.indexOf(",") > 0)
	{
		String unixTimestamp = command.substring(command.indexOf(",") + 1);
		uint32_t timestamp = unixTimestamp.toInt();
		mcu.setTime(timestamp);
	}
	else Serial.println("Cannot set date and time, no data received");
}

void Commander::cmdShow(String command)
{
	String path = command.substring(command.indexOf(',') + 1);
	showImage(path);
}

void Commander::cmdShowCoreName(void)
{
	showText(coreName);
}

void Commander::cmdShowSystemInfo(void)
{
	showSystemInfo(millis());
}

void Commander::cmdTest(void)
{
	showText("Starting test in..."); delay(2000);
	showText("3"); delay(1000);
	showText("2"); delay(1000);
	showText("1"); delay(1000);
	showSystemInfo(millis());
	delay(3000);
	for (int i = 0; i < 10; i++)
		showGIF((uint8_t *)mister_kun_blink, sizeof(mister_kun_blink));
	drawDemoShapes(5000);
	if (fileExists(config.startupImage))
		showImage(config.startupImage);
	showText("Test complete!");
	delay(3000);
}

void Commander::cmdText(String command)
{
	String displayText;
	if (command.startsWith(CMDTXT))
		displayText = command.substring(command.indexOf(',') + 1, command.length());
	else
		displayText = command;

	showText(displayText);
}

void Commander::cmdUnknown(String command)
{
#if VERBOSE_OUTPUT == 1
	Serial.print("Received unknown command: "); Serial.println(command);
#endif
	showText(command);
}

void Commander::cmdUsbMsc()
{
	if (!getMscReady())
	{
		showText("USB MSC mode enabled");
		readyUsbMsc();
	}
}

void Commander::setupQueue(void)
{
	queue_init(&cmdQ, sizeof(CommandData), 1);
}

void Commander::addToQueue(CommandData &data)
{
	queue_try_add(&cmdQ, &data);
}

bool Commander::removeFromQueue(CommandData &data)
{
	return queue_try_remove(&cmdQ, &data);
}

void Commander::loopQueue(void)
{
	static CommandData data;

	while (removeFromQueue(data))
	{
		if (data.command == TTY2CMD_COR || data.command == TTY2CMD_SHOW)
			clearDisplay();

		runCommand(data);
	}
}

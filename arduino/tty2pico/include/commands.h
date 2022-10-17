#ifndef COMMANDS_H
#define COMMANDS_H

#include "config.h"
#include <string>
#include "definitions.h"
#include "platform.h"
#include "display.h"
#include "storage.h"
#include "usbmsc.h"

using namespace std;

static String coreName;

static void cmdBye(void)
{
	showMister();
}

static void cmdCls(void)
{
	clearDisplay();
}

static void cmdDisplayOff(void)
{
	digitalWrite(TFT_BL, LOW);
}

static void cmdDisplayOn(void)
{
	digitalWrite(TFT_BL, HIGH);
}

static void cmdEnableOTA()
{
	Serial.println("Restarting in firmware update mode");
	resetForUpdate();
}

static void cmdGetTime(String command)
{
	int format = DTF_UNIX;
	if (command.indexOf(',') > 0)
	{
		String mode = command.substring(command.indexOf(',') + 1, command.indexOf(',') + 2);
		if (mode == "1")
			format = DTF_HUMAN;
	}

	char *time = getTime(format);
	Serial.println(time);
}

static void cmdGetSysInfo()
{
	string info = string("version:")  + string(TTY2PICO_VERSION)
	            + string("|board:")   + string(TTY2PICO_BOARD)
	            + string("|display:") + string(TTY2PICO_DISPLAY);

	Serial.println(info.c_str());
}

static void cmdRotate(String command)
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

static void cmdSaver(String command)
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

static void cmdSetCore(String command)
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

static void cmdSetTime(String command)
{
	if (command.indexOf(",") > 0)
	{
		String unixTimestamp = command.substring(command.indexOf(",") + 1);
		uint32_t timestamp = unixTimestamp.toInt();
		setTime(timestamp);
	}
	else Serial.println("Cannot set date and time, no data received");
}

static void cmdShow(String command)
{
	String path = command.substring(command.indexOf(',') + 1);
	showImage(path);
}

static void cmdShowCoreName(void)
{
	showText(coreName);
}

static void cmdShowSystemInfo(void)
{
	showSystemInfo();
}

static void cmdTest(void)
{
	showText("Starting test in..."); delay(2000);
	showText("3"); delay(1000);
	showText("2"); delay(1000);
	showText("1"); delay(1000);
	showSystemInfo();
	delay(3000);
	showGIF((uint8_t *)mister_kun_blink, sizeof(mister_kun_blink));
	drawDemoShapes(5000);
	showText("Test complete!");
	delay(3000);
}

static void cmdText(String command)
{
	String displayText;
	if (command.startsWith(CMDTXT))
		displayText = command.substring(command.indexOf(',') + 1, command.length());
	else
		displayText = command;

	showText(displayText);
}

static void cmdUnknown(String command)
{
#if VERBOSE_OUTPUT == 1
	Serial.print("Received unknown command: "); Serial.println(command);
#endif
	showText(command);
}

static void cmdUsbMsc()
{
	if (!getMscReady())
	{
		showText("Starting USB MSC mode, this could take a while for SD...");
		readyUsbMsc();
	}
}

CommandData parseCommand(String command)
{
	if (command != "")
	{
		if      (command.startsWith(CMDBYE))                                  return CommandData(TTY2CMD_BYE, command);
		else if (command.startsWith(CMDCLS))                                  return CommandData(TTY2CMD_CLS, command);
		else if (command.startsWith(CMDDOFF))                                 return CommandData(TTY2CMD_DOFF, command);
		else if (command.startsWith(CMDDON))                                  return CommandData(TTY2CMD_DON, command);
		else if (command.startsWith(CMDENOTA))                                return CommandData(TTY2CMD_ENOTA, command);
		else if (command.startsWith(CMDGETSYS))                               return CommandData(TTY2CMD_GETSYS, command);
		else if (command.startsWith(CMDGETTIME))                              return CommandData(TTY2CMD_GETTIME, command);
		else if (command.startsWith(CMDROT))                                  return CommandData(TTY2CMD_ROT, command);
		else if (command.startsWith(CMDSAVER))                                return CommandData(TTY2CMD_SAVER, command);
		else if (command.startsWith(CMDSETTIME))                              return CommandData(TTY2CMD_SETTIME, command);
		else if (command.startsWith(CMDSHOW))                                 return CommandData(TTY2CMD_SHOW, command);
		else if (command.startsWith(CMDSHSYSHW))                              return CommandData(TTY2CMD_SHSYSHW, command);
		else if (command.startsWith(CMDSHTEMP))                               return CommandData(TTY2CMD_SHTEMP, command);
		else if (command.startsWith(CMDSNAM))                                 return CommandData(TTY2CMD_SNAM, command);
		else if (command.startsWith(CMDSORG))                                 return CommandData(TTY2CMD_SORG, command);
		else if (command.startsWith(CMDSWSAVER))                              return CommandData(TTY2CMD_SWSAVER, command);
		else if (command.startsWith(CMDTEST))                                 return CommandData(TTY2CMD_TEST, command);
		else if (command.startsWith(CMDTXT))                                  return CommandData(TTY2CMD_TXT, command);
		else if (command.startsWith(CMDUSBMSC))                               return CommandData(TTY2CMD_USBMSC, command);
		else if (command.startsWith("CMD") && !command.startsWith(CMDCORE))   return CommandData(TTY2CMD_UNKNOWN, command);
		else    /* Assume core name if no command was matched */              return CommandData(TTY2CMD_COR, command);
	}

	return CommandData(TTY2CMD_NONE, command);
}

void runCommand(CommandData data)
{
	switch (data.command)
	{
		case TTY2CMD_BYE:     return cmdBye();
		case TTY2CMD_CLS:     return cmdCls();
		case TTY2CMD_COR:     return cmdSetCore(data.commandText);
		case TTY2CMD_DOFF:    return cmdDisplayOff();
		case TTY2CMD_DON:     return cmdDisplayOn();
		case TTY2CMD_ENOTA:   return cmdEnableOTA();
		case TTY2CMD_GETSYS:  return cmdGetSysInfo();
		case TTY2CMD_GETTIME: return cmdGetTime(data.commandText);
		case TTY2CMD_ROT:     return cmdRotate(data.commandText);
		case TTY2CMD_SAVER:   return cmdSaver(data.commandText);
		case TTY2CMD_SETTIME: return cmdSetTime(data.commandText);
		case TTY2CMD_SHSYSHW: return cmdShowSystemInfo();
		case TTY2CMD_SWSAVER: return cmdSaver(data.commandText);
		case TTY2CMD_SHOW:    return cmdShow(data.commandText);
		case TTY2CMD_SHTEMP:  return cmdShowSystemInfo();
		case TTY2CMD_SNAM:    return cmdShowCoreName();
		case TTY2CMD_SORG:    return cmdShowSystemInfo();
		case TTY2CMD_TEST:    return cmdTest();
		case TTY2CMD_TXT:     return cmdText(data.commandText);
		case TTY2CMD_UNKNOWN: return cmdUnknown(data.commandText);
		case TTY2CMD_USBMSC:  return cmdUsbMsc();
		case TTY2CMD_NONE:    return;

		// If you get here you're missing an enum definition ^^^
		default:
			Serial.print("Unrecognized TTY2CMD command: ");
			Serial.println(data.command);
			return;
	}
}

void loopQueue(void)
{
	static CommandData data;

	while (removeFromQueue(data))
		runCommand(data);
}

#endif

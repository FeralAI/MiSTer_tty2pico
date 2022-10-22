#ifndef TTY2PICO_COMMANDS_H
#define TTY2PICO_COMMANDS_H

#include <Arduino.h>
#include "MCU.h"

// When adding a new command do the following:
// * Add a `const String [CMDNAME]` variable to commands.h
// * Add to TTY2CMD enum to commands.h
// * Add or use existing function for handling command, prefix function name with `cmd`
// * Add handling to `parseCommand()` and `runCommand()`
// * Document where appropriate

// tty2oled commands
const String CMDBYE     = "CMDBYE";
const String CMDCLS     = "CMDCLS";
const String CMDCORE    = "CMDCOR";
const String CMDDOFF    = "CMDDOFF";
const String CMDDON     = "CMDDON";
const String CMDENOTA   = "CMDENOTA";
const String CMDROT     = "CMDROT";
const String CMDSAVER   = "CMDSAVER";
const String CMDSETTIME = "CMDSETTIME";
const String CMDSHSYSHW = "CMDSHSYSHW";
const String CMDSHTEMP  = "CMDSHTEMP";
const String CMDSNAM    = "CMDSNAM";
const String CMDSORG    = "CMDSORG";
const String CMDSWSAVER = "CMDSWSAVER";
const String CMDTEST    = "CMDTEST";
const String CMDTXT     = "CMDTXT";

// tty2pico commands
const String CMDGETSYS  = "CMDGETSYS";
const String CMDGETTIME = "CMDGETTIME";
const String CMDSHOW    = "CMDSHOW";
const String CMDUSBMSC  = "CMDUSBMSC";

typedef enum TTY2CMD {
	TTY2CMD_NONE = 0,
	TTY2CMD_CLS,
	TTY2CMD_COR,
	TTY2CMD_BYE,
	TTY2CMD_DOFF,
	TTY2CMD_DON,
	TTY2CMD_ENOTA,
	TTY2CMD_GETSYS,
	TTY2CMD_GETTIME,
	TTY2CMD_ROT,
	TTY2CMD_SAVER,
	TTY2CMD_SETTIME,
	TTY2CMD_SHOW,
	TTY2CMD_SHSYSHW,
	TTY2CMD_SHTEMP,
	TTY2CMD_SNAM,
	TTY2CMD_SORG,
	TTY2CMD_SWSAVER,
	TTY2CMD_TEST,
	TTY2CMD_TXT,
	TTY2CMD_UNKNOWN,
	TTY2CMD_USBMSC,
} TTY2CMD;

class CommandData
{
public:
	CommandData() { }
	CommandData(TTY2CMD command) : command(command) { }
	CommandData(TTY2CMD command, String commandText) : command(command), commandText(commandText) { }

	TTY2CMD command;
	String commandText;
};

class Commander
{
public:
	Commander(MCU *mcu) : mcu(mcu) { }

	/* Command processing */
	CommandData process(String command)
	{
		CommandData data = parseCommand(command);
		if (data.command != TTY2CMD_NONE)
			addToQueue(data);

		return data;
	}

	CommandData parseCommand(String command)
	{
		String bigcmd = command;
		bigcmd.toUpperCase();

		if (bigcmd != "")
		{
			if      (bigcmd.startsWith(CMDBYE))                                  return CommandData(TTY2CMD_BYE, command);
			else if (bigcmd.startsWith(CMDCLS))                                  return CommandData(TTY2CMD_CLS, command);
			else if (bigcmd.startsWith(CMDDOFF))                                 return CommandData(TTY2CMD_DOFF, command);
			else if (bigcmd.startsWith(CMDDON))                                  return CommandData(TTY2CMD_DON, command);
			else if (bigcmd.startsWith(CMDENOTA))                                return CommandData(TTY2CMD_ENOTA, command);
			else if (bigcmd.startsWith(CMDGETSYS))                               return CommandData(TTY2CMD_GETSYS, command);
			else if (bigcmd.startsWith(CMDGETTIME))                              return CommandData(TTY2CMD_GETTIME, command);
			else if (bigcmd.startsWith(CMDROT))                                  return CommandData(TTY2CMD_ROT, command);
			else if (bigcmd.startsWith(CMDSAVER))                                return CommandData(TTY2CMD_SAVER, command);
			else if (bigcmd.startsWith(CMDSETTIME))                              return CommandData(TTY2CMD_SETTIME, command);
			else if (bigcmd.startsWith(CMDSHOW))                                 return CommandData(TTY2CMD_SHOW, command);
			else if (bigcmd.startsWith(CMDSHSYSHW))                              return CommandData(TTY2CMD_SHSYSHW, command);
			else if (bigcmd.startsWith(CMDSHTEMP))                               return CommandData(TTY2CMD_SHTEMP, command);
			else if (bigcmd.startsWith(CMDSNAM))                                 return CommandData(TTY2CMD_SNAM, command);
			else if (bigcmd.startsWith(CMDSORG))                                 return CommandData(TTY2CMD_SORG, command);
			else if (bigcmd.startsWith(CMDSWSAVER))                              return CommandData(TTY2CMD_SWSAVER, command);
			else if (bigcmd.startsWith(CMDTEST))                                 return CommandData(TTY2CMD_TEST, command);
			else if (bigcmd.startsWith(CMDTXT))                                  return CommandData(TTY2CMD_TXT, command);
			else if (bigcmd.startsWith(CMDUSBMSC))                               return CommandData(TTY2CMD_USBMSC, command);
			else if (bigcmd.startsWith("CMD") && !bigcmd.startsWith(CMDCORE))    return CommandData(TTY2CMD_UNKNOWN, command);
			else    /* Assume core name if no command was matched */             return CommandData(TTY2CMD_COR, command);
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

	/* Platform-specific queues */

	void setupQueue(void);
	void addToQueue(CommandData &data);
	bool removeFromQueue(CommandData &data);
	void loopQueue(void);

	/* Command actions */
	void cmdBye(void);
	void cmdCls(void);
	void cmdDisplayOff(void);
	void cmdDisplayOn(void);
	void cmdEnableOTA();
	void cmdGetTime(String command);
	void cmdGetSysInfo();
	void cmdRotate(String command);
	void cmdSaver(String command);
	void cmdSetCore(String command);
	void cmdSetTime(String command);
	void cmdShow(String command);
	void cmdShowCoreName(void);
	void cmdShowSystemInfo(void);
	void cmdTest(void);
	void cmdText(String command);
	void cmdUnknown(String command);
	void cmdUsbMsc();

private:
	MCU *mcu;
};

#endif

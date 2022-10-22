#include "config.h"
#include "TTY.h"

void TTY::setup()
{
	TTY_SERIAL.begin(config.ttyBaudRate);
	while (config.waitForSerial && !TTY_SERIAL)
		delay(0);

	TTY_SERIAL.println("Serial setup complete");
}

String TTY::read()
{
	String command;

	if (TTY_SERIAL.available())
	{
		yield();
		command = TTY_SERIAL.readStringUntil('\n');
		if (command.length() > 0)
		{
			if (command.endsWith("\r"))
				command = command.substring(0, command.length() - 1);

			TTY_SERIAL.print("Received Corename or Command: "); TTY_SERIAL.println(command.c_str());
		}
	}

	return command;
}

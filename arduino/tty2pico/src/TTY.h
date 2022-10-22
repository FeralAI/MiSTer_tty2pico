#ifndef TTY2PICO_TTY_H
#define TTY2PICO_TTY_H

#include <Arduino.h>

#ifndef TTY_SERIAL
#define TTY_SERIAL Serial
#endif

class TTY
{
public:
	void setup();
	String read();
};

#endif

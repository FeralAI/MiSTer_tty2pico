#ifndef TTY2PICO_SDSPIDRIVER_H
#define TTY2PICO_SDSPIDRIVER_H

#include "SPI.h"
#include "SpiDriver/SdSpiDriver.h"

inline spi_cpol_t get_cpol(SPIMode mode)
{
	switch (mode)
	{
		case SPI_MODE0:
			return SPI_CPOL_0;
		case SPI_MODE1:
			return SPI_CPOL_0;
		case SPI_MODE2:
			return SPI_CPOL_1;
		case SPI_MODE3:
			return SPI_CPOL_1;
	}

	// Error
	return SPI_CPOL_0;
}

inline spi_cpha_t get_cpha(SPIMode mode)
{
	switch (mode)
	{
		case SPI_MODE0:
			return SPI_CPHA_0;
		case SPI_MODE1:
			return SPI_CPHA_1;
		case SPI_MODE2:
			return SPI_CPHA_0;
		case SPI_MODE3:
			return SPI_CPHA_1;
	}

	// Error
	return SPI_CPHA_0;
}

inline spi_inst_t *getDisplaySpi(void)
{
	return TFT_SPI_PORT == 0 ? spi0 : spi1;
}

inline spi_inst_t *getSdSpi(void)
{
	return &SDCARD_SPI == &SPI ? spi0 : spi1;
}

class SdSpiDriverT2P : public SdSpiBaseClass
{
public:
	SdSpiDriverT2P();

	// Activate SPI hardware with correct speed and mode.
	void activate();

	// Initialize the SPI bus.
	void begin(SdSpiConfig config);

	// Deactivate SPI hardware.
	void deactivate();

	// Receive a byte.
	uint8_t receive();

	// Receive multiple bytes.
	uint8_t receive(uint8_t *buf, size_t count);

	// Send a byte.
	void send(uint8_t data);

	// Send multiple bytes.
	void send(const uint8_t *buf, size_t count);

	// Save SPISettings for new max SCK frequency
	void setSckSpeed(uint32_t maxSck);

private:
	SPISettings spiSettings;
};

#endif

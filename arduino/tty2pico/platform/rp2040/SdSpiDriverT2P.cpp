#include "SdSpiDriverT2P.h"
#include "MCU.h"

/*******************************************************************************
 * Custom SD SPI driver implementation - TODO: Add DMA support?
 *******************************************************************************/

static spi_inst_t *sdSpi;
static spi_cpol_t sdCpol;
static spi_cpha_t sdCpha;
static spi_order_t sdBitOrder;

SdSpiDriverT2P::SdSpiDriverT2P()
{
	sdSpi = getSdSpi();
}

void SdSpiDriverT2P::activate()
{
	// SDCARD_SPI.beginTransaction(spiSettings);
}

void SdSpiDriverT2P::begin(SdSpiConfig config)
{
	spi_deinit(sdSpi);

	setSckSpeed(config.maxSck);
	sdCpol = get_cpol(spiSettings.getDataMode());
	sdCpha = get_cpha(spiSettings.getDataMode());
	sdBitOrder = spiSettings.getBitOrder() == MSBFIRST ? SPI_MSB_FIRST : SPI_LSB_FIRST;

	spi_init(sdSpi, spiSettings.getClockFreq());
	spi_set_format(sdSpi, 8, sdCpol, sdCpha, sdBitOrder);

	gpio_set_function(SDCARD_MISO_PIN, GPIO_FUNC_SPI);
	gpio_set_function(SDCARD_MOSI_PIN, GPIO_FUNC_SPI);
	gpio_set_function(SDCARD_SCK_PIN, GPIO_FUNC_SPI);

	gpio_pull_up(SDCARD_MISO_PIN); // Pull up MISO

	if (SDCARD_CS_PIN > -1)
	{
		gpio_set_function(SDCARD_CS_PIN, GPIO_FUNC_SPI);
		gpio_pull_up(SDCARD_CS_PIN);
	}
}

void SdSpiDriverT2P::deactivate()
{
	noop();
}

uint8_t SdSpiDriverT2P::receive()
{
	uint8_t value;
	spi_read_blocking(sdSpi, 0XFF, &value, 1);
	return value;
}

uint8_t SdSpiDriverT2P::receive(uint8_t *buf, size_t count)
{
	spi_read_blocking(sdSpi, 0xFF, buf, count);
	return 0;
}

void SdSpiDriverT2P::send(uint8_t data)
{
	spi_write_blocking(sdSpi, &data, 1);
}

void SdSpiDriverT2P::send(const uint8_t *buf, size_t count)
{
	spi_write_blocking(sdSpi, buf, count);
}

void SdSpiDriverT2P::setSckSpeed(uint32_t maxSck)
{
	spiSettings = SPISettings(maxSck, MSBFIRST, SPI_MODE0);
}

#include "lcd.h"

/*
ST7789 driver over SPI1 + DMA.
Commands and their parameters are sent as 8-bit SPI frames (blocking).
Pixel data is sent as 16-bit SPI frames (DMA), so RGB565 goes out high byte first.
*/

// ST7789 commands
#define ST7789_SWRESET 0x01 //reset to factory default
#define ST7789_SLPOUT  0x11 //sleep out. turns on voltage generation. if this isnt sent, screen will remain blank
#define ST7789_NORON   0x13 //enables normal mode. st7789 supports partial mode, which saves screen battery
#define ST7789_INVON   0x21 //Display inversion on. some displays will show photonegative colors
#define ST7789_DISPON  0x29 //power display switch. connects internal buffer to display via transistor driver
#define ST7789_CASET   0x2A //column address set. sets horizontal limit.
#define ST7789_RASET   0x2B //row address set.vertical limit
#define ST7789_RAMWR   0x2C //moves color cursor to top left, and defines following data send as pixel sends.
#define ST7789_MADCTL  0x36 //memory access data control. 1 byte where each bit is a switch for rotation, mirror...
#define ST7789_COLMOD  0x3A //defines how many bits represent a pizel (for color depth)

#define DMA_MAX_CHUNK 65535u // HAL_SPI_Transmit_DMA size is a uint16_t

static SPI_HandleTypeDef *lcd_spi;

// DMA state, shared with the transfer-complete interrupt
static volatile int dma_busy = 0;
static const uint16_t *volatile dma_ptr; //pointer itself is volatile. pixel read only
static volatile uint32_t dma_remaining; //how many mixels left to send. 240x320 = 76800 so 32bit

// switches the SPI frame size. SPI has to be disabled to change it
static void spi_set_datasize(uint32_t size)
{
	if (lcd_spi->Init.DataSize == size) return;

	__HAL_SPI_DISABLE(lcd_spi);
	if (size <= SPI_DATASIZE_8BIT)
		MODIFY_REG(lcd_spi->Instance->CR2, SPI_CR2_DS | SPI_CR2_FRXTH, size | SPI_RXFIFO_THRESHOLD); //switches to 8bit
	else
		MODIFY_REG(lcd_spi->Instance->CR2, SPI_CR2_DS | SPI_CR2_FRXTH, size); //switches to 16 bit
	lcd_spi->Init.DataSize = size; // HAL reads this to decide 8 vs 16-bit writes
}

// starts the next piece of a DMA frame (at most 65535 pixels)
static void dma_start_chunk(void)
{
	uint32_t n = dma_remaining > DMA_MAX_CHUNK ? DMA_MAX_CHUNK : dma_remaining; //remaining pixels to send vs max
	const uint16_t *p = dma_ptr;

	dma_ptr = p + n;
	dma_remaining -= n;
	HAL_SPI_Transmit_DMA(lcd_spi, (uint8_t *)p, (uint16_t)n); //p = pointer to data, n = nb items
}

void lcd_init(SPI_HandleTypeDef *hspi)
{
	lcd_spi = hspi;

	// display is the only device on SPI1, so CS stays low the whole time
	// (harmless on modules without a CS pin)
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

	// hardware reset
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(120);

	lcd_cmd(ST7789_SWRESET); //extra reset
	HAL_Delay(150);

	lcd_cmd(ST7789_SLPOUT); // wake up
	HAL_Delay(120);

	uint8_t colmod = 0x55; // 16 bits per pixel (RGB565)
	lcd_cmd(ST7789_COLMOD);
	lcd_data(&colmod, 1);
	HAL_Delay(10);

	uint8_t madctl = 0x00; // normal orientation, RGB order
	lcd_cmd(ST7789_MADCTL);
	lcd_data(&madctl, 1);

	lcd_cmd(ST7789_INVON); // IPS panels need inversion on for correct colors
	HAL_Delay(10);

	lcd_cmd(ST7789_NORON);
	HAL_Delay(10);

	lcd_cmd(ST7789_DISPON);
	HAL_Delay(20);
}

void lcd_cmd(uint8_t cmd)
{
	while (dma_busy) {} // never interrupt a frame in flight

	spi_set_datasize(SPI_DATASIZE_8BIT);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET); // DC low = command
	HAL_SPI_Transmit(lcd_spi, &cmd, 1, HAL_MAX_DELAY);
}

void lcd_data(const uint8_t *data, uint32_t len)
{
	while (dma_busy) {}

	spi_set_datasize(SPI_DATASIZE_8BIT);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET); // DC high = data
	HAL_SPI_Transmit(lcd_spi, (uint8_t *)data, (uint16_t)len, HAL_MAX_DELAY);
}

void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	y0 += LCD_Y_OFFSET;
	y1 += LCD_Y_OFFSET;

	// each coordinate is sent as 2 bytes, high byte first
	uint8_t col[4] = { x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF };
	uint8_t row[4] = { y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF };

	lcd_cmd(ST7789_CASET);
	lcd_data(col, 4);
	lcd_cmd(ST7789_RASET);
	lcd_data(row, 4);
	lcd_cmd(ST7789_RAMWR); // every pixel sent after this fills the window
}

void lcd_push_dma(const uint16_t *buf, uint32_t count)
{
	if (count == 0) return;
	while (dma_busy) {}

	spi_set_datasize(SPI_DATASIZE_16BIT);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET); // pixels are data

	dma_ptr = buf;
	dma_remaining = count;
	dma_busy = 1;
	dma_start_chunk();
	// returns immediately; the CPU is free while DMA sends the frame
}

int lcd_busy(void)
{
	return dma_busy;
}

void lcd_fill(uint16_t color)
{
	// one block of pixels, sent over and over until the screen is covered
	static uint16_t block[LCD_WIDTH * 16];
	const uint32_t block_len = sizeof(block) / sizeof(block[0]);

	while (dma_busy) {} // block may still be in use by the last fill
	for (uint32_t i = 0; i < block_len; i++)
		block[i] = color;

	lcd_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

	uint32_t remaining = (uint32_t)LCD_WIDTH * LCD_HEIGHT;
	while (remaining > 0) {
		uint32_t n = remaining > block_len ? block_len : remaining;
		lcd_push_dma(block, n); // waits for the previous block first
		remaining -= n;
	}
	while (dma_busy) {}
}

// called by the HAL from the DMA interrupt when a transfer finishes
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi != lcd_spi) return;

	if (dma_remaining > 0)
		dma_start_chunk(); // frame was bigger than one DMA transfer
	else
		dma_busy = 0;
}

// on an SPI/DMA error, release the busy flag so the program doesn't hang
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi != lcd_spi) return;

	dma_remaining = 0;
	dma_busy = 0;
}

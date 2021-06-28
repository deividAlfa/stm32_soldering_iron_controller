/*
 * ssd1306.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose (PTDreamer), 2017
 */

#ifndef GRAPHICS_SSD1306_H_
#define GRAPHICS_SSD1306_H_

#include "u8g2.h"
#include "main.h"
#include "widgets.h"

//For checking if SPI DMA is active. Check before drawing the buffer.
typedef enum {
	oled_idle=0,
	oled_busy=1,
} oledStatus_t;


typedef enum{
	error_NMI,
	error_HARDFAULT,
	error_MEMMANAGE,
	error_BUSFAULT,
	error_USAGEFAULT,
	error_RUNAWAY25,
	error_RUNAWAY50,
	error_RUNAWAY75,
	error_RUNAWAY100,
	error_RUNAWAY500,
	error_RUNAWAY_UNKNOWN,
}FatalErrors;

#define OledWidth	128
#define OledHeight	64

// buffer needs to be aligned to 32bit(4byte) boundary, as FillBuffer() uses 32bit transfer for increased speed
typedef struct{
	__attribute__((aligned(4))) uint8_t buffer[128*8]; // 128x64 1BPP OLED
	uint8_t *ptr;
	volatile uint8_t status;
	volatile uint8_t row;
	volatile uint8_t use_sw;
	#if defined OLED_SPI && defined OLED_DEVICE
	SPI_HandleTypeDef *device;

	#elif defined OLED_I2C && defined OLED_DEVICE
	I2C_HandleTypeDef *device;
	#endif

	DMA_HandleTypeDef *fillDMA;
}oled_t;
extern oled_t oled;

enum { fill_soft, fill_dma };

#define Oled_Set_SCL() 		SW_SCL_GPIO_Port->BSRR = (uint32_t)SW_SCL_Pin
#define Oled_Clear_SCL() 	SW_SCL_GPIO_Port->BRR = (uint32_t)SW_SCL_Pin

#define Oled_Set_SDA() 		SW_SDA_GPIO_Port->BSRR = (uint32_t)SW_SDA_Pin
#define Oled_Clear_SDA() 	SW_SDA_GPIO_Port->BRR = (uint32_t)SW_SDA_Pin

#ifdef USE_CS
#define Oled_Set_CS() 		OLED_CS_GPIO_Port->BSRR = (uint32_t)OLED_CS_Pin
#define Oled_Clear_CS() 	OLED_CS_GPIO_Port->BRR = (uint32_t)OLED_CS_Pin
#endif

#ifdef USE_DC
#define Oled_Set_DC() 		OLED_DC_GPIO_Port->BSRR = (uint32_t)OLED_DC_Pin
#define Oled_Clear_DC() 	OLED_DC_GPIO_Port->BRR = (uint32_t)OLED_DC_Pin
#endif

#ifdef USE_RST
#define Oled_Set_RES() 		OLED_RST_GPIO_Port->BSRR = (uint32_t)OLED_RST_Pin
#define Oled_Clear_RES() 	OLED_RST_GPIO_Port->BRR = (uint32_t)OLED_RST_Pin
#endif

#define BLACK 0
#define WHITE 1
#define XOR   2

#if !defined OLED_SPI && !defined OLED_I2C
#error "No display configured in board.h!"
#elif defined OLED_I2C && !defined OLED_ADDRESS
#error "No display I2C address configured in board.h!"
#endif

#if defined OLED_SPI && defined OLED_DEVICE
void ssd1306_init(SPI_HandleTypeDef *device,DMA_HandleTypeDef *dma);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *device);

#elif defined OLED_SPI
void Enable_Soft_SPI(void);
void ssd1306_init(DMA_HandleTypeDef *dma);
void spi_send(uint8_t* bf, uint16_t count);
#endif


#if defined OLED_I2C
#define i2cData 0
#define i2cCmd  1

#if !defined OLED_DEVICE || (defined OLED_DEVICE && defined I2C_TRY)
void Enable_Soft_I2C(void);
void i2cStart(void);
void i2cStop(void);
void i2cBegin(bool isCmd);
void i2cSend(uint8_t* bf, uint16_t count, bool isCmd);
void i2cWait(void);
#endif

#if defined OLED_DEVICE
void ssd1306_init(I2C_HandleTypeDef *device,DMA_HandleTypeDef *dma);
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *device);
#else
void ssd1306_init(DMA_HandleTypeDef *dma);
#endif
#endif

void FatalError(uint8_t type);
void write_data(uint8_t* data, uint16_t count);
void write_cmd(uint8_t cmd);
void pset(uint8_t x, uint8_t y, bool c);
void update_display(void);
void display_abort(void);
void update_display_ErrorHandler(void);
void setContrast(uint8_t value);
void setOledRow(uint8_t row);
uint8_t getContrast();
void FillBuffer(bool color, bool mode);
void putStrAligned(char* str, uint8_t y, AlignType align);
void Reset_onError(void);
#endif /* GRAPHICS_SSD1306_H_ */

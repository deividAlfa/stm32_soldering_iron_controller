/*
 * ssd1306.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose Barros (PTDreamer), 2017
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
  error_FLASH,
}FatalErrors;

#define OledWidth	  128
#define OledHeight	64

// buffer needs to be aligned to 32bit(4byte) boundary, as FillBuffer() uses 32bit transfer for increased speed
typedef struct{
	__attribute__((aligned(4))) uint8_t buffer[128*8]; // 128x64 1BPP OLED
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

#if defined OLED_SPI
#define Oled_Set_SCL() 		SW_SCL_GPIO_Port->BSRR = SW_SCL_Pin
#define Oled_Clear_SCL() 	SW_SCL_GPIO_Port->BRR = SW_SCL_Pin

#elif  defined OLED_I2C
#define Oled_Set_SCL()    SW_SCL_GPIO_Port->BSRR = SW_SCL_Pin; i2c_Delay_H()							// Rise time needs more time, as it's open drain
#define Oled_Clear_SCL()  SW_SCL_GPIO_Port->BRR = SW_SCL_Pin; i2c_Delay_L()								// Fall time can be much faster
#endif

#define Oled_Clock()      Oled_Set_SCL(); Oled_Clear_SCL()

#define Oled_Set_SDA() 		SW_SDA_GPIO_Port->BSRR = SW_SDA_Pin
#define Oled_Clear_SDA() 	SW_SDA_GPIO_Port->BRR = SW_SDA_Pin

#define Oled_Bit(n)      if(!(n))Oled_Clear_SDA();else Oled_Set_SDA(); Oled_Clock()				// Testing !n compiles faster code than testing n

#ifdef USE_CS
#define Oled_Set_CS() 		OLED_CS_GPIO_Port->BSRR = OLED_CS_Pin
#define Oled_Clear_CS() 	OLED_CS_GPIO_Port->BRR = OLED_CS_Pin
#endif

#ifdef USE_DC
#define Oled_Set_DC() 		OLED_DC_GPIO_Port->BSRR = OLED_DC_Pin
#define Oled_Clear_DC() 	OLED_DC_GPIO_Port->BRR = OLED_DC_Pin
#endif

#ifdef USE_RST
#define Oled_Set_RES() 		OLED_RST_GPIO_Port->BSRR = OLED_RST_Pin
#define Oled_Clear_RES() 	OLED_RST_GPIO_Port->BRR = OLED_RST_Pin
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
void oled_send(uint8_t* bf, uint16_t count);
#endif

#if !defined OLED_DEVICE || (defined OLED_DEVICE && defined I2C_TRY_HW)
void enable_soft_Oled(void);
void disable_soft_Oled(void);
#endif

#if defined OLED_I2C
#define i2cData 0x40
#define i2cCmd  0x00

#if !defined OLED_DEVICE || (defined OLED_DEVICE && defined I2C_TRY_HW)
void i2cStart(void);
void i2cStop(void);
void i2cBegin(uint8_t mode);
void oled_send(uint8_t* bf, uint16_t count, uint8_t mode);
void i2c_Delay_L(void);
void i2c_Delay_H(void);
#endif

#if defined OLED_DEVICE
void ssd1306_init(I2C_HandleTypeDef *device,DMA_HandleTypeDef *dma);
void i2c_workaround(void);
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
void setOledPower(uint8_t power);
uint8_t getOledPower(void);
void setContrast(uint8_t value);
void setOledRow(uint8_t row);
uint8_t getContrast();
void FillBuffer(bool color, bool mode);
void putStrAligned(char* str, uint8_t y, AlignType align);
void Reset_onError(void);
#endif /* GRAPHICS_SSD1306_H_ */

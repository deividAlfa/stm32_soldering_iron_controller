/*
 * ssd1306.h
 *
 *  Created on: Jul 17, 2017
 *      Author: jose
 */

#ifndef GRAPHICS_SSD1306_H_
#define GRAPHICS_SSD1306_H_
#include "ugui.h"
#include "main.h"

//For checking if SPI DMA is active. Check before drawing the buffer.
typedef enum {
	oled_idle,
	oled_sending_data,
	oled_sending_cmd,
} oled_status_t;

enum { fill_soft, fill_dma };

#define Oled_Set_CS() HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin,GPIO_PIN_SET)
#define Oled_Clear_CS() HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin,GPIO_PIN_RESET)
#define Oled_Set_DC() HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin,GPIO_PIN_SET)
#define Oled_Clear_DC() HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin,GPIO_PIN_RESET)
#define Oled_Set_RES() HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin,GPIO_PIN_SET)
#define Oled_Clear_RES() HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin,GPIO_PIN_RESET)
#define BLACK 0
#define WHITE 1

extern volatile uint8_t OledBuffer[128*8]; // 128x64 1BPP OLED
extern volatile oled_status_t oled_status;
extern volatile uint8_t *OledDmaBf;


#ifdef OLED_SOFT_SPI
void Enable_Soft_SPI(void);
void ssd1306_init(DMA_HandleTypeDef *dma);
void spi_send(uint8_t* bf, uint16_t count);

#elif defined OLED_SOFT_I2C
void Enable_Soft_I2C(void);
void ssd1306_init(DMA_HandleTypeDef *dma);
void i2c_send(bool isCMD, uint8_t* bf, uint16_t count);
#elif defined OLED_SPI
void ssd1306_init(SPI_HandleTypeDef *device,DMA_HandleTypeDef *dma);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *device);

#elif defined OLED_I2C
void ssd1306_init(I2C_HandleTypeDef *device,DMA_HandleTypeDef *dma);
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *device);
#endif

void FatalError(uint8_t type);
void write_data(uint8_t* data, uint16_t count);
void write_cmd(uint8_t cmd);
void pset(UG_U16 x, UG_U16 y, UG_COLOR c);
void update_display(void);
void display_abort(void);
void update_display_ErrorHandler(void);
void setContrast(uint8_t value);
void setOledRow(uint8_t row);
uint8_t getContrast();
void FillBuffer(bool color, bool mode);

#endif /* GRAPHICS_SSD1306_H_ */

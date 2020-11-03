/*
 * ssd1306.h
 *
 *  Created on: Jul 17, 2017
 *      Author: jose
 */

#ifndef GRAPHICS_SSD1306_H_
#define GRAPHICS_SSD1306_H_
#include "ugui.h"
#include "stm32f0xx_hal.h"
#include "main.h"

//For checking if SPI DMA is active. Check before drawing the buffer.
typedef enum {
	oled_idle,
	oled_sending_data,
	oled_sending_cmd,
} oled_status_t;

#define Oled_Set_CS() HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin,GPIO_PIN_SET)
#define Oled_Clear_CS() HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin,GPIO_PIN_RESET)
#define Oled_Set_DC() HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin,GPIO_PIN_SET)
#define Oled_Clear_DC() HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin,GPIO_PIN_RESET)
#define Oled_Set_RES() HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin,GPIO_PIN_SET)
#define Oled_Clear_RES() HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin,GPIO_PIN_RESET)




extern volatile unsigned char OledBuffer[]; // 128x64 1BPP OLED
extern volatile oled_status_t oled_status;
extern volatile uint8_t *OledDmaBf;


#ifdef Soft_SPI

void write_byte(uint8_t data);
void spi_send(uint8_t SPIData);
void ssd1306_init(void);

#else

void ssd1306_init(SPI_HandleTypeDef *hspi);

#endif

void write_data(uint8_t *data);
void write_cmd(uint8_t data);
void send_display_bf(uint8_t *oled_buffer);
void pset(UG_S16 x, UG_S16 y, UG_COLOR c);
void update_display( void );
void setContrast(uint8_t value);
uint8_t getContrast();
#endif /* GRAPHICS_SSD1306_H_ */

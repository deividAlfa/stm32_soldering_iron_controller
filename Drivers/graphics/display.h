/*
 * lcd.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David		Original work by Jose Barros (PTDreamer), 2017
 */

#ifndef GRAPHICS_SSD1306_H_
#define GRAPHICS_SSD1306_H_

#include "u8g2.h"
#include "main.h"
#include "widgets.h"


/*    Display size          */
#define displayWidth   128
#define displayHeight  64

/*    Display command set   */
#if defined ST7565
typedef enum{
  c_col_L         = 0x00,
  c_col_H         = 0x10,
  c_start_line    = 0x40,
  c_page          = 0xB0,
  c_set_volume    = 0x81,
  c_pump_set      = 0x8D,
  c_pump_off      = 0x10,
  c_pump_on       = 0x14,
  c_res_ratio     = 0x20,
  c_pwr_ctrl      = 0x28,
  c_pwr_boost     = 0x04,
  c_pwr_vreg      = 0x02,
  c_pwr_follow    = 0x01,
  c_remap_off     = 0xA0,
  c_remap_on      = 0xA1,
  c_bias_9        = 0xA2,
  c_bias_7        = 0xA3,
  c_all_off       = 0xA4,
  c_all_on        = 0xA5,
  c_inv_off       = 0xA6,
  c_inv_on        = 0xA7,
  c_mux_ratio     = 0xA8,
  c_static_off    = 0xAC,
  c_static_on     = 0xAD,
  c_static_slow   = 0x01,
  c_static_fast   = 0x02,
  c_static_always = 0x03,
  c_disp_off      = 0xAE,
  c_disp_on       = 0xAF,
  c_offset        = 0xD3,
  c_clock_set     = 0xD5,
  c_precharge     = 0xD9,
  c_vcomh_lvl     = 0xDB,
  c_com_norm      = 0xC0,
  c_com_rev       = 0xC8,
  c_com_cfg       = 0xDA,
  c_rmw_set       = 0xE0,
  c_rmw_clr       = 0xEE,
  c_sw_reset      = 0xE2,
  c_nop           = 0xE3,
  c_test          = 0xF0,
  c_boost_ratio   = 0xF8,
  c_boost_234     = 0x00,
  c_boost_5       = 0x01,
  c_boost_6       = 0x03,
}cmds;
#endif

#if defined SSD1306
typedef enum{
  c_col_L         = 0x00,
  c_col_H         = 0x10,
  c_start_line    = 0x40,
  c_page          = 0xB0,
  c_contrast      = 0x81,
  c_pump_1306_set = 0x8D,
  c_pump_off      = 0x10,
  c_pump_on       = 0x14,
  c_addr_mode     = 0x20,
  c_pump_1106_adj = 0x30,
  c_remap_off     = 0xA0,
  c_remap_on      = 0xA1,
  c_all_off       = 0xA4,
  c_all_on        = 0xA5,
  c_inv_off       = 0xA6,
  c_inv_on        = 0xA7,
  c_mux_ratio     = 0xA8,
  c_pump_1106_set = 0xAD,
  c_disp_off      = 0xAE,
  c_disp_on       = 0xAF,
  c_offset        = 0xD3,
  c_clock_set     = 0xD5,
  c_precharge     = 0xD9,
  c_vcomh_lvl     = 0xDB,
  c_com_norm      = 0xC0,
  c_com_rev       = 0xC8,
  c_com_cfg       = 0xDA,
  c_nop           = 0xE3,
  c_rmw_set       = 0xE0,
  c_rmw_clr       = 0xEE,
  c_test          = 0xF0
}cmds;
#endif

// For checking if SPI DMA is active. Checked before drawing the buffer.
typedef enum {
	oled_idle=0,
	oled_busy=1,
} oledStatus_t;


/*    Data send modes    */
#define modeCmd     0x00
#define modeData    0x40

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


// buffer needs to be aligned to 32bit(4byte) boundary, as fillBuffer() uses 32bit transfer for increased speed
typedef struct{
	__attribute__((aligned(4))) uint8_t buffer[128*8]; // 128x64 1BPP OLED
	volatile uint8_t status;
	volatile uint8_t row;
	volatile uint8_t use_sw;
#if defined DISPLAY_SPI && defined DISPLAY_DEVICE
	SPI_HandleTypeDef *device;
#elif defined DISPLAY_I2C && defined DISPLAY_DEVICE
	I2C_HandleTypeDef *device;
#endif
	DMA_HandleTypeDef *fillDMA;
}oled_t;

extern oled_t oled;
extern volatile uint32_t hardFault_args[9];
extern volatile uint32_t r4, r5, r6;

enum { fill_soft, fill_dma };

#if defined DISPLAY_SPI
#define Oled_Set_SCL() 		SW_SCL_GPIO_Port->BSRR = SW_SCL_Pin
#define Oled_Clear_SCL() 	SW_SCL_GPIO_Port->BRR = SW_SCL_Pin

#elif  defined DISPLAY_I2C
#define Oled_Set_SCL()    SW_SCL_GPIO_Port->BSRR = SW_SCL_Pin; bit_delay()							// Rise time needs more time, as it's open drain
#define Oled_Clear_SCL()  SW_SCL_GPIO_Port->BRR = SW_SCL_Pin; bit_delay()								// Fall time can be much faster
#endif

#define Oled_Clock()      Oled_Set_SCL(); Oled_Clear_SCL();

#define Oled_Set_SDA() 		SW_SDA_GPIO_Port->BSRR = SW_SDA_Pin
#define Oled_Clear_SDA() 	SW_SDA_GPIO_Port->BRR = SW_SDA_Pin

#define Oled_Bit(n)      if(!(n))Oled_Clear_SDA();else Oled_Set_SDA(); Oled_Clock()				// Testing !n compiles faster code than testing n

#ifdef USE_CS
#define Oled_Set_CS() 		DISPLAY_CS_GPIO_Port->BSRR = DISPLAY_CS_Pin
#define Oled_Clear_CS() 	DISPLAY_CS_GPIO_Port->BRR = DISPLAY_CS_Pin
#endif

#ifdef USE_DC
#define Oled_Set_DC() 		DISPLAY_DC_GPIO_Port->BSRR = DISPLAY_DC_Pin
#define Oled_Clear_DC() 	DISPLAY_DC_GPIO_Port->BRR = DISPLAY_DC_Pin
#endif

#ifdef USE_RST
#define Oled_Set_RES() 		DISPLAY_RST_GPIO_Port->BSRR = DISPLAY_RST_Pin
#define Oled_Clear_RES() 	DISPLAY_RST_GPIO_Port->BRR = DISPLAY_RST_Pin
#endif

#define BLACK 0
#define WHITE 1
#define XOR   2

#if !defined DISPLAY_DEVICE || (defined DISPLAY_DEVICE && defined I2C_TRY_HW)
void enable_soft_mode(void);
void disable_soft_mode(void);
void bitbang_write(uint8_t* bf, uint16_t count, uint8_t mode);
void bit_delay(void);
#if defined DISPLAY_I2C
void i2cStart(void);
void i2cStop(void);
void i2cBegin(uint8_t mode);
#endif
#endif

#if defined DISPLAY_DEVICE
#if defined DISPLAY_SPI
void lcd_init(SPI_HandleTypeDef *device,DMA_HandleTypeDef *dma);
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *device);
#elif defined DISPLAY_I2C
void lcd_init(I2C_HandleTypeDef *device,DMA_HandleTypeDef *dma);
void i2c_workaround(void);
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *device);
#endif
#else
void lcd_init(DMA_HandleTypeDef *dma);
#endif

void Oled_error_init(void);
void fatalError(uint8_t type);
void lcd_write(uint8_t* data, uint16_t count, uint8_t mode);
void pset(uint8_t x, uint8_t y, bool c);
void update_display(void);
void display_dma_abort(void);
void update_display_ErrorHandler(void);
void setDisplayRow(uint8_t row);
void setDisplayPower(uint8_t power);
uint8_t getDisplayPower(void);
void setDisplayContrastOrBrightness(uint8_t value);
void setDisplayXflip(uint8_t f);
void setDisplayYflip(uint8_t f);
#ifdef ST7565
void setDisplayResRatio(uint8_t r);
#endif
uint8_t getDisplayContrastOrBrightness(void);
void fillBuffer(bool color, bool mode);
void putStrAligned(char* str, uint8_t y, AlignType align);
void buttonReset(void);


/*    Error checking    */
#if !defined FILL_DMA
#error "No FILL_DMA handler defined!"
#endif

#if !defined DISPLAY_SPI && !defined DISPLAY_I2C
#error "No display configured in board.h!"
#endif

#if defined DISPLAY_I2C && !defined DISPLAY_ADDRESS
#error "No display I2C address configured in board.h!"
#endif

#if defined DISPLAY_SPI && !defined USE_DC
#error "Mandatory DISPLAY_DC Pin for SPI not configured!"
#endif

#if (!defined DISPLAY_DEVICE || defined I2C_TRY_HW) && (!defined SW_SCL_Pin || !defined SW_SDA_Pin)
#error "Current configuration requires definition of display pins "SW_SDA_Pin" and "SW_SCL_Pin""
#endif

#if defined I2C_TRY_HW && (!defined HW_SCL_Pin || !defined HW_SDA_Pin)
#error "Current configuration requires definition of display pins "HW_SDA_Pin" and "HW_SCL_Pin""
#endif

#if defined I2C_TRY_HW && !defined DISPLAY_DEVICE
#error "I2C_TRY_HW defined, but DISPLAY_DEVICE is missing!"
#endif

#if defined DISPLAY_I2C && defined ST7565
#error "ST7565 cannot operate in I2C mode!"
#endif

#endif /* GRAPHICS_SSD1306_H_ */

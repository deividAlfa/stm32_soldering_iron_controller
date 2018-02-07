#include "ssd1306.h"
static unsigned char buffer[128*8]; // 128x64 1BPP OLED
static SPI_HandleTypeDef * m_hspi;
static uint8_t m_contrast = 0xCF;

void write_data(uint8_t *data) {
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin,GPIO_PIN_SET);//CS
	HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin,GPIO_PIN_SET);//DC
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin,GPIO_PIN_RESET);//CS
	HAL_SPI_Transmit(m_hspi, data, 128, 1000);
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin,GPIO_PIN_SET);//CS
}
void write_cmd(uint8_t data) {
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin,GPIO_PIN_SET);//CS
	HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin,GPIO_PIN_RESET);//DC
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin,GPIO_PIN_RESET);//CS
	HAL_SPI_Transmit(m_hspi, &data, 1, 1000);
	HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin,GPIO_PIN_SET);//CS
}
void pset(UG_S16 x, UG_S16 y, UG_COLOR c)
{
   unsigned int p;

   if ( x > 127 ) return;
   p = y>>3; // :8
   p = p<<7; // *128
   p +=x;

   if( c )
   {
      buffer[p] |= 1<<(y%8);
   }
   else
   {
      buffer[p] &= ~(1<<(y%8));
   }

}

void update_display( void )
{
   unsigned int p;
   for(p=0;p<8;p++)
   {
      write_cmd(0xB0|p);
      write_cmd(0x00+((0x10&0x0F)*16+0x00)%16);
      write_cmd(0x10+((0x10&0x0F)*16+0x00)/16);
      write_data(buffer + p * 128);
   }
}

void setContrast(uint8_t value) {
	write_cmd(0x81);         // Set Contrast Control
	write_cmd(value);         //   Default => 0x7F
	m_contrast = value;
}
uint8_t getContrast() {
	return m_contrast;
}
void ssd1306_init(SPI_HandleTypeDef *hspi)
{
	m_hspi = hspi;
	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin,GPIO_PIN_RESET);//RST
	HAL_Delay(100);
	HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin,GPIO_PIN_SET);//RST
	HAL_Delay(100);

   write_cmd(0xAE| 0x00);  // Display Off (0x00/0x01)

   write_cmd(0xD5);         // Set Display Clock Divide Ratio / Oscillator Frequency
   write_cmd(0x80);         // Set Clock as 100 Frames/Sec

   write_cmd(0xA8);         // Set Multiplex Ratio
   write_cmd(0x3F);         //   Default => 0x3F (1/64 Duty)

   write_cmd(0xD3);         // Set Display Offset
   write_cmd(0x00);         //   Default => 0x00

   write_cmd(0x40|0x00);   // Set Display Start Line

   write_cmd(0x8D);         // Set Charge Pump
   write_cmd(0x10|0x04);   //   Default => 0x10
  // write_cmd_2(0x10|0x04);   //   Default => 0x10

   write_cmd(0x20);         // Set Memory Addressing Mode
   write_cmd(0x02);         //   Default => 0x02

   write_cmd(0xA0|0x01);   // Set Segment Re-Map

   write_cmd(0xC0|0x08);   // Set COM Output Scan Direction

   write_cmd(0xDA);         // Set COM Pins Hardware Configuration
   write_cmd(0x02|0x10);   //   Default => 0x12 (0x10)

   write_cmd(0x81);         // Set Contrast Control
   write_cmd(0xCF);         //   Default => 0x7F

   write_cmd(0xD9);         // Set Pre-Charge Period
   write_cmd(0x22);         //   Default => 0x22 (2 Display Clocks [Phase 2] / 2 Display Clocks [Phase 1])
 //  write_cmd_2(0xF1);         //   Default => 0x22 (2 Display Clocks [Phase 2] / 2 Display Clocks [Phase 1])

   write_cmd(0xDB);         // Set VCOMH Deselect Level
   write_cmd(0x30);         //   Default => 0x20 (0.77*VCC)

   write_cmd(0xA4|0x00);   // Set Entire Display On/Off

   write_cmd(0xA6|0x00);   // Set Inverse Display On/Off

   write_cmd(0xAE|0x01);   // Set Display On/Off
}

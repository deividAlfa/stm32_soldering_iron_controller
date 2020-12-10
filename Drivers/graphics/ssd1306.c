#include "ssd1306.h"
#include "settings.h"

// Need to be aligned to 32bit(4byte) boundary, as FillBuffer() uses 32bit tranfer for increased speed
__attribute__((aligned(4))) volatile uint8_t OledBuffer[128*8]; // 128x64 1BPP OLED

volatile uint8_t *OledDmaBf = &OledBuffer[0];
volatile oled_status_t oled_status=oled_idle;
static SPI_HandleTypeDef *spi_device;

#ifdef Soft_SPI

void Enable_Soft_SPI_SPI(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	HAL_SPI_MspDeInit(&SPI_DEVICE);
	 /*Configure GPIO pins : SCK_Pin */
	 GPIO_InitStruct.Pin = 	SCK_Pin;
	 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	 GPIO_InitStruct.Pull = GPIO_NOPULL;
	 GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	 HAL_GPIO_Init(SCK_GPIO_Port, &GPIO_InitStruct);

	 /*Configure GPIO pins : SDO_Pin */
	 GPIO_InitStruct.Pin = 	SDO_Pin;
	 GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	 GPIO_InitStruct.Pull = GPIO_NOPULL;
	 GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	 HAL_GPIO_Init(SDO_GPIO_Port, &GPIO_InitStruct);
}

void spi_send(uint8_t SPIData){
	unsigned char SPICount;                               // Counter used to clock out the data

	for (SPICount = 0; SPICount < 8; SPICount++)          // Prepare to clock out the Address byte
		  {
		    if (SPIData & 0x80)                                 // Check for a 1
		    	HAL_GPIO_WritePin(SDO_GPIO_Port, SDO_Pin, GPIO_PIN_SET);
		    else
		    	HAL_GPIO_WritePin(SDO_GPIO_Port, SDO_Pin, GPIO_PIN_RESET);
		    	HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_SET);
		    	HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_RESET);
		    	SPIData <<= 1;                                      // Rotate to get the next bit
		  }
}

void write_data(uint8_t *data) {
	uint8_t spiBytes=0;

	Oled_Set_CS();
	Oled_Set_DC();
	Oled_Clear_CS();
	while(spiBytes<128){
		spi_send(*data++);
		spiBytes++;
	}

	Oled_Set_CS();
}


void write_byte(uint8_t data) {

	Oled_Set_CS();
	Oled_Set_DC();
	Oled_Clear_CS();
	spi_send(data);

	Oled_Set_CS();
}

void write_cmd(uint8_t data) {
	Oled_Set_CS();	spidelay();
	Oled_Clear_DC();	spidelay();
	Oled_Clear_CS();	spidelay();
	spi_send(data);
	Oled_Set_CS();
}


void update_display( void )
{
	unsigned int p;
	for(p=0;p<8;p++){
		setOledRow(p);
		write_data(OledBuffer + p * 128);
   }
}



#else

// Send data in blocking mode (Not used in screen update)
void write_data(uint8_t data) {
	while(oled_status!=oled_idle);	// Wait for DMA to finish
	Oled_Set_CS();
	Oled_Set_DC();
	Oled_Clear_CS();
	if(HAL_SPI_Transmit(spi_device, &data, 1, 1000)!=HAL_OK){
		Error_Handler();
	}
	Oled_Set_CS();
}

// Send command in blocking mode
void write_cmd(uint8_t data) {
	while(oled_status==oled_sending_data);	//Wait for DMA to finish
	// Now, else we are in idle (oled_idle) or DMA wants to send a cmd (oled_sending_cmd)

	Oled_Set_CS();
	Oled_Clear_DC();
	Oled_Clear_CS();
	if(HAL_SPI_Transmit(spi_device, &data, 1, 1000)!=HAL_OK){
		Error_Handler();
	}
	Oled_Set_CS();
}

// Trigger DMA transfer
void update_display( void ){
		if(oled_status!=oled_idle) { return; }	// If OLED busy, skip update
		HAL_SPI_TxCpltCallback(spi_device); 	// Call the DMA callback function to send the frame
}

#endif

// Abort DMA transfers and reset status
void display_abort(void){
	HAL_SPI_Abort(spi_device);	// Abort SPI DMA
	HAL_DMA_PollForTransfer(spi_device->hdmatx, HAL_DMA_FULL_TRANSFER, 3000);	//Wait for DMA to finish
	oled_status=oled_idle;		// Force oled idle status
}

// Screen update for hard error handlers (crashes) not using DMA
void update_display_ErrorHandler(void){
	uint8_t p;
	for(p=0;p<8;p++){
		setOledRow(p);
		Oled_Clear_CS();
		Oled_Set_DC();
		if(HAL_SPI_Transmit(spi_device, (uint8_t*)OledDmaBf + (p * 128), 128, 1000)!=HAL_OK){
			while(1);			// If error happens at this stage, just do nothing
		}
   }
}

// Function for drawing a pixel in display buffer
void pset(UG_U16 x, UG_U16 y, UG_COLOR c){
   unsigned int p;

   if((x>127)||(y>63)) return;
   p = y>>3; // y/8
   p = p<<7; // *128
   p +=x;

   if( c )
   {
      OledBuffer[p] |= 1<<(y%8);
   }
   else
   {
      OledBuffer[p] &= ~(1<<(y%8));
   }
}

void setOledRow(uint8_t row){
	write_cmd(0xB0|row);									// Set the OLED Row address
	if(systemSettings.OledFix){	write_cmd(0x02); }
	else{ write_cmd(0x00); }
	write_cmd(0x10);
}

void setContrast(uint8_t value) {
	write_cmd(0x81);         // Set Contrast Control
	write_cmd(value);         //   Default => 0xFF
}


#ifdef Soft_SPI

void ssd1306_init(void){

#else

void ssd1306_init(SPI_HandleTypeDef *hspi){
	spi_device = hspi;
#endif
	Oled_Clear_RES();			// Enable RST
	HAL_Delay(0);				// HAL Adds+1 = 1mS
	Oled_Set_RES();				// Disable RST
	HAL_IWDG_Refresh(&hiwdg);	// Clear watchdog
	HAL_Delay(100);				// 100mS wait for internal initialization
	write_cmd(0xAE);  			// Display Off
	write_cmd(0xD5);         	// Set Display Clock Divide Ratio / Oscillator Frequency
	write_cmd(0b11110000);      // Set Clock as 100 Frames/Sec
	write_cmd(0xA8);         	// Set Multiplex Ratio
	write_cmd(0x3F);         	// Default => 0x3F (1/64 Duty)
	write_cmd(0xD3);         	// Set Display Offset
	write_cmd(0x00);         	// Default => 0x00
	write_cmd(0x40|0x00);   	// Set Display Start Line
	write_cmd(0x8D);         	// Set Charge Pump
	write_cmd(0x10|0x04);   	// Default => 0x10
	write_cmd(0x20);         	// Set Memory Addressing Mode
	write_cmd(0x02);         	// Default => 0x02
	write_cmd(0xA0|0x01);   	// Set Segment Re-Map
	write_cmd(0xC0|0x08);   	// Set COM Output Scan Direction
	write_cmd(0xDA);         	// Set COM Pins Hardware Configuration
	write_cmd(0x02|0x10);   	// Default => 0x12 (0x10)
	setContrast(0xFF);			// Init in max contrast
	write_cmd(0xD9);         	// Set Pre-Charge Period
	write_cmd(0x22);         	// Default => 0x22 (2 Display Clocks [Phase 2] / 2 Display Clocks [Phase 1])
//  write_cmd_2(0xF1);         	// Default => 0x22 (2 Display Clocks [Phase 2] / 2 Display Clocks [Phase 1])
	write_cmd(0xDB);         	// Set VCOMH Deselect Level
	write_cmd(0x30);         	// Default => 0x20 (0.77*VCC)
	write_cmd(0xA4|0x00);   	// Set Entire Display On/Off
	write_cmd(0xA6|0x00);   	// Set Inverse Display On/Off
	FillBuffer(C_BLACK,fill_dma);	// Clear buffer
	update_display();			// Update display CGRAM
	while(oled_status!=oled_idle);	// Wait for DMA completion
	write_cmd(0xAF);   			// Set Display On
}

/*
*	Clear buffer with 32bit-transfer for fast filling (ensure that Oled buffer is 32-bit aligned!)
* 	128 * 8 = 1KB, / 4byte DMA txfer = 256 clock cycles (in theory)
* 	Args:
* 			color: 	0 = black, 1 = white
* 			mode: 	0 = Use software(fail-safe), 1= Use DMA (normal operation)
*/

void FillBuffer(bool color, bool mode){
	uint32_t oled_fill;

	while(oled_status!=oled_idle);				// Don't wirte to buffer while screen buffger is being transfered

	if(color==C_WHITE){ oled_fill=0xffffffff; }	// Fill color = white
	else{ oled_fill=0; }						// Fill color = black

	if(mode==fill_dma){							// use DMA, pointing to
		 HAL_DMA_Start(&hdma_memtomem_dma1_channel2,(uint32_t)&oled_fill,(uint32_t)OledDmaBf,sizeof(OledBuffer)/sizeof(uint32_t));
		 HAL_DMA_PollForTransfer(&hdma_memtomem_dma1_channel2, HAL_DMA_FULL_TRANSFER, 3000);
	}
	else{										// use software
		uint32_t* bf=(uint32_t*)OledDmaBf;		// Pointer to oled buffer using 32bit data for faster operation
		for(uint16_t x=0;x<((128*8)/4);x++){	// Write to oled buffer
			bf[x]=oled_fill;
		}
	}
}

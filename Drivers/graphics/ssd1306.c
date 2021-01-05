#include "ssd1306.h"
#include "settings.h"
#include "buzzer.h"

// Need to be aligned to 32bit(4byte) boundary, as FillBuffer() uses 32bit tranfer for increased speed
__attribute__((aligned(4))) volatile uint8_t OledBuffer[128*8]; // 128x64 1BPP OLED

volatile uint8_t *OledPtr = &OledBuffer[0];
volatile oled_status_t oled_status=oled_idle;
#ifdef OLED_SPI
static SPI_HandleTypeDef *oledDevice;
#elif defined OLED_I2C
static I2C_HandleTypeDef *oledDevice;
#endif
static DMA_HandleTypeDef *oledFillDMA;


#if defined OLED_SOFT_I2C || defined OLED_SOFT_SPI

#ifdef OLED_SOFT_I2C
void Enable_Soft_I2C(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/*Configure GPIO pins : SCL_Pin */
	GPIO_InitStruct.Pin = 	OLED_SCL_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(OLED_SCL_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : SDA_Pin */
	GPIO_InitStruct.Pin = 	OLED_SDA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(OLED_SDA_GPIO_Port, &GPIO_InitStruct);

	Oled_Clear_SDA();
	HAL_Delay(1);
	Oled_Clear_SCL();
	HAL_Delay(1);
	Oled_Set_SCL();			//Force stop condition on init
	HAL_Delay(1);
	Oled_Set_SDA();
	HAL_Delay(1);
}
#endif

#ifdef OLED_SOFT_SPI
void Enable_Soft_SPI(void){
	//HAL_SPI_MspDeInit(&OLED_DEVICE);
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/*Configure GPIO pins : SCL_Pin */
	GPIO_InitStruct.Pin = 	OLED_SCL_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(OLED_SCL_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : SDA_Pin */
	GPIO_InitStruct.Pin = 	OLED_SDA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(OLED_SDA_GPIO_Port, &GPIO_InitStruct);

}
#endif


#endif

#ifdef OLED_SOFT_SPI
__attribute__((section(".RamFunc")))
void spi_send(uint8_t* bf, uint16_t count){
	uint8_t shift,data;
	while(count--){
		data = *bf++;
		if(data==0){
			Oled_Clear_SDA();
			for(shift = 0; shift < 8; shift++){
				Oled_Set_SCL();
				Oled_Clear_SCL();
			}
		}

		else if(data==0xFF){
			Oled_Set_SDA();
			for(shift = 0; shift < 8; shift++){
				Oled_Set_SCL();
				Oled_Clear_SCL();
			}
		}

		else{
			for(shift = 0; shift < 8; shift++){
				if(data & 0x80){
					Oled_Set_SDA();
				}
				else{
					Oled_Clear_SDA();
				}

				Oled_Set_SCL();
				data <<= 1;
				Oled_Clear_SCL();
			}
		}
	}
}
#endif

#ifdef OLED_SOFT_I2C
void i2cWait(void){
	asm(	"nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n\
			 nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n\
			 nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
}
void i2cStart(void){						// 	Start condition, SDA transition to low with SCL high
	Oled_Set_SCL();
	i2cWait();
	Oled_Clear_SDA();
	i2cWait();
	Oled_Clear_SCL();
	i2cWait();
}
void i2cStop(void){						// 	Stop condition, SCL transition to high with SDA low
	Oled_Clear_SDA();
	i2cWait();
	Oled_Set_SCL();
	i2cWait();
	Oled_Set_SDA();
	i2cWait();
}
__attribute__((section(".RamFunc")))
void i2cBegin(bool isCmd){
	uint8_t bf[2]= { OLED_ADDRESS, 0x00 };
	if(!isCmd){
		bf[1] = 0x40;
	}
	for(uint8_t d=0;d<2;d++){
		uint8_t data = bf[d];
		for(uint8_t shift = 0; shift < 8; shift++){
			if(data & 0x80){
				Oled_Set_SDA();
				i2cWait();
			}
			else{
				Oled_Clear_SDA();
				i2cWait();
			}
			Oled_Set_SCL();
			i2cWait();
			Oled_Clear_SCL();
			data <<= 1;
		}
		i2cWait();
		Oled_Set_SCL();
		i2cWait();
		//Oled_Set_SDA();										// As we don't care the ACK, don't release SDA
		//i2cWait();
		//Get ACK here
		Oled_Clear_SCL();
	}
}
__attribute__((section(".RamFunc")))
void i2cSend(uint8_t* bf, uint16_t count, bool isCmd){
	volatile uint8_t shift,data;
	//bool ack=0;
	i2cStart();
	i2cBegin(isCmd);
	while(count--){
		data = *bf++;
		if(data==0){
			Oled_Clear_SDA();
			i2cWait();
			for(shift = 0; shift < 8; shift++){
				Oled_Set_SCL();
				i2cWait();
				Oled_Clear_SCL();
			}
			i2cWait();
			Oled_Set_SCL();
			i2cWait();
			//Oled_Set_SDA();										// As we don't care the ACK, don't release SDA
			//i2cWait();
			//Get ACK here
			Oled_Clear_SCL();
		}

		else if(data==0xFF){
			Oled_Set_SDA();
			i2cWait();
			for(shift = 0; shift < 8; shift++){
				Oled_Set_SCL();
				i2cWait();
				Oled_Clear_SCL();
			}
			i2cWait();
			Oled_Set_SCL();
			i2cWait();
			//Oled_Set_SDA();										// As we don't care the ACK, don't release SDA
			//i2cWait();
			//Get ACK here
			Oled_Clear_SCL();
		}

		else{
			for(shift = 0; shift < 8; shift++){
				if(data & 0x80){
					Oled_Set_SDA();
					i2cWait();
				}
				else{
					Oled_Clear_SDA();
					i2cWait();
				}
				Oled_Set_SCL();
				i2cWait();
				Oled_Clear_SCL();
				data <<= 1;
			}
			i2cWait();
			Oled_Set_SCL();
			i2cWait();
			//Oled_Set_SDA();										// As we don't care the ACK, don't release SDA
			//i2cWait();
			//Get ACK here
			Oled_Clear_SCL();
		}

	}
	i2cStop();
}
#endif


// Send command in blocking mode
void write_cmd(uint8_t cmd) {
	while(oled_status==oled_sending_data);	//Wait for DMA to finish
	// Now, else we are in idle (oled_idle) or DMA wants to send a cmd (oled_sending_cmd)
#if defined OLED_SOFT_I2C
	i2cSend(&cmd,1,i2cCmd);
#elif defined OLED_SPI || defined OLED_SOFT_SPI
	Oled_Clear_CS();
	Oled_Clear_DC();
#ifdef OLED_SPI
	if(HAL_SPI_Transmit(oledDevice, &cmd, 1, 1000)!=HAL_OK){
		Error_Handler();
	}
#elif defined OLED_SOFT_SPI
	spi_send(&cmd,1);
#endif
	Oled_Set_DC();
	Oled_Set_CS();
#elif defined OLED_I2C
	if(HAL_I2C_Mem_Write(oledDevice, OLED_ADDRESS, 0x00, 1, &cmd, 1, 100)){
		 Error_Handler();
	 }
#endif

}

// Trigger DMA transfer
void update_display( void ){
		if(oled_status!=oled_idle) { return; }	// If OLED busy, skip update

		#if defined OLED_SOFT_SPI || defined OLED_SOFT_I2C
		for(uint8_t row=0;row<8;row++){
			HAL_IWDG_Refresh(&HIWDG);
			setOledRow(row);
			#if defined OLED_SOFT_SPI
			Oled_Clear_CS();
			spi_send((uint8_t *)&OledBuffer[128*row],128);
			Oled_Set_CS();
			#elif defined OLED_SOFT_I2C
			i2cSend((uint8_t *)&OledBuffer[128*row],128,i2cData);
			#endif
		}
#elif defined OLED_SPI
		HAL_SPI_TxCpltCallback(oledDevice); 	// Call the DMA callback function to send the frame
#elif defined OLED_I2C
		HAL_I2C_MemTxCpltCallback(oledDevice);	// Call the DMA callback function to send the frame
#endif
}

#if defined OLED_I2C || defined OLED_SPI
// Abort DMA transfers and reset status
void display_abort(void){
#if defined OLED_SPI
	HAL_SPI_Abort(oledDevice);
#elif defined OLED_I2C
	HAL_I2C_Abort(oledDevice);
#endif
	HAL_DMA_PollForTransfer(oledDevice->hdmatx, HAL_DMA_FULL_TRANSFER, 3000);	//Wait for DMA to finish
	oled_status=oled_idle;		// Force oled idle status
}


// Screen update for hard error handlers (crashes) not using DMA
void update_display_ErrorHandler(void){
	for(uint8_t row=0;row<8;row++){
		setOledRow(row);
#if defined OLED_SPI
		Oled_Clear_CS();
		Oled_Set_DC();
		if(HAL_SPI_Transmit(oledDevice, (uint8_t*)OledPtr + (row * 128), 128, 1000)!=HAL_OK){
			while(1);			// If error happens at this stage, just do nothing
		}
#elif defined OLED_I2C
		if(HAL_I2C_Mem_Write(oledDevice, OLED_ADDRESS, 0x40, 1, (uint8_t*)OledPtr + (row * 128), 128, 1000)!=HAL_OK){
			while(1);			// If error happens at this stage, just do nothing

		}
#endif
	}

}
#endif

// Function for drawing a pixel in display buffer
void pset(UG_U16 x, UG_U16 y, UG_COLOR c){
   unsigned int p;

   if((x>127)||(y>63)) return;
   p = y>>3; // y/8
   p = p<<7; // *128
   p +=x;

   if(c){
      OledBuffer[p] |= 1<<(y%8);
   }
   else{
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


#if defined OLED_SOFT_SPI
void ssd1306_init(DMA_HandleTypeDef *dma){
	Enable_Soft_SPI();
#elif defined OLED_SOFT_I2C
void ssd1306_init(DMA_HandleTypeDef *dma){
	Enable_Soft_I2C();
#elif defined OLED_SPI
void ssd1306_init(SPI_HandleTypeDef *device,DMA_HandleTypeDef *dma){
	oledDevice	= device;

#elif defined OLED_I2C
void ssd1306_init(I2C_HandleTypeDef *device,DMA_HandleTypeDef *dma){
	oledDevice	= device;

#endif


	oledFillDMA	= dma;
#if defined OLED_SPI || defined OLED_SOFT_SPI
	Oled_Set_CS();				// De-select
	Oled_Clear_RES();			// Set RST
	HAL_Delay(1);				//
	Oled_Set_RES();				// Release RST
#endif
	HAL_IWDG_Refresh(&HIWDG);	// Clear watchdog
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
	write_cmd(0xAF);   			// Set Display On
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
	while(oled_status!=oled_idle);				// Don't write to buffer while screen buffer is being transfered

	if(color==C_WHITE){ oled_fill=0xffffffff; }	// Fill color = white
	else{ oled_fill=0; }						// Fill color = black

	if(mode==fill_dma){							// use DMA
		 HAL_DMA_Start(oledFillDMA,(uint32_t)&oled_fill,(uint32_t)OledPtr,sizeof(OledBuffer)/sizeof(uint32_t));
		 HAL_DMA_PollForTransfer(oledFillDMA, HAL_DMA_FULL_TRANSFER, 3000);
	}
	else{										// use software
		uint32_t* bf=(uint32_t*)OledPtr;		// Pointer to oled buffer using 32bit data for faster operation
		for(uint16_t x=0;x<((128*8)/4);x++){	// Write to oled buffer
			bf[x]=oled_fill;
		}
	}
}

#if defined OLED_I2C || defined OLED_SPI
#ifdef OLED_SPI
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *device){
#elif defined OLED_I2C
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *device){
#endif
	static uint8_t OledRow=0;

	if(device != oledDevice){ return; }
	Oled_Set_CS();
	if(OledRow>7){
		OledRow=0;												// We sent the last row of the OLED buffer data
		oled_status=oled_idle;
		return;													// Return without retriggering DMA.
	}
	oled_status=oled_sending_cmd;

#ifdef OLED_SPI
	setOledRow(OledRow);
	Oled_Clear_CS();
	Oled_Set_DC();
	oled_status=oled_sending_data;								// Update status
	// Send next OLED row
	if(HAL_SPI_Transmit_DMA(oledDevice,(uint8_t *) OledPtr+(128*OledRow++), 128) != HAL_OK){
		Error_Handler();
	}
#elif defined OLED_I2C
	static uint8_t cmd[3]={0,0,0x10};

	cmd[0] = (0xB0|oledRow);

	if(systemSettings.OledFix){ cmd[1] = 0x02; }
	else{ 						cmd[1] = 0x00; }

	if(HAL_I2C_Mem_Write(oledDevice, OLED_ADDRESS, 0x00, 1, &cmd[0], 3, 50)){
		Error_Handler();
	}
	oled_status=oled_sending_data;								// Update status
	if( HAL_I2C_Mem_Write_DMA(oledDevice, OLED_ADDRESS, 0x40, 1, OledPtr+(128*oledRow++), 128)){
		Error_Handler();
	}
#endif
}
#endif

void FatalError(uint8_t type){

	#if defined OLED_I2C || defined OLED_SPI
	display_abort();
	#endif

	SetFailState(1);
	buzzer_fatal_beep();
	FillBuffer(C_BLACK,fill_soft);
	UG_FontSelect(&FONT_10X16_reduced);
	UG_SetForecolor(C_WHITE);
	UG_SetBackcolor(C_BLACK);
	switch(type){
	case 1:
		UG_PutString(2,15,"NMI HANDLER");//10
		break;
	case 2:
		UG_PutString(8,15,"HARD FAULT");//10
		break;
	case 3:
		UG_PutString(8,15,"MEM MANAGE");//10
		break;
	case 4:
		UG_PutString(11,15,"BUS FAULT");//9
		break;
	case 5:
		UG_PutString(2,15,"USAGE FAULT");//11
		break;
	default:
		break;
	}
	UG_PutString(24,31,"ERROR!!");//7

	#if defined OLED_I2C || defined OLED_SPI
	update_display_ErrorHandler();

	#elif defined OLED_SOFT_I2C || defined OLED_SOFT_SPI
	update_display();
	#endif

	uint32_t x=0;
	while(1){
		if(++x>(uint32_t)6000000){ // ~5s delay before rebooting (Interrupts no longer work here)
			NVIC_SystemReset();
		}
		HAL_IWDG_Refresh(&HIWDG);
	}

}

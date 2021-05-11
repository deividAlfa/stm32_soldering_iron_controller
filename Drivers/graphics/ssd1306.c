#include "ssd1306.h"
#include "settings.h"
#include "buzzer.h"
#include "iron.h"
#include "gui.h"

oled_t oled = {
	ptr: 		&oled.buffer[0],
	status:		oled_idle,
	row:		0,
#if defined OLED_SPI || defined OLED_I2C
	device:		NULL,
#endif
	fillDMA:	NULL,
};
static uint8_t lastContrast;



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
void i2cWait(void){													// This is pretty adjusted for max speed without errors. Might need more time in specific boards/displays
	asm(	"nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n\
			 nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n\
			 nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
}
void i2cStart(void){												// 	Start condition, SDA transition to low with SCL high
	Oled_Set_SCL();
	i2cWait();
	Oled_Clear_SDA();
	i2cWait();
	Oled_Clear_SCL();
	i2cWait();
}
void i2cStop(void){													// 	Stop condition, SCL transition to high with SDA low
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
		//Oled_Set_SDA();											      // As we don't care about the ACK, don't release SDA
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
		if( (data==0)||(data==0xFF)){								// If data 0 or 0xff, we don't have to toggle data line, send the data fast
			if(data==0){											        // Just toggling clock line
				Oled_Clear_SDA();
			}
			else{
				Oled_Set_SDA();
			}
			i2cWait();
			for(shift = 0; shift < 8; shift++){
				Oled_Set_SCL();
				i2cWait();
				Oled_Clear_SCL();
			}
			i2cWait();
			Oled_Set_SCL();
			i2cWait();
			//Oled_Set_SDA();										      // As we don't care about the ACK, don't release SDA
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
			//Oled_Set_SDA();										      // As we don't care about the ACK, don't release SDA
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
#if defined OLED_SPI || defined OLED_I2C
	while(oled.status==oled_busy);              // Wait for DMA to finish
#endif

#if defined OLED_SPI || defined OLED_SOFT_SPI
	#ifdef USE_CS
	Oled_Clear_CS();
	#endif

	#ifdef USE_DC
	Oled_Clear_DC();
	#endif

	#ifdef OLED_SPI
	HAL_StatusTypeDef err=HAL_SPI_Transmit(oled.device, &cmd, 1, 10);
	if(err!=HAL_OK){
		Error_Handler();
	}
	#elif defined OLED_SOFT_SPI
	spi_send(&cmd,1);
	#endif

	#ifdef USE_DC
	Oled_Set_DC();
	#endif

	#ifdef USE_CS
	Oled_Set_CS();
	#endif

#elif defined OLED_SOFT_I2C
	i2cSend(&cmd,1,i2cCmd);

#elif defined OLED_I2C
	if(HAL_I2C_Mem_Write(oled.device, OLED_ADDRESS, 0x00, 1, &cmd, 1, 100)){
		 Error_Handler();
	 }
#endif

}

void update_display( void ){
		if(oled.status!=oled_idle) { return; }	      // If OLED busy, skip update
		if(oled.row!=0){ Error_Handler(); }

		#if defined OLED_SOFT_SPI || defined OLED_SOFT_I2C
		for(uint8_t row=0;row<8;row++){
			HAL_IWDG_Refresh(&hiwdg);
			setOledRow(row);

			#if defined OLED_SOFT_SPI

			#ifdef USE_CS
			Oled_Clear_CS();
			#endif

			#ifdef USE_DC
			Oled_Set_DC();
			#endif

			spi_send((uint8_t *)&oled.buffer[128*row],128);

			#ifdef USE_CS
			Oled_Set_CS();
			#endif

			#elif defined OLED_SOFT_I2C
			i2cSend((uint8_t *)&oled.buffer[128*row],128,i2cData);
			#endif

		}
#elif defined OLED_SPI
		HAL_SPI_TxCpltCallback(oled.device); 	        // Call the DMA callback function to start sending the frame

#elif defined OLED_I2C
		HAL_I2C_MemTxCpltCallback(oled.device);	      // Call the DMA callback function to start sending the frame
#endif
}

#if defined OLED_SOFT_SPI || defined OLED_SOFT_I2C
void setOledRow(uint8_t row){
	write_cmd(0xB0|row);									          // Set the OLED Row address
	write_cmd(systemSettings.settings.OledOffset);
	write_cmd(0x10);
}
#endif

void setContrast(uint8_t value) {
	write_cmd(0x81);                                // Set Contrast Control
	write_cmd(value);                               // Default => 0xFF
	lastContrast = value;
}

uint8_t getContrast(void) {
	return lastContrast;
}

#if defined OLED_SOFT_SPI
void ssd1306_init(DMA_HandleTypeDef *dma){
	Enable_Soft_SPI();

#elif defined OLED_SOFT_I2C
void ssd1306_init(DMA_HandleTypeDef *dma){
	Enable_Soft_I2C();

#elif defined OLED_SPI
void ssd1306_init(SPI_HandleTypeDef *device,DMA_HandleTypeDef *dma){
	oled.device	= device;

#elif defined OLED_I2C
void ssd1306_init(I2C_HandleTypeDef *device,DMA_HandleTypeDef *dma){
	oled.device	= device;

#endif


	oled.fillDMA= dma;

#if defined OLED_SPI || defined OLED_SOFT_SPI
	#ifndef USE_DC
	#error Mandatory OLED DC Pin not configured
	#endif

	#ifdef USE_CS
	Oled_Set_CS();				  // De-select
	#endif

	#ifdef USE_RST
	Oled_Clear_RES();			  // Set RST
	HAL_Delay(1);				    // Delay
	Oled_Set_RES();				  // Release RST
	#endif

#endif

#if defined OLED_I2C || defined OLED_SOFT_I2C
	#ifdef USE_CS
	Oled_Clear_CS();			  // Unused in I2C mode, set low
	#endif

	#ifdef USE_DC
	Oled_Clear_DC();			  // DC is the LSB address in I2C mode
	#endif

	#ifdef USE_RST
	Oled_Clear_RES();			  // Set RST
	HAL_Delay(1);				    //
	Oled_Set_RES();				  // Release RST
	#endif

#endif
	systemSettings.settings.OledOffset = 2;		// Set by default while system settings are not loaded
	HAL_IWDG_Refresh(&hiwdg);	              // Clear watchdog
	HAL_Delay(100);				    // 100mS wait for internal initialization

	write_cmd(0xAE);  			  // Display Off
	write_cmd(0xD5);         	// Set Display Clock Divide Ratio / Oscillator Frequency
	write_cmd(0xF0);      		// Set max framerate
	write_cmd(0xA8);         	// Set Multiplex Ratio
	write_cmd(0x3F);         	// Default => 0x3F (1/64 Duty)
	write_cmd(0xD3);         	// Set Display Offset
	write_cmd(0x00);         	// Default => 0x00
	write_cmd(0x40|0x00);   	// Set Display Start Line
	write_cmd(0x20);         	// Set Memory Addressing Mode
	write_cmd(0x02);         	// Default => 0x02
	write_cmd(0xA0|0x01);   	// Set Segment Re-Map
	write_cmd(0xC0|0x08);   	// Set COM Output Scan Direction
	write_cmd(0xDA);         	// Set COM Pins Hardware Configuration
	write_cmd(0x02|0x10);   	// Default => 0x12 (0x10)
	setContrast(0xFF);			  // Init in max contrast
	write_cmd(0xD9);         	// Set Pre-Charge Period
	write_cmd(0x22);         	// Default => 0x22 (2 Display Clocks [Phase 2] / 2 Display Clocks [Phase 1])

	write_cmd(0xDB);         	// Set VCOMH Deselect Level
	write_cmd(0x30);         	// Default => 0x20 (0.77*VCC)

	write_cmd(0xA4|0x00);   	// Set Entire Display On/Off
	write_cmd(0xA6|0x00);   	// Set Inverse Display On/Off
	write_cmd(0x8D);         	// Set Charge Pump command
	write_cmd(0x14);   			  // Enable charge pump
	write_cmd(0x33);         	// Charge pump to 9V
    FillBuffer(BLACK,fill_dma);	// Clear buffer
    update_display();			  // Update display CGRAM

	#if defined OLED_SPI || defined OLED_I2C
	while(oled.status!=oled_idle);  // Wait for DMA completion
	#endif
	write_cmd(0xAF);   			  // Set Display On
}

/*
*	Clear buffer with 32bit-transfer for fast filling (ensure that Oled buffer is 32-bit aligned!)
* 	128 * 8 = 1KB, / 4byte DMA txfer = 256 clock cycles (in theory)
* 	Args:
* 			color: 	0 = black, 1 = white
* 			mode: 	0 = Use software(fail-safe), 1= Use DMA (normal operation)
*/

void FillBuffer(bool color, bool mode){
	uint32_t fillVal;
#if defined OLED_SPI || defined OLED_I2C
	while(oled.status!=oled_idle);          // Don't write to buffer while screen buffer is being transfered
#endif

	if(color==WHITE){ fillVal=0xffffffff; }	// Fill color = white
	else{ fillVal=0; }						          // Fill color = black

	if(mode==fill_dma){							        // use DMA
		 HAL_DMA_Start(oled.fillDMA,(uint32_t)&fillVal,(uint32_t)oled.ptr,sizeof(oled.buffer)/sizeof(uint32_t));
		 HAL_DMA_PollForTransfer(oled.fillDMA, HAL_DMA_FULL_TRANSFER, 3000);
	}
	else{										                // use software
		uint32_t* bf=(uint32_t*)oled.ptr;		  // Pointer to oled buffer using 32bit data for faster operation
		for(uint16_t x=0;x<sizeof(oled.buffer)/sizeof(uint32_t);x++){	// Write to oled buffer
			bf[x]=fillVal;
		}
	}
}

#if defined OLED_I2C || defined OLED_SPI

// Abort DMA transfers and reset status
void display_abort(void){
#if defined OLED_SPI
	if(oled.device!=NULL){
		HAL_SPI_Abort(oled.device);
	}
#elif defined OLED_I2C
	if(oled.device!=NULL){
		HAL_I2C_Abort(oled.device);
	}
#endif

	__HAL_UNLOCK(oled.device);
	HAL_DMA_PollForTransfer(oled.device->hdmatx, HAL_DMA_FULL_TRANSFER, 3000);	//Wait for DMA to finish
	oled.status=oled_idle;		// Force oled idle status
}


// Screen update for hard error handlers (crashes) not using DMA
void update_display_ErrorHandler(void){
	for(uint8_t row=0;row<8;row++){

		uint8_t cmd[3]={
			0xB0|row,
			systemSettings.settings.OledOffset,
			0x10
		};

		#ifdef OLED_SPI

		#ifdef USE_CS
		Oled_Clear_CS();
		#endif

		#ifdef USE_DC
		Oled_Clear_DC();
		#endif

		if(HAL_SPI_Transmit(oled.device, cmd, 3, 50)){
			while(1){						                        // If error happens at this stage, just do nothing
				HAL_IWDG_Refresh(&hiwdg);
			}
		}
		#ifdef USE_DC
		Oled_Set_DC();
		#endif

		if(HAL_SPI_Transmit(oled.device, (uint8_t*)oled.ptr + (row * 128), 128, 1000)!=HAL_OK){
			while(1){						                        // If error happens at this stage, just do nothing
				HAL_IWDG_Refresh(&hiwdg);
			}
		}
		#elif defined OLED_I2C
		if(HAL_I2C_Mem_Write(oled.device, OLED_ADDRESS, 0x00, 1, cmd, 3, 50)){
			while(1){						                        // If error happens at this stage, just do nothing
				HAL_IWDG_Refresh(&hiwdg);
			}
		}
		if(HAL_I2C_Mem_Write(oled.device, OLED_ADDRESS, 0x40, 1, (uint8_t*)oled.ptr + (row * 128), 128, 1000)!=HAL_OK){
			while(1){						                        // If error happens at this stage, just do nothing
				HAL_IWDG_Refresh(&hiwdg);
			}
		}
		#endif
	}
	#if defined OLED_SPI && defined USE_CS
	Oled_Set_CS();
	#endif
}


#ifdef OLED_SPI
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *device){
#elif defined OLED_I2C
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *device){

#endif

	if(device == oled.device){
		HAL_DMA_PollForTransfer(oled.device->hdmatx, HAL_DMA_FULL_TRANSFER, 100);	//Wait for DMA to finish
		if(oled.row>7){

			#if defined OLED_SPI && defined USE_CS
			Oled_Set_CS();											            // Release CS
			#endif

			oled.row=0;												              // Reset row position
			oled.status=oled_idle;
			return;													                // Return without retriggering DMA.
		}

		uint8_t cmd[3]={
			0xB0|oled.row,
			systemSettings.settings.OledOffset,
			0x10
		};
		oled.status=oled_busy;

#ifdef OLED_SPI

		#ifdef USE_CS
		Oled_Clear_CS();
		#endif

		#ifdef USE_DC
		Oled_Clear_DC();

		#endif
		uint8_t try =3;
		while(try){
			if(HAL_SPI_Transmit(oled.device, cmd, 3, 50)==HAL_OK){									// Send row command in blocking mode
				break;
			}
			else{
				display_abort();
				oled.status=oled_busy;
				try--;
			}
		}
		if(try==0){
			Error_Handler();
		}

		#ifdef USE_DC
		Oled_Set_DC();
		#endif

		if(HAL_SPI_Transmit_DMA(oled.device,(uint8_t *) oled.ptr+((uint16_t)128*oled.row), 128)!= HAL_OK){    // Send row data in DMA interrupt mode
			Error_Handler();
		}

#elif defined OLED_I2C
		if(HAL_I2C_Mem_Write(oled.device, OLED_ADDRESS, 0x00, 1, cmd, 3, 50)){			                          // Send row command in blocking mode
			Error_Handler();
		}
		oled.status=oled_sending_data;
		if( HAL_I2C_Mem_Write_DMA(oled.device, OLED_ADDRESS, 0x40, 1, oled.ptr+(128*oled.row), 128)){	        // Send row data in DMA interrupt mode
			Error_Handler();
		}
#endif

		oled.row++;
	}
}

#endif
void FatalError(uint8_t type){

	#if defined OLED_I2C || defined OLED_SPI
	display_abort();
	#endif

	SetFailState(setError);
	buzzer_fatal_beep();
	Diag_init();
	switch(type){
		case error_NMI:
			putStrAligned("NMI HANDLER", 0, align_center);
			break;
		case error_HARDFAULT:
			putStrAligned("HARD FAULT", 0, align_center);
			break;
		case error_MEMMANAGE:
			putStrAligned("MEM MANAGE", 0, align_center);
			break;
		case error_BUSFAULT:
			putStrAligned("BUS FAULT", 0, align_center);
			break;
		case error_USAGEFAULT:
			putStrAligned("USAGE FAULT", 0, align_center);
			break;
		case error_RUNAWAY25:
		case error_RUNAWAY50:
		case error_RUNAWAY75:
		case error_RUNAWAY100:
		{
			uint8_t level = 25 * ((type - error_RUNAWAY25)+1);
			char strRunawayLevel[8];
			sprintf(strRunawayLevel,">%u\260C\n",level);
			putStrAligned("TEMP RUNAWAY", 0, align_center);
			putStrAligned(strRunawayLevel, 15, align_center);
			break;
		}
		case error_RUNAWAY500:
			putStrAligned("EXCEEDED", 0, align_center);
			putStrAligned("500\260C!", 15, align_center);
			break;
		case error_RUNAWAY_UNKNOWN:
			putStrAligned("TEMP RUNAWAY", 0, align_center);
			putStrAligned("UNDEFINED!", 15, align_center);
			break;
		default:
			putStrAligned("UNKNOWN ERROR", 0, align_center);
			break;
	}
	putStrAligned("SYSTEM HALTED", 35, align_center);
	putStrAligned("Use btn to reset", 50, align_center);

	#if defined OLED_I2C || defined OLED_SPI
	update_display_ErrorHandler();

	#elif defined OLED_SOFT_I2C || defined OLED_SOFT_SPI
	update_display();
	#endif

	Reset_onError();
}

void putStrAligned(char* str, uint8_t y, AlignType align){

	if(align==align_left){
		u8g2_DrawStr(&u8g2, 0, y, str);
	}
	else{
		uint8_t len = u8g2_GetStrWidth(&u8g2, str);
		if(align==align_center){
			u8g2_DrawStr(&u8g2, ((OledWidth-1)-len)/2, y, str);
		}
		else if(align==align_right){
			u8g2_DrawStr(&u8g2, (OledWidth-1)-len, y, str);
		}
	}
}
void Reset_onError(void){
	while(BUTTON_input()){							    // Wait until the button is pressed
		for(uint16_t i=0;i<50000;i++);				// Small delay
		HAL_IWDG_Refresh(&hiwdg);					    // Clear watchdog
	}
	while(!BUTTON_input()){							    // Wait until the button is released
		for(uint16_t i=0;i<50000;i++);				// Small delay
		HAL_IWDG_Refresh(&hiwdg);					    // Clear watchdog
	}
	NVIC_SystemReset();							        // Reset system
}

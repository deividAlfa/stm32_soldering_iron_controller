#include "ssd1306.h"
#include "settings.h"
#include "buzzer.h"
#include "iron.h"
#include "gui.h"


#ifdef __BASE_FILE__
#undef __BASE_FILE__
#define __BASE_FILE__ "ssd1306.c"
#endif

oled_t oled;

static uint8_t lastContrast;
static uint8_t powerStatus;
static uint8_t faultMode;

// Command size in bytes, data

#if defined ST7565
uint8_t disp_init[] = { // Initialization for ST7565R (Source: u8g2)
    1, 0xE2,            // soft reset
    1, 0xAE,            // display off
    1, 0x40,            // set display start line to 0
    1, 0xA0 | 1,        // ADC set to reverse
    1, 0xC8,            // Common output mode
    1, 0xA6,            // display normal, bit val 0: LCD pixel off.
    1, 0xA2,            // LCD bias 1/9
    1, 0x2F,            // all power  control circuits on (regulator, booster and follower)
    2, 0xF8, 0x00,      // set booster ratio to 4x
    1, 0x27,            // set V0 voltage resistor ratio to max
    2, 0x81, 0x07,      // set contrast, contrast value, EA default: 0x016
    1, 0xAE,            // display off
    1, 0xA5             // All pixel on (Sleep mode)
};
#elif defined SSD1306
uint8_t disp_init[] = { // Initialization for SH1106 / SSD1306 / SSD1309
    1, 0xAE,            // Display Off
    1, 0xD5,            // Set Display Clock Divide Ratio / Oscillator Frequency
    1, 0xF0,            // Set max framerate
    2, 0xA8, 0x3F,      // Set Multiplex Ratio, 0x3F (1/64 Duty)
    2, 0xD3, 0x00,      // Set Display Offset, 0x00
    1, 0x40,            // Set Display Start Line
    2, 0x20, 0x02,      // Set Memory Addressing Mode (0x02)
    1, 0xA1,            // Set Segment Re-Map
    1, 0xC8,            // Set COM Output Scan Direction
    2, 0xDA, 0x12,      // Set COM Pins Hardware Configuration Default => 0x12 (0x10)
    2, 0x81, 0xFF,      // Init in max contrast
    2, 0xD9, 0x44,      // Set Pre-Charge Period, 0x44 (4 Display Clocks [Phase 2] / 4 Display Clocks [Phase 1])
    2, 0xDB, 0x3C,      // Set VCOMH Deselect Level, 0x3C (0.84*VCC)
    1, 0xA4,            // Set Entire Display Off
    1, 0xA6,            // Set Inverse Display Off
    2, 0x8D, 0x14,      // Set Charge Pump comm, Enable charge pump
    1, 0x33             // Charge pump to 9V
};
#else
#error "No display defined!"
#endif

// Silicon bug workaround for STM32F103 as ST document ES093 rev 7

#if defined OLED_I2C && defined OLED_DEVICE
// Silicon bug workaround for STM32F103, force I2C RCC reset and re-init
void i2c_workaround(void){
#ifdef I2C1
  __HAL_RCC_I2C1_FORCE_RESET();
#endif
#ifdef I2C2
  __HAL_RCC_I2C2_FORCE_RESET();
#endif
  HAL_Delay(10);
#ifdef I2C1
  __HAL_RCC_I2C1_RELEASE_RESET();
#endif
#ifdef I2C2
  __HAL_RCC_I2C2_RELEASE_RESET();
#endif
  __HAL_RESET_HANDLE_STATE(oled.device);
  __HAL_RCC_DMA1_CLK_ENABLE();
  if (HAL_I2C_Init(oled.device) != HAL_OK){
    Error_Handler();
  }
}
#endif


// This delays are made for 36MHz (Ksger v2 software i2c). If increasing the cpu frequency, also increase the nop count
#if !defined OLED_DEVICE  || (defined OLED_DEVICE && defined I2C_TRY_HW)
__attribute__ ((noinline)) void bit_delay(void){                  // Intended no inline to add further delay and reduce nops
  asm("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
}
#endif

#if (defined OLED_SPI && !defined OLED_DEVICE)  || \
    (defined OLED_I2C && (!defined OLED_DEVICE  || (defined OLED_DEVICE && defined I2C_TRY_HW)))
void enable_soft_mode(void){
  GPIO_InitTypeDef GPIO_InitStruct = {0};

#if defined HW_SCL_Pin && defined HW_SDA_Pin
  GPIO_InitStruct.Mode =  GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull =  GPIO_NOPULL;

  /*Configure GPIO pins : SDA_Pin */
  GPIO_InitStruct.Pin =   HW_SDA_Pin;
  HAL_GPIO_Init(HW_SDA_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SDA_Pin */
  GPIO_InitStruct.Pin =   HW_SCL_Pin;
  HAL_GPIO_Init(HW_SCL_GPIO_Port, &GPIO_InitStruct);
#endif

#if defined SW_SDA_Pin && defined SW_SCL_Pin
  Oled_Set_SDA();
  Oled_Set_SCL();
#ifdef OLED_I2C
  GPIO_InitStruct.Pull =  GPIO_PULLUP;
  GPIO_InitStruct.Mode =  GPIO_MODE_OUTPUT_OD;
#else
  GPIO_InitStruct.Mode =  GPIO_MODE_OUTPUT_PP;
#endif
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  /*Configure GPIO pins : SCL_Pin */
  GPIO_InitStruct.Pin =   SW_SCL_Pin;
  HAL_GPIO_Init(SW_SCL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SDA_Pin */
  GPIO_InitStruct.Pin =   SW_SDA_Pin;
  HAL_GPIO_Init(SW_SDA_GPIO_Port, &GPIO_InitStruct);
#endif

#ifdef OLED_I2C
  // Reset the bus
  i2cStart();
  Oled_Set_SDA();
  bit_delay();
  for(uint8_t c=0;c<9;c++){
    Oled_Set_SCL();
    bit_delay();
    Oled_Clear_SCL();
    bit_delay();
  }
  i2cStop();
#endif
}

void disable_soft_i2c(void){
  GPIO_InitTypeDef GPIO_InitStruct = {0};

#if defined SW_SCL_Pin && defined SW_SDA_Pin
  GPIO_InitStruct.Mode =  GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull =  GPIO_NOPULL;

  GPIO_InitStruct.Pin =   SW_SCL_Pin;
  HAL_GPIO_Init(SW_SCL_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin =   SW_SDA_Pin;
  HAL_GPIO_Init(SW_SDA_GPIO_Port, &GPIO_InitStruct);
#endif

#if defined HW_SCL_Pin && defined HW_SDA_PIN
  GPIO_InitStruct.Mode =  GPIO_MODE_AF_OD;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  GPIO_InitStruct.Pin =   HW_SDA_Pin;
  HAL_GPIO_Init(HW_SDA_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin =   HW_SCL_Pin;
  HAL_GPIO_Init(HW_SCL_GPIO_Port, &GPIO_InitStruct);
#endif
}

#endif

#if (defined OLED_SPI && !defined OLED_DEVICE) || (defined OLED_I2C && (!defined OLED_DEVICE  || (defined OLED_DEVICE && defined I2C_TRY_HW)))

void clock_byte(uint8_t b){
  if((!b) || (b==0xFF)){                        // Sends "fixed" bytes much faster as skips SDA toggling
    Oled_Bit(b);
    Oled_Clock();
    Oled_Clock();
    Oled_Clock();
    Oled_Clock();
    Oled_Clock();
    Oled_Clock();
    Oled_Clock();
  }
  else{                                         // Unrolled byte loop boost the performance noticeably
    Oled_Bit(b & 0x80);
    Oled_Bit(b & 0x40);
    Oled_Bit(b & 0x20);
    Oled_Bit(b & 0x10);
    Oled_Bit(b & 0x08);
    Oled_Bit(b & 0x04);
    Oled_Bit(b & 0x02);
    Oled_Bit(b & 0x01);
  }
#if (defined OLED_I2C)
  Oled_Set_SDA();
  Oled_Clock();                     // Ack Nack pulse (we don't read it)
#endif
}

void bitbang_write(uint8_t* bf, uint16_t count, uint8_t mode){
#if (defined OLED_SPI)
  if(mode==modeData)
    Oled_Set_DC();
  else
    Oled_Clear_DC();
#elif (defined OLED_I2C)
  i2cStart();
  i2cBegin(mode);
#endif
  while(count--){
    uint8_t data = *bf++;
    clock_byte(data);
  }
  #if (defined OLED_I2C)
  i2cStop();
  #endif
}
#endif


#if (defined OLED_I2C && !defined OLED_DEVICE) || (defined OLED_DEVICE && defined I2C_TRY_HW)

// This sw i2c driver is extremely timing optimized, done specially for ksger v2.1 and compatibles running at 36MHz.
// Hacks clock low time using the slow rise time (i2c pullup resistors) as the delay.
// Will start failing if the core runs faster than 44-48MHz because of the tight timing.
void i2cBegin(uint8_t mode){
  uint8_t b[] = { OLED_ADDRESS, mode };
  for(uint8_t i=0; i<2; i++){
      clock_byte(b[i]);
  }
}

void i2cStart(void){                                      // Start condition, SDA transition to low with SCL high
  Oled_Set_SCL();
  bit_delay();
  Oled_Clear_SDA();
  bit_delay();
  Oled_Clear_SCL();
  bit_delay();
}
void i2cStop(void){                                       // Stop condition, SCL transition to high with SDA low
  Oled_Clear_SDA();
  bit_delay();
  Oled_Set_SCL();
  bit_delay();
  Oled_Set_SDA();
  bit_delay();
}


#endif

// Send data
void lcd_write(uint8_t *data, uint16_t count, uint8_t mode){
  while(oled.status==oled_busy);                          // Wait for DMA to finish

#if defined OLED_SPI
  #ifdef USE_CS
  Oled_Clear_CS();
  #endif

  #ifdef OLED_DEVICE
  if (mode==modeData)
    Oled_Set_DC();
  else
    Oled_Clear_DC();
  if(HAL_SPI_Transmit(oled.device, data, count, HAL_MAX_DELAY) != HAL_OK){
    if(!faultMode)
      Error_Handler();
    else
      Reset_onError();          // We're already on ErrorHandler, can't do anymore!
  }
  #else
  bitbang_write(data, count, mode);
  #endif

  #ifdef USE_CS
  Oled_Set_CS();
  #endif


#elif defined OLED_I2C
  if(oled.use_sw){
    bitbang_write(data, count, mode);
  }
#if defined OLED_DEVICE
#if defined I2C_TRY_HW
  else{
#endif
    if(HAL_I2C_Mem_Write(oled.device, OLED_ADDRESS, mode, 1, data, count, HAL_MAX_DELAY)!=HAL_OK){
      if(!faultMode)
        Error_Handler();
      else
        Reset_onError();          // We're already on ErrorHandler, can't do anymore!
    }
#if defined I2C_TRY_HW
  }
#endif
#endif
#endif
}


void update_display( void ){
    if(oled.status!=oled_idle) { return; }                // If OLED busy, skip update
    if(oled.row!=0){ Error_Handler(); }                   // If called while already updating, something went wrong!

#if !defined OLED_DEVICE || (defined OLED_DEVICE && defined I2C_TRY_HW)
    if(oled.use_sw){
      for(uint8_t row=0;row<8;row++){
        HAL_IWDG_Refresh(&hiwdg);
        setOledRow(row);
        lcd_write(&oled.buffer[128*row], 128, modeData);
      }
      return;
    }
#endif
    oled.status=oled_busy;
#if defined OLED_SPI && defined OLED_DEVICE
    HAL_SPI_TxCpltCallback(oled.device);                  // Call the DMA callback function to start sending the frame

#elif defined OLED_I2C && defined OLED_DEVICE
    HAL_I2C_MemTxCpltCallback(oled.device);               // Call the DMA callback function to start sending the frame
#endif
}

void setOledRow(uint8_t row){
  uint8_t cmd[] = { 0xB0|row, systemSettings.settings.OledOffset, 0x10 };
  lcd_write(cmd, sizeof(cmd), modeCmd);
}

void setOledPower(uint8_t power){
  powerStatus = power;
#ifdef  ST7565
  uint8_t cmd[2];
  cmd[0] = ((power==enable) ? 0xA4 : 0xAF );  // All pixel Off, Display On
  cmd[1] = ((power==enable) ? 0xAE : 0xA5 );  // Display Off, All pixel On
#else
  uint8_t cmd[1] = { ((power==enable) ? 0xAF : 0xAE ) }; // Display On, Display Off
#endif
  lcd_write(cmd, sizeof(cmd), modeCmd);
}
uint8_t getOledPower(void){
  return powerStatus;
}

void setContrast(uint8_t value) {
#ifndef  ST7565
  uint8_t cmd [] = { 0x81, value };
  lcd_write(cmd, sizeof(cmd), modeCmd);
#endif
  lastContrast = value;
}

uint8_t getContrast(void) {
  return lastContrast;
}

#if !defined OLED_DEVICE
void ssd1306_init(DMA_HandleTypeDef *dma){
  enable_soft_mode();
  oled.use_sw=1;
#elif defined OLED_SPI && defined OLED_DEVICE
void ssd1306_init(SPI_HandleTypeDef *device,DMA_HandleTypeDef *dma){
  oled.device  = device;
#elif defined OLED_I2C && defined OLED_DEVICE
void ssd1306_init(I2C_HandleTypeDef *device,DMA_HandleTypeDef *dma){
  oled.device  = device;
  i2c_workaround();
#endif
  oled.fillDMA= dma;
#if defined OLED_SPI
#ifdef USE_CS
  Oled_Set_CS();          // De-select
#endif
#elif defined OLED_I2C
  #ifdef USE_CS
  Oled_Clear_CS();          // Unused in I2C mode, set low
  #endif

  #ifdef USE_DC
  Oled_Clear_DC();          // DC is the LSB address in I2C mode
  #endif
#endif

#ifdef USE_RST
  Oled_Clear_RES();       // Set RST
  HAL_Delay(10);          // Delay
  Oled_Set_RES();         // Release RST
#endif

  systemSettings.settings.OledOffset = 2;         // Set by default while system settings are not loaded
  HAL_IWDG_Refresh(&hiwdg);                       // Clear watchdog
  HAL_Delay(200);                                 // 200mS wait for internal initialization

#if defined OLED_I2C && defined OLED_DEVICE && defined I2C_TRY_HW
  oled.use_sw=1;
  // Check if OLED is connected to hardware I2C
  for(uint8_t try=0; try<5; try++){
    uint8_t data;
    uint8_t res = HAL_I2C_Mem_Read(oled.device, OLED_ADDRESS, 0x00, 1, &data, 1, 10);
    if(res==HAL_OK){
      oled.use_sw=0;                              // Detected, enable hw
      break;
    }
    HAL_IWDG_Refresh(&hiwdg);                     // Clear watchdog
    HAL_Delay(10);                                // Failed, wait before next try
  }
  if(oled.use_sw){                                // Display not detected
    enable_soft_mode();                            // Set sw mode
  }
#endif

  for(uint16_t i=0;i<sizeof(disp_init);){
    lcd_write(&disp_init[i+1], disp_init[i], modeCmd);
    i+=disp_init[i]+1;
  }
  FillBuffer(BLACK,fill_dma); // Clear buffer
  update_display();           // Update display RAM
  setOledPower(enable);
}

/*
*  Clear buffer with 32bit-transfer for fast filling (ensure that Oled buffer is 32-bit aligned!)
*   128 * 8 = 1KB, / 4byte DMA txfer = 256 clock cycles (in theory)
*   Args:
*       color:   0 = black, 1 = white
*       mode:   0 = Use software(fail-safe), 1= Use DMA (normal operation)
*/

void FillBuffer(bool color, bool mode){
  uint32_t fillVal = ((color==WHITE) ? 0xFFFFFFFF : 0x00000000 );  // Fill color = white : black
  while(oled.status!=oled_idle);              // Don't write to buffer while screen buffer is being transfered
  if(mode==fill_dma){                         // use DMA
     if(HAL_DMA_Start(oled.fillDMA,(uint32_t)&fillVal,(uint32_t)oled.buffer,sizeof(oled.buffer)/sizeof(uint32_t))!=HAL_OK){
	    	Error_Handler();
     }
     HAL_DMA_PollForTransfer(oled.fillDMA, HAL_DMA_FULL_TRANSFER, 5);
  }
  else{                                       // use software
    uint32_t *bf = (uint32_t*)oled.buffer;         // Pointer to oled buffer using 32bit data for faster operation
    for(uint16_t x=0;x<sizeof(oled.buffer)/sizeof(uint32_t);x++){  // Write to oled buffer
      bf[x]=fillVal;
    }
  }
}

#if (defined OLED_I2C || defined OLED_SPI) && defined OLED_DEVICE

// Abort DMA transfers and reset status
void display_abort(void){
  if(oled.device!=NULL){
#if defined OLED_SPI
    HAL_SPI_Abort(oled.device);
#elif defined OLED_I2C
    HAL_I2C_Master_Abort_IT(oled.device, 0);
#endif
    __HAL_UNLOCK(oled.device);
    HAL_DMA_PollForTransfer(oled.device->hdmatx, HAL_DMA_FULL_TRANSFER, 100);  // Wait for DMA to finish
  }
  oled.status=oled_idle;                                                      // Force oled idle status
}


// Screen update for hard error handlers (crashes) not using DMA
void update_display_ErrorHandler(void){
  for(uint8_t row=0;row<8;row++){
    setOledRow(row);
    lcd_write(&oled.buffer[128*row], 128, modeData);
  }
}


#ifdef OLED_SPI
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *device){
#elif defined OLED_I2C
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *device){

#endif

  if(device == oled.device){
    if(HAL_DMA_GetState(oled.device->hdmatx)!=HAL_DMA_STATE_READY){           // If DMA busy
      HAL_DMA_PollForTransfer(oled.device->hdmatx, HAL_DMA_FULL_TRANSFER, 10);
    }
    if(oled.row>7){

      #if defined OLED_SPI && defined USE_CS
      Oled_Set_CS();                                    // Release CS
      #endif

      oled.row=0;                                       // Reset row position
      oled.status=oled_idle;
      return;                                           // Return without retriggering DMA.
    }

    uint8_t cmd[3]={
        0xB0|oled.row,
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
    uint8_t try =3;
    while(try){
      if(HAL_SPI_Transmit(oled.device, cmd, sizeof(cmd), 50)==HAL_OK){      // Send row command in blocking mode
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

    if(HAL_SPI_Transmit_DMA(oled.device, &oled.buffer[128*oled.row], 128)!= HAL_OK){      // Send row data in DMA interrupt mode
      Error_Handler();
    }

#elif defined OLED_I2C
    uint8_t try =3;
		while(try){
      if(HAL_I2C_Mem_Write(oled.device, OLED_ADDRESS, modeCmd, 1, cmd, sizeof(cmd), 50)==HAL_OK){
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
    if(HAL_I2C_Mem_Write_DMA(oled.device, OLED_ADDRESS, modeData, 1, &oled.buffer[128*oled.row], 128)!=HAL_OK){
      Error_Handler();
    }
#endif

    oled.row++;
  }
}

#endif

void FatalError(uint8_t type){
  faultMode = 1;
 uint8_t lang = systemSettings.settings.language;
 if(lang>(LANGUAGE_COUNT-1)){
   lang=lang_english;
 }

  #if (defined OLED_I2C || defined OLED_SPI) && defined OLED_DEVICE
  if(!oled.use_sw){
    display_abort();
  }
  #endif

  setSafeMode(enable);
  buzzer_fatal_beep();
  Oled_error_init();
  switch(type){
    case error_FLASH:
      putStrAligned("FLASH ERROR", 0, align_center);
      break;
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
      putStrAligned(strings[lang].ERROR_RUNAWAY, 0, align_center);
      putStrAligned(strRunawayLevel, 15, align_center);
      break;
    }
    case error_RUNAWAY500:
      putStrAligned(strings[lang].ERROR_EXCEEDED, 0, align_center);
      putStrAligned("500\260C!", 15, align_center);
      break;

    default:
      putStrAligned(strings[lang].ERROR_UNKNOWN, 0, align_center);
      break;
  }
  putStrAligned(strings[lang].ERROR_SYSTEM_HALTED, 35, align_center);
  putStrAligned(strings[lang].ERROR_BTN_RESET, 50, align_center);

  #if (defined OLED_I2C || defined OLED_SPI) && defined OLED_DEVICE
  if(!oled.use_sw){
    update_display_ErrorHandler();
  }
  #if defined OLED_I2C && defined I2C_TRY_HW
  else{
    update_display();
  }
  #endif

  #else
  update_display();
  #endif

  Reset_onError();
}

void putStrAligned(char* str, uint8_t y, AlignType align){

  if(align==align_left){
    u8g2_DrawUTF8(&u8g2, 0, y, str);
  }
  else{
    uint8_t len = u8g2_GetUTF8Width(&u8g2, str);
    if(align==align_center){
      u8g2_DrawUTF8(&u8g2, ((OledWidth-1)-len)/2, y, str);
    }
    else if(align==align_right){
      u8g2_DrawUTF8(&u8g2, (OledWidth-1)-len, y, str);
    }
  }
}
void Reset_onError(void){
  __disable_irq();
  while(BUTTON_input()){                    // Wait until the button is pressed
    for(uint16_t i=0;i<50000;i++);          // Small delay
    HAL_IWDG_Refresh(&hiwdg);               // Clear watchdog
  }
  while(!BUTTON_input()){                   // Wait until the button is released
    for(uint16_t i=0;i<50000;i++);          // Small delay
    HAL_IWDG_Refresh(&hiwdg);               // Clear watchdog
  }
  NVIC_SystemReset();                       // Reset system
}

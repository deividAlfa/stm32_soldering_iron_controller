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

// Silicon bug workaround for STM32F103 as ST document ES093 rev 7
/*
  __HAL_I2C_DISABLE(device);

  HW_SCL_GPIO_Port->ODR |= HW_SCL_Pin;
  HW_SDA_GPIO_Port->ODR |= HW_SDA_Pin;

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin =   SW_SCL_Pin;
  GPIO_InitStruct.Mode =  GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SW_SCL_GPIO_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin =   SW_SDA_Pin;
  HAL_GPIO_Init(SW_SCL_GPIO_Port, &GPIO_InitStruct);

  // SCL, SDA HIGH
  while( !(HW_SDA_GPIO_Port->IDR & HW_SDA_Pin) || !(HW_SCL_GPIO_Port->IDR & HW_SCL_Pin) );

  // SDA LOW
  HW_SDA_GPIO_Port->ODR &= ~(HW_SDA_Pin);
  while(HW_SDA_GPIO_Port->IDR & HW_SDA_Pin);

  // SCL LOW
  HW_SCL_GPIO_Port->ODR &= ~(HW_SCL_Pin);
  while(HW_SCL_GPIO_Port->IDR & HW_SCL_Pin);

  // SCL HIGH
  HW_SCL_GPIO_Port->ODR |= HW_SCL_Pin;
  while(!(HW_SCL_GPIO_Port->IDR & HW_SCL_Pin));

  // SDA HIGH
  HW_SDA_GPIO_Port->ODR |= HW_SDA_Pin;
  while(!(HW_SDA_GPIO_Port->IDR & HW_SDA_Pin));


  GPIO_InitStruct.Pin =   SW_SCL_Pin;
  GPIO_InitStruct.Mode =  GPIO_MODE_AF_OD;
  HAL_GPIO_Init(SW_SCL_GPIO_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin =   SW_SDA_Pin;
  HAL_GPIO_Init(SW_SCL_GPIO_Port, &GPIO_InitStruct);

  device->Instance->CR1 |= I2C_CR1_SWRST;
  device->Instance->CR1 &= ~(I2C_CR1_SWRST);
  __HAL_I2C_ENABLE(device);
  */



#if defined OLED_I2C && defined OLED_DEVICE
// Silicon bug workaround for STM32F103, force I2C RCC reset and re-init
void i2c_workaround(void){
  __HAL_RCC_I2C1_FORCE_RESET();
  __HAL_RCC_I2C2_FORCE_RESET();
  HAL_Delay(10);
  __HAL_RCC_I2C1_RELEASE_RESET();
  __HAL_RCC_I2C2_RELEASE_RESET();
  oled.device->State = HAL_I2C_STATE_RESET;
  __HAL_RCC_DMA1_CLK_ENABLE();
  oled.device->Instance = I2C1;
  oled.device->Init.ClockSpeed = 400000;
  oled.device->Init.DutyCycle = I2C_DUTYCYCLE_2;
  oled.device->Init.OwnAddress1 = 0;
  oled.device->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  oled.device->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  oled.device->Init.OwnAddress2 = 0;
  oled.device->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  oled.device->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(oled.device) != HAL_OK){
    Error_Handler();
  }
}
#endif

#if (defined OLED_SPI && !defined OLED_DEVICE)  || (defined OLED_I2C && (!defined OLED_DEVICE  || (defined OLED_DEVICE && defined I2C_TRY_HW)))
void enable_soft_Oled(void){
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
  i2c_Delay_H();
  for(uint8_t c=0;c<9;c++){
    Oled_Set_SCL();
    i2c_Delay_H();
    Oled_Clear_SCL();
    i2c_Delay_H();
  }
  i2cStop();
  #endif
}

void disable_soft_Oled(void){
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
#if (defined OLED_SPI)
void oled_send(uint8_t* bf, uint16_t count){
#elif (defined OLED_I2C)
void oled_send(uint8_t* bf, uint16_t count, uint8_t mode){
  i2cStart();
  i2cBegin(mode);
#endif
  uint8_t data;
  while(count--){                     // Unrolled byte loop increase a lot the performance
    data = *bf++;
    if((!data) || (data==0xFF)){
      Oled_Bit(data);
      Oled_Clock();
      Oled_Clock();
      Oled_Clock();
      Oled_Clock();
      Oled_Clock();
      Oled_Clock();
      Oled_Clock();
    }
    else{
      Oled_Bit(data & 0x80);
      Oled_Bit(data & 0x40);
      Oled_Bit(data & 0x20);
      Oled_Bit(data & 0x10);
      Oled_Bit(data & 0x08);
      Oled_Bit(data & 0x04);
      Oled_Bit(data & 0x02);
      Oled_Bit(data & 0x01);
    }
    #if (defined OLED_I2C)
    Oled_Set_SDA();
    Oled_Clock();                     // Ack Nack pulse (we don't read it)
    #endif
  }
  #if (defined OLED_I2C)
  i2cStop();
  #endif
}
#endif


#if defined OLED_I2C && (!defined OLED_DEVICE  || (defined OLED_DEVICE && defined I2C_TRY_HW))
// This delays are made for 36MHz (Ksger v2 software i2c). If increasing the cpu frequency, also increase the nop count 
__attribute__ ((noinline)) void i2c_Delay_H(void){                  // Intended no inline to add further delay and reduce nops
  asm("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
}

__attribute__ ((noinline)) void i2c_Delay_L(void){
  asm("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
}

void i2cStart(void){                                      // Start condition, SDA transition to low with SCL high
  Oled_Set_SCL();
  i2c_Delay_H();
  Oled_Clear_SDA();
  i2c_Delay_H();
  Oled_Clear_SCL();
  i2c_Delay_H();
}
void i2cStop(void){                                       // Stop condition, SCL transition to high with SDA low
  Oled_Clear_SDA();
  i2c_Delay_H();
  Oled_Set_SCL();
  i2c_Delay_H();
  Oled_Set_SDA();
  i2c_Delay_H();
}

// This sw i2c driver is extremely timing optimized, done specially for ksger v2.1 and compatibles running at 36MHz.
// Hacks clock low time using the slow rise time (i2c pullup resistors) as the delay.
// Will start failing if the core runs faster than 44-48MHz because of the tight timing.
void i2cBegin(uint8_t mode){
  uint8_t data, bf[2] = { OLED_ADDRESS, mode };
  for(uint8_t bytes=0;bytes<2;bytes++){
      data = bf[bytes];
      if((!data) || (data==0xFF)){                     // Unrolled byte loop increase a lot the performance
        Oled_Bit(data);
        Oled_Clock();
        Oled_Clock();
        Oled_Clock();
        Oled_Clock();
        Oled_Clock();
        Oled_Clock();
        Oled_Clock();
      }
      else{
        Oled_Bit(data & 0x80);
        Oled_Bit(data & 0x40);
        Oled_Bit(data & 0x20);
        Oled_Bit(data & 0x10);
        Oled_Bit(data & 0x08);
        Oled_Bit(data & 0x04);
        Oled_Bit(data & 0x02);
        Oled_Bit(data & 0x01);
      }
      Oled_Set_SDA();
      Oled_Clock();                     // Ack Nack pulse (we don't read it)
  }
}
#endif


// Send command in blocking mode
void write_cmd(uint8_t cmd) {

  while(oled.status==oled_busy);                          // Wait for DMA to finish

#if defined OLED_SPI
  #ifdef USE_CS
  Oled_Clear_CS();
  #endif

  #ifdef USE_DC
  Oled_Clear_DC();
  #endif

  #ifdef OLED_DEVICE
  HAL_StatusTypeDef err=HAL_SPI_Transmit(oled.device, &cmd, 1, 10);
  if(err!=HAL_OK){
    Error_Handler();
  }
  #else
  oled_send(&cmd,1);
  #endif

  #ifdef USE_DC
  Oled_Set_DC();
  #endif

  #ifdef USE_CS
  Oled_Set_CS();
  #endif


#elif defined OLED_I2C
  #if defined OLED_DEVICE && !defined I2C_TRY_HW
  if(HAL_I2C_Mem_Write(oled.device, OLED_ADDRESS, 0x00, 1, &cmd, 1, 100)!=HAL_OK){
    Error_Handler();
  }
  #elif defined OLED_DEVICE && defined I2C_TRY_HW
  if(oled.use_sw){
    oled_send(&cmd,1,i2cCmd);
  }
  else{
  	if(HAL_I2C_Mem_Write(oled.device, OLED_ADDRESS, 0x00, 1, &cmd, 1, 100)!=HAL_OK){
    	Error_Handler();
	  }
  }
  #else
  oled_send(&cmd,1,i2cCmd);
  #endif
#endif
}

void update_display( void ){
    if(oled.status!=oled_idle) { return; }                // If OLED busy, skip update
    if(oled.row!=0){ Error_Handler(); }

#if (defined OLED_I2C || defined OLED_SPI) && (!defined OLED_DEVICE || (defined OLED_DEVICE && defined I2C_TRY_HW))
    if(oled.use_sw){
      for(uint8_t row=0;row<8;row++){
        HAL_IWDG_Refresh(&hiwdg);
        setOledRow(row);

        #if defined OLED_SPI

        #ifdef USE_CS
        Oled_Clear_CS();
        #endif

        #ifdef USE_DC
        Oled_Set_DC();
        #endif

        oled_send((uint8_t *)&oled.buffer[128*row],128);

        #ifdef USE_CS
        Oled_Set_CS();
        #endif

        #elif defined OLED_I2C
        oled_send((uint8_t *)&oled.buffer[128*row],128,i2cData);
        #endif
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

#if !defined OLED_DEVICE || (defined OLED_DEVICE && defined I2C_TRY_HW)
void setOledRow(uint8_t row){
  write_cmd(0xB0|row);                                    // Set the OLED Row address
  write_cmd(systemSettings.settings.OledOffset);
  write_cmd(0x10);
}
#endif

void setOledPower(uint8_t power){
  powerStatus = power;
  if(power==enable){
    write_cmd(0xAF);          // Display On
  }
  else{
    write_cmd(0xAE);          // Display Off
  }
}
uint8_t getOledPower(void){
  return powerStatus;
}

void setContrast(uint8_t value) {
  write_cmd(0x81);                                        // Set Contrast Control
  write_cmd(value);                                       // Default => 0xFF
  lastContrast = value;
}

uint8_t getContrast(void) {
  return lastContrast;
}

#if defined OLED_SPI && !defined OLED_DEVICE
void ssd1306_init(DMA_HandleTypeDef *dma){
  enable_soft_Oled();

#elif defined OLED_SPI && defined OLED_DEVICE
void ssd1306_init(SPI_HandleTypeDef *device,DMA_HandleTypeDef *dma){
  oled.device  = device;
#elif defined OLED_I2C && defined OLED_DEVICE
void ssd1306_init(I2C_HandleTypeDef *device,DMA_HandleTypeDef *dma){
  oled.device  = device;
  i2c_workaround();
#elif defined OLED_I2C && !defined OLED_DEVICE && !defined I2C_TRY_HW
void ssd1306_init(DMA_HandleTypeDef *dma){
  enable_soft_Oled();
#else
  #error "Wrong display configuration in board.h!"
#endif

  oled.fillDMA= dma;

#if defined OLED_SPI
  #ifndef USE_DC
  #error Mandatory OLED DC Pin not configured
  #endif

  #ifdef USE_CS
  Oled_Set_CS();          // De-select
  #endif

  #ifdef USE_RST
  Oled_Clear_RES();       // Set RST
  HAL_Delay(10);          // Delay
  Oled_Set_RES();         // Release RST
  #endif

#endif

#if defined OLED_I2C
  #ifdef USE_CS
  Oled_Clear_CS();          // Unused in I2C mode, set low
  #endif

  #ifdef USE_DC
  Oled_Clear_DC();          // DC is the LSB address in I2C mode
  #endif

  #ifdef USE_RST
  Oled_Clear_RES();         // Set RST
  HAL_Delay(1);
  Oled_Set_RES();           // Release RST
  #endif

#endif
  systemSettings.settings.OledOffset = 2;         // Set by default while system settings are not loaded
  HAL_IWDG_Refresh(&hiwdg);                       // Clear watchdog
  HAL_Delay(200);                                 // 200mS wait for internal initialization
#if defined OLED_I2C && defined OLED_DEVICE && defined I2C_TRY_HW
  oled.use_sw=1;
  //disable_soft_Oled();
  //HAL_Delay(1);

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
    enable_soft_Oled();                            // Set sw mode
  }

#elif !defined OLED_DEVICE
    oled.use_sw=1;
#endif
  setOledPower(disable);
  write_cmd(0xD5);          // Set Display Clock Divide Ratio / Oscillator Frequency
  write_cmd(0xF0);          // Set max framerate
  write_cmd(0xA8);          // Set Multiplex Ratio
  write_cmd(0x3F);          // Default => 0x3F (1/64 Duty)
  write_cmd(0xD3);          // Set Display Offset
  write_cmd(0x00);          // Default => 0x00
  write_cmd(0x40|0x00);     // Set Display Start Line
  write_cmd(0x20);          // Set Memory Addressing Mode
  write_cmd(0x02);          // Default => 0x02
  write_cmd(0xA0|0x01);     // Set Segment Re-Map
  write_cmd(0xC0|0x08);     // Set COM Output Scan Direction
  write_cmd(0xDA);          // Set COM Pins Hardware Configuration
  write_cmd(0x02|0x10);     // Default => 0x12 (0x10)
  setContrast(0xFF);        // Init in max contrast
  write_cmd(0xD9);          // Set Pre-Charge Period
  write_cmd(0x22);          // Default => 0x22 (2 Display Clocks [Phase 2] / 2 Display Clocks [Phase 1])

  write_cmd(0xDB);          // Set VCOMH Deselect Level
  write_cmd(0x30);          // Default => 0x20 (0.77*VCC)

  write_cmd(0xA4|0x00);     // Set Entire Display On/Off
  write_cmd(0xA6|0x00);     // Set Inverse Display On/Off
  write_cmd(0x8D);          // Set Charge Pump command
  write_cmd(0x14);          // Enable charge pump
  write_cmd(0x33);          // Charge pump to 9V
  FillBuffer(BLACK,fill_dma);     // Clear buffer
  update_display();         // Update display CGRAM

  while(oled.status!=oled_idle);  // Wait for DMA completion (If enabled)

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
  uint32_t fillVal;
  while(oled.status!=oled_idle);              // Don't write to buffer while screen buffer is being transfered

  if(color==WHITE){ fillVal=0xffffffff; }     // Fill color = white
  else{ fillVal=0; }                          // Fill color = black

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
      while(1){                                                               // If error happens at this stage, just do nothing
        HAL_IWDG_Refresh(&hiwdg);
      }
    }
    #ifdef USE_DC
    Oled_Set_DC();
    #endif

    if(HAL_SPI_Transmit(oled.device, &oled.buffer[128*row], 128, 1000)!=HAL_OK){
      while(1){                                                               // If error happens at this stage, just do nothing
        HAL_IWDG_Refresh(&hiwdg);
      }
    }

    #elif defined OLED_I2C
    if(HAL_I2C_Mem_Write(oled.device, OLED_ADDRESS, 0x00, 1, cmd, 3, 50)){
      while(1){                                                               // If error happens at this stage, just do nothing
        HAL_IWDG_Refresh(&hiwdg);
      }
    }
    if(HAL_I2C_Mem_Write(oled.device, OLED_ADDRESS, 0x40, 1, &oled.buffer[128*row], 128, 1000)!=HAL_OK){
      while(1){                                                               // If error happens at this stage, just do nothing
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
      if(HAL_SPI_Transmit(oled.device, cmd, 3, 50)==HAL_OK){      // Send row command in blocking mode
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
      if(HAL_I2C_Mem_Write(oled.device, OLED_ADDRESS, 0x00, 1, cmd, 3, 50)==HAL_OK){
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
    if(HAL_I2C_Mem_Write_DMA(oled.device, OLED_ADDRESS, 0x40, 1, &oled.buffer[128*row], 128)!=HAL_OK){
      Error_Handler();
    }
#endif

    oled.row++;
  }
}

#endif

void FatalError(uint8_t type){
 uint8_t lang =systemSettings.settings.language;
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

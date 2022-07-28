#include "display.h"
#include "settings.h"
#include "buzzer.h"
#include "iron.h"
#include "gui.h"


#ifdef __BASE_FILE__
#undef __BASE_FILE__
#define __BASE_FILE__ "lcd.c"
#endif

oled_t oled;

static uint8_t lastContrast;
static uint8_t powerStatus;
static uint8_t inFatalError;
volatile uint32_t hardFault_args[9];
volatile uint32_t r4, r5, r6;
static void _lcd_write(uint8_t *data, uint16_t count, uint8_t mode);

/* This displays require both cmd and arguments to be sent with DC=0, so everythign is send at once */
#if defined ST7565
const uint8_t disp_init[] = { // Initialization for ST7565R
    1, c_disp_off,
    1, c_start_line,
    1, c_remap_off,
    1, c_com_rev,
    1, c_inv_off,
    1, c_bias_9,
    1, c_pwr_ctrl | c_pwr_boost | c_pwr_vreg | c_pwr_follow,
    2, c_boost_ratio, c_boost_234,
    1, c_res_ratio | 5,
    2, c_set_volume, 34,
    1, c_all_on,
};
#elif defined SSD1306
const uint8_t disp_init[] = { // Initialization for SH1106 / SSD1306 / SSD1309
    1, c_disp_off,
    2, c_clock_set, 0xF0,              // Set max framerate
    2, c_mux_ratio, 0x3F,
    2, c_offset, 0x00,
    1, c_start_line,
    2, c_addr_mode, 0x02,
    1, c_remap_on,
    1, c_com_rev,
    2, c_com_cfg, 0x12,
    2, c_contrast, 0xFF,               // Init in max contrast
    2, c_precharge, 0x44,              // Set Pre-Charge Period, 0x44 (4 Display Clocks [Phase 2] / 4 Display Clocks [Phase 1])
    2, c_vcomh_lvl, 0x3C,              // Set VCOMH Deselect Level, 0x3C (0.84*VCC)
    1, c_all_off,
    1, c_inv_off,
    2, c_pump_1306_set, c_pump_on,     // For SSD1306
    //2, c_pump_1106_set, c_pump_on,     // For SH1106 -- Disabled, caused issues with ssd1309, sh1106 seems to work without it */
    1, c_pump_1106_adj | 0x03          // For SH1106, pump to 9V
};
#else
#error "No display defined!"
#endif

// Silicon bug workaround for STM32F103 as ST document ES093 rev 7

#if defined DISPLAY_I2C && defined DISPLAY_DEVICE
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




#if !defined DISPLAY_DEVICE  || (defined DISPLAY_DEVICE && defined I2C_TRY_HW)
// This delays are made for 36MHz (Ksger v2 software i2c). If increasing the cpu frequency, also increase the nop count
__attribute__ ((noinline)) void bit_delay(void){                  // Intended no inline to add further delay and reduce nops
  asm("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
}

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

#ifdef DISPLAY_I2C
  Oled_Set_SCL();
  Oled_Set_SDA();
  GPIO_InitStruct.Pull =  GPIO_PULLUP;
  GPIO_InitStruct.Mode =  GPIO_MODE_OUTPUT_OD;
#else
  Oled_Clear_SCL();
  Oled_Clear_SDA();
  GPIO_InitStruct.Mode =  GPIO_MODE_OUTPUT_PP;
#endif
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  /*Configure GPIO pins : SCL_Pin */
  GPIO_InitStruct.Pin =   SW_SCL_Pin;
  HAL_GPIO_Init(SW_SCL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SDA_Pin */
  GPIO_InitStruct.Pin =   SW_SDA_Pin;
  HAL_GPIO_Init(SW_SDA_GPIO_Port, &GPIO_InitStruct);
#ifdef DISPLAY_I2C
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

void disable_soft_mode(void){
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

#if (defined DISPLAY_SPI && !defined DISPLAY_DEVICE) || (defined DISPLAY_I2C && (!defined DISPLAY_DEVICE  || (defined DISPLAY_DEVICE && defined I2C_TRY_HW)))

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
#if (defined DISPLAY_I2C)
  Oled_Bit(1);                                // Ack Nack pulse (we don't read it)
#endif
}

void bitbang_write(uint8_t* bf, uint16_t count, uint8_t mode){
#if defined DISPLAY_SPI
#if defined USE_CS
  Oled_Clear_CS();
#endif
  if(mode==modeData)
    Oled_Set_DC();
  else
    Oled_Clear_DC();
#elif (defined DISPLAY_I2C)
  i2cStart();
  i2cBegin(mode);
#endif
  while(count--)
    clock_byte(*bf++);
#if (defined DISPLAY_I2C)
  i2cStop();
#elif defined DISPLAY_SPI
#if defined USE_CS
  Oled_Set_CS();
#endif
#endif
}
#endif


#if (defined DISPLAY_I2C && !defined DISPLAY_DEVICE) || (defined DISPLAY_DEVICE && defined I2C_TRY_HW)

// This sw i2c driver is extremely timing optimized, done specially for ksger v2.1 and compatibles running at 36MHz.
// Hacks clock low time using the slow rise time (i2c pullup resistors) as the delay.
// Will start failing if the core runs faster than 44-48MHz because of the tight timing.
void i2cBegin(uint8_t mode){
  clock_byte(DISPLAY_ADDRESS);
  clock_byte(mode);
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

// Normal call, wait while oled dma is busy
void lcd_write(uint8_t *data, uint16_t count, uint8_t mode){
  while(oled.status==oled_busy){;}                          // Wait for DMA to finish
  _lcd_write(data, count, mode);
}

// Call from ISR only, bypassing busy checks
void _lcd_write(uint8_t *data, uint16_t count, uint8_t mode){
#if defined I2C_TRY_HW || !defined DISPLAY_DEVICE
  if(oled.use_sw)
    bitbang_write(data, count, mode);
#if defined I2C_TRY_HW
  else{
#endif
#endif

#if defined DISPLAY_SPI && defined DISPLAY_DEVICE
#ifdef USE_CS
      Oled_Clear_CS();
#endif
      if (mode==modeData)
        Oled_Set_DC();
      else
        Oled_Clear_DC();
#endif

#ifdef DISPLAY_DEVICE
    for(uint8_t t=0;;t++){                                              // Weird bug workaround, sometimes HAL returns BUSY here.
#ifdef DISPLAY_SPI
      if(HAL_SPI_Transmit(oled.device, data, count, HAL_MAX_DELAY)==HAL_OK)
#elif defined DISPLAY_I2C
      if(HAL_I2C_Mem_Write(oled.device, DISPLAY_ADDRESS, mode, 1, data, count, HAL_MAX_DELAY)==HAL_OK)
#endif
        break;                                                          // Success, break loop
      else
        display_dma_abort();                                            // Failure, reset display handler
      if(t>4){                                                          // Max 5 tries
        if(!inFatalError)
          Error_Handler();
        else
          buttonReset();          // We're already on ErrorHandler, can't do anymore!
      }
    }
#if defined I2C_TRY_HW || !defined DISPLAY_DEVICE
  }
#endif
#endif
#if defined DISPLAY_SPI && defined DISPLAY_DEVICE && defined USE_CS
  Oled_Clear_CS();
#endif
}


void update_display( void ){
    if(oled.status!=oled_idle) { return; }                // If OLED busy, skip update
    if(oled.row!=0){ Error_Handler(); }                   // If called while already updating, something went wrong!

#if !defined DISPLAY_DEVICE || (defined DISPLAY_DEVICE && defined I2C_TRY_HW)
    if(oled.use_sw){
      for(uint8_t row=0;row<8;row++){
        HAL_IWDG_Refresh(&hiwdg);
        setDisplayRow(row);
        lcd_write(&oled.buffer[128*row], 128, modeData);
      }
      return;
    }
#endif
    oled.status=oled_busy;
#if defined DISPLAY_SPI && defined DISPLAY_DEVICE
    HAL_SPI_TxCpltCallback(oled.device);                  // Call the DMA callback function to start sending the frame

#elif defined DISPLAY_I2C && defined DISPLAY_DEVICE
    HAL_I2C_MemTxCpltCallback(oled.device);               // Call the DMA callback function to start sending the frame
#endif
}

void setDisplayRow(uint8_t row){
  uint8_t cmd[] = { c_page | row, c_col_H, c_col_L | systemSettings.settings.displayOffset };
  for(uint8_t i=0; i<sizeof(cmd); i++)
      lcd_write(&cmd[i], 1, modeCmd);     // Normal
}

void setDisplayPower(uint8_t power){
#ifdef ST7565
  uint8_t cmd[] = { ((power==enable) ? c_all_off : c_disp_off ),
                    ((power==enable) ? c_disp_on : c_all_on ) };
#else
  uint8_t cmd[] = { ((power==enable) ? c_disp_on : c_disp_off ) };
#endif
  for(uint8_t i=0; i<sizeof(cmd); i++)
      lcd_write(&cmd[i], 1, modeCmd);
  powerStatus = power;
}

uint8_t getDisplayPower(void){
  return powerStatus;
}

void setDisplayXflip(uint8_t f) {
  uint8_t cmd [] = { ((f==enable) ? c_com_rev : c_com_norm) };
  lcd_write(cmd, sizeof(cmd), modeCmd);
}

void setDisplayYflip(uint8_t f) {
  uint8_t cmd [] = { ((f==enable) ? c_remap_on : c_remap_off) };
  lcd_write(cmd, sizeof(cmd), modeCmd);
}

#ifdef ST7565
void setDisplayResRatio(uint8_t r){
  uint8_t cmd [] = { c_res_ratio | r };
  lcd_write(cmd, sizeof(cmd), modeCmd);
}
#endif

void setDisplayContrastOrBrightness(uint8_t value) {
  uint8_t cmd [] = { 0x81, value };
  lcd_write(cmd, sizeof(cmd), modeCmd);
  lastContrast = value;
}

uint8_t getDisplayContrastOrBrightness(void) {
  return lastContrast;
}

#if !defined DISPLAY_DEVICE
void lcd_init(DMA_HandleTypeDef *dma){
  enable_soft_mode();
  oled.use_sw=1;
#elif defined DISPLAY_SPI && defined DISPLAY_DEVICE
void lcd_init(SPI_HandleTypeDef *device,DMA_HandleTypeDef *dma){
  oled.device  = device;
#elif defined DISPLAY_I2C && defined DISPLAY_DEVICE
void lcd_init(I2C_HandleTypeDef *device,DMA_HandleTypeDef *dma){
  oled.device  = device;
  i2c_workaround();
#endif
  oled.fillDMA= dma;
#if defined DISPLAY_SPI
#ifdef USE_CS
  Oled_Set_CS();          // De-select
#endif
#elif defined DISPLAY_I2C
  #ifdef USE_CS
  Oled_Clear_CS();          // Unused in I2C mode, set low
  #endif

  #ifdef USE_DC
  Oled_Clear_DC();          // DC is the LSB address in I2C mode
  #endif
#endif

#ifdef USE_RST
  Oled_Clear_RES();
  HAL_Delay(1);
  Oled_Set_RES();
#endif
  HAL_IWDG_Refresh(&hiwdg);                       // Clear watchdog
  HAL_Delay(200);                                 // 200mS wait for internal initialization
  systemSettings.settings.displayOffset = 2;         // Set by default while system settings are not loaded
  HAL_IWDG_Refresh(&hiwdg);                       // Clear watchdog

#if defined DISPLAY_I2C && defined DISPLAY_DEVICE && defined I2C_TRY_HW
  oled.use_sw=1;
  // Check if OLED is connected to hardware I2C
  for(uint8_t try=0; try<5; try++){
    uint8_t data;
    uint8_t res = HAL_I2C_Mem_Read(oled.device, DISPLAY_ADDRESS, 0x00, 1, &data, 1, 100);
    if(res==HAL_OK){
      oled.use_sw=0;                              // Detected, enable hw
      break;
    }
    HAL_IWDG_Refresh(&hiwdg);                     // Clear watchdog
  }
  if(oled.use_sw){                                // Display not detected
    enable_soft_mode();                            // Set sw mode
  }
#endif
  /* Send initalization stream */
  for(uint16_t i=0; i<sizeof(disp_init);){
    lcd_write((uint8_t*)&disp_init[i+1], disp_init[i], modeCmd);
    i+= disp_init[i]+1;
  }
  fillBuffer(BLACK,fill_dma); // Clear buffer
  update_display();           // Update display RAM
  setDisplayPower(enable);
}

/*
*  Clear buffer with 32bit-transfer for fast filling (ensure that Oled buffer is 32-bit aligned!)
*   128 * 8 = 1KB, / 4byte DMA txfer = 256 clock cycles (in theory)
*   Args:
*       color:   0 = black, 1 = white
*       mode:   0 = Use software(fail-safe), 1= Use DMA (normal operation)
*/

void fillBuffer(bool color, bool mode){
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

#if (defined DISPLAY_I2C || defined DISPLAY_SPI) && defined DISPLAY_DEVICE

// Abort DMA transfers and reset status
void display_dma_abort(void){
  if(oled.device!=NULL){
#if defined DISPLAY_SPI
    HAL_SPI_Abort(oled.device);
#elif defined DISPLAY_I2C
    HAL_I2C_Master_Abort_IT(oled.device, 0);
#endif
    __HAL_UNLOCK(oled.device);
    HAL_DMA_PollForTransfer(oled.device->hdmatx, HAL_DMA_FULL_TRANSFER, 100);  // Wait for DMA to finish
  }
  oled.status=oled_idle;  // Force oled idle status
}


// Screen update for hard error handlers (crashes) not using DMA
void update_display_ErrorHandler(void){
  for(uint8_t row=0;row<8;row++){
    setDisplayRow(row);
    lcd_write(&oled.buffer[128*row], 128, modeData);
  }
}


#ifdef DISPLAY_SPI
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *device){
#elif defined DISPLAY_I2C
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *device){

#endif

  if(device == oled.device){

    if(oled.status!=oled_busy)        // We should get there only with busy flag.
      Error_Handler();                // Additional check to ensure DMA/ISR doesn't make anything weird

    if(HAL_DMA_GetState(oled.device->hdmatx)!=HAL_DMA_STATE_READY){           // If DMA busy
      HAL_DMA_PollForTransfer(oled.device->hdmatx, HAL_DMA_FULL_TRANSFER, 10);
    }
    if(oled.row>7){

      #if defined DISPLAY_SPI && defined USE_CS
      Oled_Set_CS();                                    // Release CS
      #endif

      oled.row=0;                                       // Reset row position
      oled.status=oled_idle;
      return;                                           // Return without re-triggering DMA.
    }
    {
      uint8_t cmd[] = { c_page | oled.row, c_col_H, c_col_L | systemSettings.settings.displayOffset };
      for(uint8_t i=0; i<sizeof(cmd); i++)
        _lcd_write(&cmd[i], 1, modeCmd);
    }

    oled.status=oled_busy;        // Can be resetted if _lcd_write calls display_dma_abort, so always set this value

#ifdef DISPLAY_SPI
#ifdef USE_CS
    Oled_Clear_CS();
#endif
    Oled_Set_DC();
    if(HAL_SPI_Transmit_DMA(oled.device, &oled.buffer[128*oled.row], 128)!= HAL_OK)      // Send row data in DMA interrupt mode. Never failed here, so no need to re-try.
#elif defined DISPLAY_I2C
    if(HAL_I2C_Mem_Write_DMA(oled.device, DISPLAY_ADDRESS, modeData, 1, &oled.buffer[128*oled.row], 128)!=HAL_OK)
#endif
      Error_Handler();

    oled.row++;
  }
}

#endif

void fatalError(uint8_t type){
  inFatalError = 1;
  uint8_t lang = systemSettings.settings.language;
  if(lang>(LANGUAGE_COUNT-1)){
    lang=lang_english;
  }
  Oled_error_init();
  switch(type){
    case error_FLASH:
      putStrAligned("FLASH ERROR", 0, align_center);
      break;
    case error_NMI:
      putStrAligned("NMI HANDLER", 0, align_center);
      break;
    case error_HARDFAULT:
    {
      char str[32];
      putStrAligned("HARD FAULT", 0, align_center);
      u8g2_SetFont(&u8g2,u8g2_font_5x8_tr);
      sprintf(str,"r0:%08lX   r1:%08lX", hardFault_args[0], hardFault_args[1]);
      u8g2_DrawUTF8(&u8g2, 0, 12, str);
      sprintf(str,"r2:%08lX   r3:%08lX", hardFault_args[2], hardFault_args[3]);
      u8g2_DrawUTF8(&u8g2, 0, 20, str);
      sprintf(str,"r4:%08lX   r5:%08lX", r4, r5);
      u8g2_DrawUTF8(&u8g2, 0, 28, str);
      sprintf(str,"r6:%08lX   sp:%08lX", r6, hardFault_args[8]);
      u8g2_DrawUTF8(&u8g2, 0, 36, str);
      sprintf(str,"lr:%08lX  r12:%08lX", hardFault_args[5], hardFault_args[4]);
      u8g2_DrawUTF8(&u8g2, 0, 44, str);
      sprintf(str,"pc:%08lX  psr:%08lX", hardFault_args[6], hardFault_args[7]);
      u8g2_DrawUTF8(&u8g2, 0, 52, str);
      /*
      sprintf(str,"PC: %08lX", SCB->BFAR);
      */
      u8g2_SetFont(&u8g2, default_font);
      break;
    }
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
      char str[8];
      sprintf(str,">%u\260C",level);
      putStrAligned(strings[lang].ERROR_RUNAWAY, 0, align_center);
      putStrAligned(str, 15, align_center);
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
  if(type!=error_HARDFAULT)
    putStrAligned(strings[lang].ERROR_BTN_RESET, 50, align_center);

#if (defined DISPLAY_I2C || defined DISPLAY_SPI) && defined DISPLAY_DEVICE
  if(!oled.use_sw)
    update_display_ErrorHandler();

#if defined DISPLAY_I2C && defined I2C_TRY_HW
  else{
    update_display();
  }
#endif

#else
  update_display();
#endif

  buttonReset();
}

void putStrAligned(char* str, uint8_t y, AlignType align){

  if(align==align_left){
    u8g2_DrawUTF8(&u8g2, 0, y, str);
  }
  else{
    uint8_t len = u8g2_GetUTF8Width(&u8g2, str);
    if(align==align_center){
      u8g2_DrawUTF8(&u8g2, ((displayWidth-1)-len)/2, y, str);
    }
    else if(align==align_right){
      u8g2_DrawUTF8(&u8g2, (displayWidth-1)-len, y, str);
    }
  }
}
void buttonReset(void){
  __disable_irq();
  while(!BUTTON_input()){                   // Wait while the button is released
    for(uint16_t i=0;i<50000;i++);          // Small delay
    HAL_IWDG_Refresh(&hiwdg);               // Clear watchdog
  }
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

void Oled_error_init(void){
  setSafeMode(enable);
#if defined DISPLAY_DEVICE
  if(!oled.use_sw)
    display_dma_abort();
#endif  
  buzzer_fatal_beep();
  setDisplayContrastOrBrightness(defaultSettings.contrastOrBrightness);
  fillBuffer(BLACK,fill_soft);
  u8g2_SetFont(&u8g2,default_font );
  u8g2_SetDrawColor(&u8g2, WHITE);
  u8g2_SetMaxClipWindow(&u8g2);
  systemSettings.settings.displayOffset = DISPLAY_OFFSET;
}

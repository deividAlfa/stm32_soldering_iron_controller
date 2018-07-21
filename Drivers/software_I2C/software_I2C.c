#include <stdint.h>
#include "main.h"
#include "stm32f1xx_hal.h"
#include "dwt_stm32_delay.h"

#define HIGH GPIO_PIN_SET
#define LOW  GPIO_PIN_RESET

static void i2c_delay() {
	DWT_Delay_us(1);
}

static void digitalWrite(GPIO_TypeDef *GPIO_Port, uint16_t GPIO_Pin, GPIO_PinState PinState){
	HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, PinState);
}

static GPIO_PinState digitalRead(GPIO_TypeDef *GPIO_Port, uint16_t GPIO_Pin){
	return HAL_GPIO_ReadPin(GPIO_Port, GPIO_Pin);
}

static void i2c_start(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin) {
  
  i2c_delay();
  digitalWrite(sclport, sclpin, HIGH);
  i2c_delay();
  digitalWrite(sdaport, sdapin, HIGH);
  i2c_delay();
  digitalWrite(sdaport, sdapin, LOW);
  i2c_delay();
  digitalWrite(sclport, sclpin, LOW);
  
}

static void i2c_stop(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin) {
  
  i2c_delay();
  digitalWrite(sclport, sclpin, HIGH);
  i2c_delay();
  digitalWrite(sdaport, sdapin, HIGH);
  
}

static void i2c_send_ack(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin) {
  
  i2c_delay();
  digitalWrite(sdaport, sdapin, LOW);
  i2c_delay();
  digitalWrite(sclport, sclpin, HIGH);
  i2c_delay();
  digitalWrite(sclport, sclpin, LOW);
  
}

static uint8_t i2c_get_ack(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin) {
  
  i2c_delay();
  digitalWrite(sclport, sclpin, LOW);
  i2c_delay();
  digitalWrite(sdaport, sdapin, HIGH);
  i2c_delay();
  digitalWrite(sclport, sclpin, HIGH);
  for (int i=100;i>0;i--) {
    if (!digitalRead(sdaport, sdapin)) {
      i2c_delay();
      i2c_delay();
      digitalWrite(sclport, sclpin, LOW);
      return 1;
    }
  }
  return 0;
  
}

static void i2c_send_nack(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin) {
  
  i2c_delay();
  digitalWrite(sdaport, sdapin, HIGH);
  i2c_delay();
  digitalWrite(sclport, sclpin, HIGH);
  
}

static void i2c_shift_out(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin, uint8_t val) {
  

  for (int i=0;i<8;i++) {
    i2c_delay();
		if (val & (1 << (7 - i)))
       digitalWrite(sdaport, sdapin, HIGH);
		else
       digitalWrite(sdaport, sdapin, LOW);
		
    i2c_delay();
    digitalWrite(sclport, sclpin, HIGH);
		
    i2c_delay();
    digitalWrite(sclport, sclpin, LOW);
  }

}

static uint8_t i2c_shift_in(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin) {
  
  uint8_t data=0;
  for (int i=0;i<8;i++) {
    i2c_delay();
    digitalWrite(sclport, sclpin, HIGH);
    i2c_delay();
    data += digitalRead(sdaport, sdapin) << (7-i);
    i2c_delay();
    digitalWrite(sclport, sclpin, LOW);
  }
  return data;
  
}

/*
static uint8_t i2c_read8(uint8_t addr) {
  
  / *
  1) start
  2) slave address write
  3) ack
  4) register address
  5) ack
  6) start
  7) slave address read
  8) ack
  9) data in
  10) nack
  11) stop
  * /

  uint8_t data = 0;
  i2c_start();
  i2c_shift_out(WRITE_ADDRESS);
  if (!i2c_get_ack()) {
//
// I2C ack missing
//
  }
  i2c_shift_out(addr);
  if (!i2c_get_ack()) {
//
// I2C ack missing
//
  }
// Repeated start
  i2c_start();  
  i2c_shift_out(READ_ADDRESS);  
  if (!i2c_get_ack()) {
//
// I2C ack missing
//
  }
  data = i2c_shift_in();
  i2c_send_nack();
  i2c_stop();
  return data;
  
}
*/

void i2c_write_raw(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin, uint8_t *val, uint8_t size) {

  int i=0;
  i2c_start(sclport, sclpin, sdaport, sdapin);
  for(i=0;i<size;i++){
    i2c_shift_out(sclport, sclpin, sdaport, sdapin, val[i]);
    if (!i2c_get_ack(sclport, sclpin, sdaport, sdapin)) {
      //
      // I2C ack missing
      //
    }
  }
  i2c_stop(sclport, sclpin, sdaport, sdapin);
  
}

void i2c_write(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin, uint8_t i2c_addr, uint8_t *val, uint8_t size) {

  int i=0;
  i2c_start(sclport, sclpin, sdaport, sdapin);
  i2c_shift_out(sclport, sclpin, sdaport, sdapin, i2c_addr);
  if (!i2c_get_ack(sclport, sclpin, sdaport, sdapin)) {
    //
    // I2C ack missing
    //
  }
  for(i=0;i<size;i++){
    i2c_shift_out(sclport, sclpin, sdaport, sdapin, val[i]);
    if (!i2c_get_ack(sclport, sclpin, sdaport, sdapin)) {
      //
      // I2C ack missing
      //
    }
  }
  i2c_stop(sclport, sclpin, sdaport, sdapin);
  
}

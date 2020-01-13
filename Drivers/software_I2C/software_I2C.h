#ifndef __software_I2C_H
#define __software_I2C_H
#ifdef __cplusplus
 extern "C" {
#endif

void i2c_delay();
void digitalWrite(GPIO_TypeDef *GPIO_Port, uint16_t GPIO_Pin, GPIO_PinState PinState);
GPIO_PinState digitalRead(GPIO_TypeDef *GPIO_Port, uint16_t GPIO_Pin);
void i2c_start(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin);
void i2c_stop(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin);
void i2c_send_ack(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin);
uint8_t i2c_get_ack(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin);
void i2c_send_nack(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin);
void i2c_shift_out(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin, uint8_t val);
uint8_t i2c_shift_in(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin);
//static uint8_t i2c_read8(uint8_t addr);
void i2c_write_raw(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin, uint8_t *val, uint8_t size);
void i2c_write(GPIO_TypeDef *sclport, uint16_t sclpin, GPIO_TypeDef *sdaport, uint16_t sdapin, uint8_t i2c_addr, uint8_t *val, uint8_t size);

#ifdef __cplusplus
}
#endif
#endif


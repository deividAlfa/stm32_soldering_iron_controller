/*
 * filtrai.h
 *
 *  Created on: 2012.09.18
 *      Author: Admin
 */
#ifndef FILTRAI_H_
#define FILTRAI_H_
#include "main.h"
#include "stdint.h"

typedef struct{
  int (*buf_p)[1];
  const unsigned int buf_size;
  unsigned int counter;
  uint8_t inited;
}INTEGRATOR_INT;

typedef struct{
  float (*buf_p)[1];
  const unsigned int buf_size;
  unsigned int counter;
  uint8_t inited;
}INTEGRATOR_FT;

typedef struct{
  uint16_t (*const buf_p)[1];
  const unsigned int buf_size;
  unsigned int counter;
  uint8_t inited;
}INTEGRATOR_U16;

#define INIT_INTEGRATOR(size,type)						     \
{            												 \
	.buf_p		= 	 ( type(*)[1])&( type [size]){0}   		,\
	.buf_size	= size										,\
	.counter 	= 0											 \
}


typedef struct{
  int delta, daliklis, false_cnt,idx;
  int arr5[5];
  float avg;
  uint8_t busena;
}ISAVGOF5;


int32_t integrator_int(int32_t sample, INTEGRATOR_INT *i);
float integrator_ft(float sample, INTEGRATOR_FT *i);
uint16_t integrator_u16(uint16_t sample, INTEGRATOR_U16 *i);
uint8_t isavgof5(int32_t data, ISAVGOF5 *i);

#endif /* FILTRAI_H_ */

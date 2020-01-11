/*
 * Filtrai.c
 *
 *  Created on: 2012.09.18
 *      Author: Admin
 */
#include "filtrai.h"

#define APPEND(A,B) (A##B)

int APPEND(int,_buf)[50];





int32_t integrator(int32_t sample, INTEGRATOR_INT *i)
{
 float result;
 int64_t accu=0;
 int j;
 (*i->buf_p)[i->counter]=sample;
 i->counter=( (i->counter)+1)%i->buf_size;
 accu=0;
  for(j=0; j<i->buf_size;j++){
    accu+=(*i->buf_p)[j];
  }
  uint32_t div = (i->inited)?i->buf_size:i->counter;

  result=(float)accu/div+0.5;
  return result;
}

float integrator_ft(float sample, INTEGRATOR_FT *i)
{
 float result;
 double accu=0;
 int j;
 (*i->buf_p)[i->counter]=sample;
 i->counter=( (i->counter)+1)%i->buf_size;
 if(i->counter == 0){
	 i->inited = 1;
 }
 accu=0;
  for(j=0; j<i->buf_size;j++){
    accu+=(*i->buf_p)[j];
  }
  float div = (i->inited)?(i->buf_size):(i->counter);
  result= (accu)/div;
  return result;
}

uint16_t integrator_u16(uint16_t sample, INTEGRATOR_U16 *i)
{
 uint16_t result;
 uint64_t accu=0;
 int j;
 (*i->buf_p)[i->counter]=sample;
 i->counter=( (i->counter)+1)%i->buf_size;
 if(i->counter == 0){
	 i->inited = 1;
 }
 accu=0;
  for(j=0; j<i->buf_size;j++){
    accu+=(*i->buf_p)[j];
  }
  uint32_t div = (i->inited)?(i->buf_size):(i->counter);
  result= (uint16_t)(((float)(accu+0.5))/div);
  return result;
}



uint8_t isavgof5(int32_t data, ISAVGOF5 *i){
  int j;
  i->busena=0;
  if(data<(i->avg+i->delta) && data>(i->avg-i->delta) ) { //jei patenka i diapazona
    if(i->daliklis>1){  //jei rastos bent 2 reiksmes lange
      i->busena=1;    
    }    
    if(i->daliklis<5){
      i->daliklis++;
    }

    i->arr5[i->idx]=data;

    //perskaiciuojam avg
    i->avg=0;
    for(j=0;j<5;j++){
        i->avg+=i->arr5[j];
    }
    i->avg=i->avg/i->daliklis;

    i->idx=(i->idx+1)%5;
    i->false_cnt=0;
  }else{
    i->false_cnt++;

    if(i->false_cnt>=3){    //reset
      i->daliklis=1;
      i->false_cnt=0;
      memset(i->arr5,0,sizeof(i->arr5));
      i->arr5[0]=data;
      i->avg=data;
      i->idx=1;
    }
  }

  return  i->busena;
}

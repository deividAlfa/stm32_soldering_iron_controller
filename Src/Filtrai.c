/*
 * Filtrai.c
 *
 *  Created on: 2012.09.18
 *      Author: Admin
 */
#include "filtrai.h"
#include "string.h"

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
 double accu=0;

 (*i->buf_p)[i->counter]=sample;
 i->counter=( (i->counter)+1)%i->buf_size;
 if(i->counter == 0 && i->inited == 0){
	 i->inited = 1;
 }

  if( (i->abs_counter % (i->update_size)) ==0 ){
	  accu=0;
	  int j;
	  int buf_size = (i->inited)?(i->buf_size):(i->counter);
	  for(j=0; j<buf_size;j++){
		accu+=(*i->buf_p)[j];
	  }
	  i->result= (accu)/buf_size;
  }
  i->abs_counter++;
  return i->result;
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

uint16_t arr_u16_avg(uint16_t* arr, uint16_t len){

	uint32_t acc = 0;
	uint16_t max = 0;
	uint16_t min = 0xFFFF;
	uint16_t temp;
	uint16_t result;
	for(int x = 0; x < len; x++) {
		temp = *arr++;
		acc += temp;
		if(temp > max)
			max = temp;
		if(temp < min)
			min = temp;
	}

	result = UINT_DIV( (acc - min - max) , len-2);
	return result;
}

uint16_t arr_set_zeros_above_threshold(uint16_t *src, uint16_t len, uint16_t thr, uint16_t rising_edge_dur, uint16_t falling_edge_dur){

	uint16_t tmp;
	uint16_t prev_tmp;
	uint16_t edge;
	uint16_t* dst;
	uint16_t zeroed = 0;

	if(src == NULL || len < 2 ){
		return 0;
	}

	edge = 0;
	prev_tmp = *src;
	/* find falling edge */
	for(int i = 1; i < len; i++) {
		dst = src + i; /* array forward search */
		tmp = *dst;
                
                if(edge && tmp>=thr){
                    edge = 0;                    
                }
                
                if(!edge){
                    edge = (tmp < thr && prev_tmp >= thr && edge == 0)?falling_edge_dur:0;
                }
		if(edge){
			edge--;
			zeroed++;
			*dst = 0;
		}
		prev_tmp = tmp;
	}

	edge = 0;
	prev_tmp = *(src+(len-1));
	/* find rising edge */
	for(int i = 1; i < len; i++) {
		dst = src + (len-1) - i; /* array backwards search */
		tmp = *dst;

                if(edge && tmp>=thr){
                    edge = 0;                    
                }
                
                if(!edge){
                    edge = (tmp < thr && prev_tmp >= thr && edge == 0)?rising_edge_dur:0;
                }
                
		if(edge){
			edge--;
			zeroed++;
			*dst = 0;
		}
		prev_tmp = tmp;
	}

	/* above thr search */
	for(int i = 0; i < len; i++) {
		dst = src + i;
		tmp = *dst;

		if(tmp >= thr){
			zeroed++;
			*dst = 0;
		}
	}

	return zeroed;
}

uint16_t arr_rem_selected_val(uint16_t selected_val, uint16_t *src, uint16_t len ){
	uint16_t new_size = 0;
	uint16_t tmp;
	uint16_t* dst = src;

	for(int i = 0; i < len; i++) {
		tmp = * (src + i);
		if( tmp != selected_val){
			*dst++ = tmp;
            new_size++;
		}
	}
        
    //memset( (uint8_t*)dst,0,(len-new_size)*2);
	return new_size;
}

uint16_t arr_u16_avg_ignore_val(uint16_t ignore_val, uint16_t* arr, uint16_t len, uint16_t* new_len){

	uint32_t acc = 0;
	uint16_t max = 0;
	uint16_t min = 0xFFFF;
	uint16_t tmp;
	uint16_t result;
    uint16_t len2=0;
	for(int x = 0; x < len; x++) {
		tmp = *arr++;
                if( tmp == ignore_val){
                    continue;
                }
                len2++;
		acc += tmp;
		if(tmp > max)
			max = tmp;
		if(tmp < min)
			min = tmp;
	}
        if(len2>2){
            result = UINT_DIV( (acc - min - max) , len2-2);
        }else{
            result = 0;
        }
    *new_len = len2;
	return result;
}
//uint16_t arr_calc_avgU16_when_ref_value_is(uint16_t* arr, uint16_t len, int8_t* ref, uint8_t ref_valid_val){
//	uint32_t acc = 0;
//	uint16_t max = 0;
//	uint16_t min = 0xFFFF;
//	uint16_t temp;
//	uint16_t result;
//	uint16_t acc_len =0;
//	for(int x = 0; x < len; x++) {
//		temp = *arr++;
//			if(*ref++ == ref_valid_val){
//				acc += temp;
//				acc_len++;
//				if(temp > max)
//					max = temp;
//				if(temp < min)
//					min = temp;
//			}
//	}
//	if(acc_len>10){
//		result = UINT_DIV( (acc - min - max) , acc_len-2);
//	}else{
//		result = UINT_DIV( acc , acc_len);
//	}
//	return result;
//}
//
//void replace_ignored_val_by_neighbours(int8_t ignored_val, int8_t* arr, uint16_t len, int8_t* out){
//	int8_t neighbour = ignored_val;
//	int8_t tmp;
//
//	for(int x = 0; x < len; x++) {
//		tmp = *arr++;
//		if(tmp!=ignored_val && tmp != neighbour){
//			/* new neighbour */
//			neighbour = tmp;
//		}
//		*out++ = neighbour;
//	}
//}
//
//void arr_find_vals_u8(uint8_t* arr, uint16_t len, uint16_t val, uint8_t* out, uint16_t* out_len){
//
//	uint16_t tmp;
//	uint16_t out_idx = 0;
//	for(int x = 0; x < len; x++) {
//		tmp = *arr++;
//		if(tmp == val){
//			*out++ = tmp;
//			out_idx++;
//		}
//	}
//	*out_len = out_idx;
//}

















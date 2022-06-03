/*
 * user_main.h
 *
 *  Created on: Apr 22, 2022
 *      Author: KocsisV
 */

#ifndef INC_USER_MAIN_H_
#define INC_USER_MAIN_H_

#include "board.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

extern IWDG_HandleTypeDef hiwdg;
extern CRC_HandleTypeDef hcrc;

//#define DISABLE_OUTPUT                // Enable to fully disable the tip power (Ex. for debugging)
#define DEBUG_ERROR                   // Enable to show file/line error messages

#ifdef DEBUG                          // By default, enable heap debugging in debug builds
#define DEBUG_ALLOC                   // Watch "mi" and "max_allocated" variables!
#endif

/*
 * Macro to enable debugging of the allocated memory
 * max_allocated will hold the max used memory at any time
 */
#ifdef DEBUG_ALLOC
extern uint32_t        max_allocated;
extern struct mallinfo mi;
#define dbg_mem() mi=mallinfo();                  \
                  if(mi.uordblks>max_allocated){  \
                    max_allocated=mi.uordblks;    \
                  }

#define _malloc(x)    malloc(x); dbg_mem()
#define _calloc(x,y)  calloc(x,y); dbg_mem()
#define _free(x)      free(x); dbg_mem()

#else
#define _malloc     malloc
#define _calloc     calloc
#define _free       free
#endif

#define WAKE_input()   HAL_GPIO_ReadPin(WAKE_GPIO_Port, WAKE_Pin)
#define BUTTON_input() HAL_GPIO_ReadPin(ENC_SW_GPIO_Port, ENC_SW_Pin)

void Program_Handler(void);
void initBeforeMCUConfiguration(void);
void InitAfterMCUConfiguration(void);
void mainCycle(void);
void CrashErrorHandler(char * file, int line);

#endif /* INC_USER_MAIN_H_ */

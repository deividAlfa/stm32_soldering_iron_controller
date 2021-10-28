/*
 * rotary_encoder.h
 *
 *  Created on: Jan 12, 2021
 *      Author: David    Original work by Jose Barros (PTDreamer), 2017
 */

#ifndef GENERALIO_ROTARY_ENCODER_H_
#define GENERALIO_ROTARY_ENCODER_H_

#define BIG_TRESHOLD 200

#include "main.h"

typedef enum {
  Rotate_Increment,                   /*!< Encoder was incremented */
  Rotate_Decrement,                   /*!< Encoder was decremented */
  Rotate_Increment_while_click,       /*!< Encoder was incremented */
  Rotate_Decrement_while_click,       /*!< Encoder was decremented */
  Release,                            /*!< Encoder was decremented */
  Click,
  LongClick,
  Rotate_Nothing,                     /*!< Encoder stop at it was before */
} RE_Rotation_t;

/**
 * @brief  Rotary encoder mode selection for rotation
 */
typedef enum {
  RE_Mode_Reverse,                         // Rotary encoder reverse mode
  RE_Mode_Forward                          // Rotary encoder forward mode.
} RE_Mode_t;

typedef enum {
  RE_BT_PRESSED,
  RE_BT_CLICKED,
  RE_BT_LONG_CLICK,
  RE_BT_DRAG,
  RE_BT_UNRELEASED,
  RE_BT_HIDLE
} RE_Click_t;
/**
 * @brief  Rotary main working structure
 */
volatile typedef struct {
  int32_t       Absolute;             /*!< Absolute rotation from beginning, for public use */
  int32_t       Diff;                 /*!< Rotary difference from last check, for public use */
  RE_Rotation_t Rotation;             /*!< Increment, Decrement or nothing, for public use */
  RE_Mode_t     Mode;                 /*!< Rotary encoder mode selected */
  bool          LastA;                /*!< Last status of A pin when checking. Meant for private use */
  int32_t       RE_Count;             /*!< Temporary variable to store data between rotation and user check */
  GPIO_TypeDef* GPIO_A;               /*!< Pointer to GPIOx for Rotary encode A pin. Meant for private use */
  GPIO_TypeDef* GPIO_B;               /*!< Pointer to GPIOx for Rotary encode B pin. Meant for private use */
  GPIO_TypeDef* GPIO_BUTTON;          /*!< Pointer to GPIOx for Rotary encode B pin. Meant for private use */
  uint16_t      GPIO_PIN_A;           /*!< GPIO pin for rotary encoder A pin. This pin is also set for interrupt */
  uint16_t      GPIO_PIN_B;           /*!< GPIO pin for rotary encoder B pin. */
  uint16_t      GPIO_PIN_BUTTON;      /*!< GPIO pin for rotary encoder B pin. */
  RE_Click_t    pv_click;
  bool          direction;
  bool          halfPointReached;
} RE_State_t;

void RE_Init(RE_State_t* data, GPIO_TypeDef* GPIO_A_Port, uint16_t GPIO_A_Pin, GPIO_TypeDef* GPIO_B_Port, uint16_t GPIO_B_Pin, GPIO_TypeDef* GPIO_BUTTON_Port, uint16_t GPIO_BUTTON_Pin);
void RE_SetMode(RE_State_t* data, RE_Mode_t mode);
RE_Rotation_t RE_Get(RE_State_t* data);
void RE_Process(RE_State_t* data);
extern volatile RE_State_t RE1_Data;

#endif /* GENERALIO_ROTARY_ENCODER_H_ */

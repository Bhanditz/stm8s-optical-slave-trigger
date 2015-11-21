/*==============================================================================
 * MODULE: Main
 * DESCRIPTION: Main definitions
 *============================================================================*/
#ifndef _OT_MAIN_H_
#define _OT_MAIN_H_

#ifdef __cplusplus
extern "C"
{
#endif
/*==============================================================================
 * INCLUDES
 *============================================================================*/
#include <stm8s.h>
#include "config.h"
/*==============================================================================
 * CONSTANTS
 *============================================================================*/
/*==============================================================================
 * MACROS
 *============================================================================*/
// SENSOR_ENABLE is ActiveHigh
#define SENSOR_ON()    \
  GPIO_Init(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN, SENSOR_ENABLE_ON_MODE); \
  GPIO_WriteHigh(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN)
#define SENSOR_OFF()   \
  GPIO_WriteLow(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN); \
  GPIO_Init(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN, SENSOR_ENABLE_OFF_MODE)

#define TRIGGER_IN_ENABLE()   \
  GPIO_Init(TRIGGER_IN_PORT, TRIGGER_IN_PIN, TRIGGER_IN_ENABLE_MODE); \
  EXTI_SetExtIntSensitivity(TRIGGER_IN_EXTI_PORT, TRIGGER_IN_EXTI_SENSITIVITY)
#define TRIGGER_IN_DISABLE()  \
  GPIO_Init(TRIGGER_IN_PORT, TRIGGER_IN_PIN, TRIGGER_IN_DISABLE_MODE)

// TRIGGER_OUT is ActiveLow
#define TRIGGER_OUT_ON()      \
  GPIO_WriteLow(TRIGGER_OUT_PORT, TRIGGER_OUT_PIN); \
  GPIO_Init(TRIGGER_OUT_PORT, TRIGGER_OUT_PIN, TRIGGER_OUT_ON_MODE)
#define TRIGGER_OUT_OFF()     \
  GPIO_WriteHigh(TRIGGER_OUT_PORT, TRIGGER_OUT_PIN); \
  GPIO_Init(TRIGGER_OUT_PORT, TRIGGER_OUT_PIN, TRIGGER_OUT_OFF_MODE)

// GREEN_LED is ActiveLow
#define GREEN_LED_ON()        GPIO_WriteLow(GREEN_LED_PORT, GREEN_LED_PIN)
#define GREEN_LED_OFF()       GPIO_WriteHigh(GREEN_LED_PORT, GREEN_LED_PIN)
#define GREEN_LED_TOGGLE()    GPIO_WriteReverse(GREEN_LED_PORT, GREEN_LED_PIN)

// RED_LED is ActiveHigh
#define RED_LED_ON()          GPIO_WriteHigh(RED_LED_PORT, RED_LED_PIN)
#define RED_LED_OFF()         GPIO_WriteLow(RED_LED_PORT, RED_LED_PIN)

#if defined(WAKEUP_BUTTON)
  #define BUTTON_ENABLE()     \
    GPIO_Init(BUTTON_DET_PORT, BUTTON_DET_PIN, BUTTON_DET_ENABLE_MODE); \
    EXTI_SetExtIntSensitivity(BUTTON_DET_EXTI_PORT, BUTTON_DET_EXTI_SENSITIVITY)
  #define BUTTON_DISABLE()    \
    GPIO_Init(BUTTON_DET_PORT, BUTTON_DET_PIN, BUTTON_DET_DISABLE_MODE)
#endif // WAKEUP_BUTTON

#define DIP_ENABLE()    \
  GPIO_Init(DIP0_PORT, DIP0_PIN, DIP0_ENABLE_MODE); \
  GPIO_Init(DIP1_PORT, DIP1_PIN, DIP1_ENABLE_MODE); \
  GPIO_Init(DIP2_PORT, DIP2_PIN, DIP2_ENABLE_MODE)

#define DIP_DISABLE()   \
  GPIO_Init(DIP0_PORT, DIP0_PIN, DIP0_DISABLE_MODE); \
  GPIO_Init(DIP1_PORT, DIP1_PIN, DIP1_DISABLE_MODE); \
  GPIO_Init(DIP2_PORT, DIP2_PIN, DIP2_DISABLE_MODE)
/*==============================================================================
 * TYPEDEFs and STRUCTs
 *============================================================================*/
/*==============================================================================
 * GLOBAL (extern) VARIABLES
 *============================================================================*/
/*==============================================================================
 * EXPORTED (GLOBAL) FUNCTIONS
 *============================================================================*/
/*============================================================================*/
#ifdef __cplusplus
}
#endif

#endif /* _OT_MAIN_H_ */

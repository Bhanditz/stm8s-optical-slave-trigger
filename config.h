/*==============================================================================
 * MODULE: Config
 * DESCRIPTION: Configuration definitions
 *============================================================================*/
#ifndef _OT_CONFIG_H_
#define _OT_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif
/*==============================================================================
 * INCLUDES
 *============================================================================*/
#include <stm8s.h>
/*==============================================================================
 * CONSTANTS
 *============================================================================*/
// Feature Defines
#define DEBUG          // General debug
//#define TIMER_DEBUG    // Use timer to delay entry into READY state by 1 sec

// SENSOR_ENABLE Output Pushpull Low impedance Slow
#define SENSOR_ENABLE_PORT    GPIOA
#define SENSOR_ENABLE_PIN     GPIO_PIN_3

// TRIGGER_IN Input Floating Interrupt enabled
#define TRIGGER_IN_PORT       GPIOB
#define TRIGGER_IN_PIN        GPIO_PIN_7
#define TRIGGER_IN_EXTI_PORT  EXTI_PORT_GPIOB

// TRIGGER_OUT Output Pushpull Low impedance Slow
#define TRIGGER_OUT_PORT      GPIOA
#define TRIGGER_OUT_PIN       GPIO_PIN_5

// GREEN_LED Output Pushpull Low impedance Slow
#define GREEN_LED_PORT        GPIOD
#define GREEN_LED_PIN         GPIO_PIN_0

// RED_LED Output Pushpull Low impedance Slow
#define RED_LED_PORT          GPIOE
#define RED_LED_PIN           GPIO_PIN_7
/*==============================================================================
 * MACROS
 *============================================================================*/
// SENSOR_ENABLE is ActiveHigh
#define SENSOR_ENABLE_ON()    \
  GPIO_WriteHigh(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN)
#define SENSOR_ENABLE_OFF()   \
  GPIO_WriteLow(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN)

// TRIGGER_OUT is ActiveLow
#define TRIGGER_OUT_ON()      GPIO_WriteLow(TRIGGER_OUT_PORT, TRIGGER_OUT_PIN)
#define TRIGGER_OUT_OFF()     GPIO_WriteHigh(TRIGGER_OUT_PORT, TRIGGER_OUT_PIN)

// GREEN_LED is ActiveLow
#define GREEN_LED_ON()        GPIO_WriteLow(GREEN_LED_PORT, GREEN_LED_PIN)
#define GREEN_LED_OFF()       GPIO_WriteHigh(GREEN_LED_PORT, GREEN_LED_PIN)
#define GREEN_LED_TOGGLE()    GPIO_WriteReverse(GREEN_LED_PORT, GREEN_LED_PIN)

// RED_LED is ActiveHigh
#define RED_LED_ON()          GPIO_WriteHigh(RED_LED_PORT, RED_LED_PIN)
#define RED_LED_OFF()         GPIO_WriteLow(RED_LED_PORT, RED_LED_PIN)
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

#endif /* _OT_CONFIG_H_ */

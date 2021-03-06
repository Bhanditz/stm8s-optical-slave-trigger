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
// SENSOR_ENABLE Output Pushpull Low impedance Slow
#define SENSOR_ENABLE_PORT    GPIOA
#define SENSOR_ENABLE_PIN     GPIO_PIN_3
#define SENSOR_ENABLE_ON_MODE    GPIO_MODE_OUT_PP_LOW_SLOW
#define SENSOR_ENABLE_OFF_MODE   GPIO_MODE_IN_FL_NO_IT

// TRIGGER_IN Input Floating Interrupt enabled
#define TRIGGER_IN_PORT       GPIOB
#define TRIGGER_IN_PIN        GPIO_PIN_7
#define TRIGGER_IN_ENABLE_MODE     GPIO_MODE_IN_FL_IT
#define TRIGGER_IN_DISABLE_MODE    GPIO_MODE_IN_FL_NO_IT
#define TRIGGER_IN_EXTI_PORT           EXTI_PORT_GPIOB
#define TRIGGER_IN_EXTI_SENSITIVITY    EXTI_SENSITIVITY_RISE_ONLY

// TRIGGER_OUT Output Pushpull Low impedance Slow
#define TRIGGER_OUT_PORT      GPIOA
#define TRIGGER_OUT_PIN       GPIO_PIN_5
#define TRIGGER_OUT_ON_MODE    GPIO_MODE_OUT_PP_LOW_SLOW
#define TRIGGER_OUT_OFF_MODE   GPIO_MODE_IN_FL_NO_IT

// GREEN_LED Output Pushpull Low impedance Slow
#define GREEN_LED_PORT        GPIOD
#define GREEN_LED_PIN         GPIO_PIN_0

// RED_LED Output Pushpull Low impedance Slow
#define RED_LED_PORT          GPIOE
#define RED_LED_PIN           GPIO_PIN_7

#if defined(WAKEUP_BUTTON)
  // BUTTON_DET Input Pullup Interrupt enabled
  #define BUTTON_DET_PORT       GPIOC
  #define BUTTON_DET_PIN        GPIO_PIN_4
  #define BUTTON_DET_ENABLE_MODE    GPIO_MODE_IN_PU_IT
  #define BUTTON_DET_DISABLE_MODE   GPIO_MODE_IN_FL_NO_IT
  #define BUTTON_DET_EXTI_PORT           EXTI_PORT_GPIOC
  #define BUTTON_DET_EXTI_SENSITIVITY    EXTI_SENSITIVITY_FALL_ONLY
#endif // WAKEUP_BUTTON

// DIP[2:0] switches to set number of pre-flash bursts to ignore (0-6)
#define DIP0_PORT    GPIOD
#define DIP0_PIN     GPIO_PIN_2
#define DIP0_ENABLE_MODE    GPIO_MODE_IN_PU_NO_IT
#define DIP0_DISABLE_MODE   GPIO_MODE_IN_FL_NO_IT

#define DIP1_PORT    GPIOD
#define DIP1_PIN     GPIO_PIN_4
#define DIP1_ENABLE_MODE    GPIO_MODE_IN_PU_NO_IT
#define DIP1_DISABLE_MODE   GPIO_MODE_IN_FL_NO_IT

#define DIP2_PORT    GPIOD
#define DIP2_PIN     GPIO_PIN_6
#define DIP2_ENABLE_MODE    GPIO_MODE_IN_PU_NO_IT
#define DIP2_DISABLE_MODE   GPIO_MODE_IN_FL_NO_IT

#define MAX_DIP_SWITCHES   3

// Analog 'knob' for delay-based triggering when DIP[2:0] is set to 7.
#define DELAY_SENSE_PORT    GPIOB
#define DELAY_SENSE_PIN     GPIO_PIN_1
#define DELAY_SENSE_MODE    GPIO_MODE_IN_FL_NO_IT
#define DELAY_SENSE_ADC_CHANNEL       ADC1_CHANNEL_1
#define DELAY_SENSE_SCHMTRIG_CHANNEL  ADC1_SCHMITTTRIG_CHANNEL1
/*==============================================================================
 * MACROS
 *============================================================================*/
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

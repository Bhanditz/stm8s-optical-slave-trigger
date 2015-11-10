/*==============================================================================
 * MODULE: GPIO
 * DESCRIPTION: Abstraction for the GPIO mappings
 *============================================================================*/
/*==============================================================================
 * INCLUDES
 *============================================================================*/
#include "config.h"
#include "gpio.h"
/*==============================================================================
 * CONSTANTS
 *============================================================================*/
/*==============================================================================
 * MACROS
 *============================================================================*/
/*==============================================================================
 * TYPEDEFs and STRUCTs
 *============================================================================*/
/*==============================================================================
 * LOCAL VARIABLES
 *============================================================================*/
static OT_GPIO_CB_T *ot_gpio_cb    = (void*)0;
static void         *ot_gpio_cbarg = (void*)0;
/*==============================================================================
 * GLOBAL (extern) VARIABLES
 *============================================================================*/
/*==============================================================================
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/
/*==============================================================================
 * LOCAL FUNCTIONS
 *============================================================================*/
static void ot_gpio_isr(void) __interrupt(ITC_IRQ_PORTB) {
  // This ISR detects a flash burst. Inform the state machine.
  if ((void*)0 != ot_gpio_cb) (*ot_gpio_cb)(ot_gpio_cbarg);
  return;
}
/*==============================================================================
 * EXPORTED (GLOBAL) FUNCTIONS
 *============================================================================*/
/*==============================================================================
 * DESCRIPTION:
 * @param
 * @return
 * @precondition
 * @postcondition
 * @caution
 * @notes
 *============================================================================*/
void OT_GPIO_init(OT_GPIO_CB_T *cb, void *cbarg) {
  ot_gpio_cb    = cb;
  ot_gpio_cbarg = cbarg;

  GPIO_Init(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  SENSOR_ENABLE_ON();

  GPIO_Init(TRIGGER_IN_PORT, TRIGGER_IN_PIN, GPIO_MODE_IN_FL_IT);
  // TRIGGER_IN Sensitivity for Rising Edge
  EXTI_SetExtIntSensitivity(TRIGGER_IN_EXTI_PORT, EXTI_SENSITIVITY_RISE_ONLY);

  TRIGGER_OUT_OFF(); // Do this first to prevent accidental turn ON
  GPIO_Init(TRIGGER_OUT_PORT, TRIGGER_OUT_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);

  GPIO_Init(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  GREEN_LED_OFF();

  return;
}
/*============================================================================*/

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

  GPIO_Init(TRIGGER_IN_PORT, TRIGGER_IN_PIN, TRIGGER_IN_MODE);
  // TRIGGER_IN Sensitivity for Rising Edge
  EXTI_SetExtIntSensitivity(TRIGGER_IN_EXTI_PORT, TRIGGER_IN_EXTI_SENSITIVITY);

  TRIGGER_OUT_OFF(); // Do this first to prevent accidental turn ON
  GPIO_Init(TRIGGER_OUT_PORT, TRIGGER_OUT_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);

  GPIO_Init(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  GREEN_LED_OFF();

  GPIO_Init(RED_LED_PORT, RED_LED_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  RED_LED_OFF();

#if defined(WAKEUP_BUTTON)
  BUTTON_DISABLE();
#endif // WAKEUP_BUTTON

  GPIO_Init(DIP0_PORT, DIP0_PIN, DIP0_MODE);
  GPIO_Init(DIP1_PORT, DIP1_PIN, DIP1_MODE);
  GPIO_Init(DIP2_PORT, DIP2_PIN, DIP2_MODE);

  return;
}
/*==============================================================================
 * DESCRIPTION:
 * @param
 * @return
 * @precondition
 * @postcondition
 * @caution
 * @notes
 *============================================================================*/
uint8_t OT_GPIO_bursts_to_ignore(void) {
  // Interpret DIP2, DIP1, DIP0 as a binary value
  uint8_t retval = 0;
  if (RESET != GPIO_ReadInputPin(DIP0_PORT, DIP0_PIN)) {
    retval |= 0x01;
  }
  if (RESET != GPIO_ReadInputPin(DIP1_PORT, DIP1_PIN)) {
    retval |= 0x02;
  }
  if (RESET != GPIO_ReadInputPin(DIP2_PORT, DIP2_PIN)) {
    retval |= 0x04;
  }
  return retval;
}
/*==============================================================================
 * DESCRIPTION:
 * @param
 * @return
 * @precondition
 * @postcondition
 * @caution
 * @notes
 *============================================================================*/
INTERRUPT_HANDLER(ot_gpiob_isr, ITC_IRQ_PORTB) {
  // Inform the 'owner' module of this interrupt
  if ((void*)0 != ot_gpio_cb) (*ot_gpio_cb)(GPIOB, ot_gpio_cbarg);
  return;
}
/*==============================================================================
 * DESCRIPTION:
 * @param
 * @return
 * @precondition
 * @postcondition
 * @caution
 * @notes
 *============================================================================*/
#if defined(WAKEUP_BUTTON)
INTERRUPT_HANDLER(ot_gpioc_isr, ITC_IRQ_PORTC) {
  // Inform the 'owner' module of this interrupt
  if ((void*)0 != ot_gpio_cb) (*ot_gpio_cb)(GPIOC, ot_gpio_cbarg);
  return;
}
#endif // WAKEUP_BUTTON
/*============================================================================*/

/*==============================================================================
 * MODULE: main
 * DESCRIPTION: The main module for the Optical Slave Trigger
 *============================================================================*/
/*==============================================================================
 * INCLUDES
 *============================================================================*/
#include "config.h"
#include "gpio.h"
#include "timer.h"
#include "state_machine.h"
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
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/
// Callbacks
static OT_TIMER_CB_T ot_timer_cb;
static OT_GPIO_CB_T  ot_gpio_cb;
/*==============================================================================
 * LOCAL VARIABLES
 *============================================================================*/
/*==============================================================================
 * GLOBAL (extern) VARIABLES
 *============================================================================*/
/*==============================================================================
 * LOCAL FUNCTIONS
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
// Called by Timer module's interrupt upon expiry of a period (1msec)
static void ot_timer_cb(void *cbarg) {
  (void)cbarg; // Unused
  // Send Timeout event to State Machine
  OT_SM_execute(OT_SM_EVENT_TIMEOUT);
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
// Called by the GPIO module's interrupt upon detection of a flash burst
// or wake-up button press
static void ot_gpio_cb(GPIO_TypeDef *port, void *cbarg) {
  (void)cbarg; // Unused
  if (TRIGGER_IN_PORT == port) {
    // Send Flash Detected event to State Machine
    OT_SM_execute(OT_SM_EVENT_FLASH_DETECTED);
  }
#if defined(WAKEUP_BUTTON)
  else if (BUTTON_DET_PORT == port) {
    // Send Button Press event to State Machine
    OT_SM_execute(OT_SM_EVENT_BUTTON_PRESS);
  }
#endif // WAKEUP_BUTTON
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
/*============================================================================*/
void main(void) {
  /* Config Clock - N/A (Default is 2MHz) */
  OT_GPIO_init(ot_gpio_cb, (void*)0);
  OT_TIMER_init(ot_timer_cb, (void*)0);
  OT_SM_init();
  enableInterrupts();
  while (1) {
#if defined(WAKEUP_BUTTON)
    // Deep sleep (halt) when we want to wake up only due to an external
    // interrupt
    if (OT_SM_STATE_SLEEPING == OT_SM_get_state()) { halt(); }
    else
#endif // WAKEUP_BUTTON
    { wfi(); }
  }
}

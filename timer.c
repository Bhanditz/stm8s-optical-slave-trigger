/*==============================================================================
 * MODULE: Timer
 * DESCRIPTION: Uses TIM4 block to generate 1msec interrupts
 *============================================================================*/
/*==============================================================================
 * INCLUDES
 *============================================================================*/
#include <stm8s.h>
#include "timer.h"
/*==============================================================================
 * CONSTANTS
 *============================================================================*/
/*==============================================================================
 * MACROS
 *============================================================================*/
/*==============================================================================
 * TYPEDEFs and STRUCTs
 *============================================================================*/
typedef enum OT_TIMER_STATE_S {
  OT_TIMER_STATE_STOP,
  OT_TIMER_STATE_START,
  OT_TIMER_STATE_MAX    // Not a real state
} OT_TIMER_STATE_T;
/*==============================================================================
 * LOCAL VARIABLES
 *============================================================================*/
static OT_TIMER_STATE_T ot_timer_state  = OT_TIMER_STATE_STOP;
static OT_TIMER_CB_T    *ot_timer_cb    = (void*)0;
static void             *ot_timer_cbarg = (void*)0;
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
 * DESCRIPTION:
 * @param
 * @return
 * @precondition
 * @postcondition
 * @caution
 * @notes
 *============================================================================*/
static void tim4_isr_ovf(void) __interrupt(ITC_IRQ_TIM4_OVF) {
  if (TIM4_GetITStatus(TIM4_IT_UPDATE)) {
    if ((void*)0 != ot_timer_cb) {
      (*ot_timer_cb)(ot_timer_cbarg);
    }
    TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
  }
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
void OT_TIMER_init(OT_TIMER_CB_T *cb, void* cbarg) {
  ot_timer_cb = cb;
  ot_timer_cbarg = cbarg;
  OT_TIMER_stop();
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
void OT_TIMER_start(void) {
  // Stop currently running timer
  OT_TIMER_stop();
  // Set period
  TIM4_TimeBaseInit(TIM4_PRESCALER_16, 131); // 1msec period
  // Clear interrupts
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
  // Start timer
  TIM4_Cmd(ENABLE);
  ot_timer_state = OT_TIMER_STATE_START;
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
void OT_TIMER_stop(void) {
  TIM4_DeInit();
  ot_timer_state = OT_TIMER_STATE_STOP;
  return;
}
/*============================================================================*/

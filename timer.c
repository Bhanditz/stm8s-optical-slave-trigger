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
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/
static void ot_timer_busywait(uint16_t period);
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
 * DESCRIPTION:
 * @param
 * @return
 * @precondition - Assumes TIM3_DeInit() has been done by the caller
 * @postcondition
 * @caution
 * @notes
 *============================================================================*/
static void ot_timer_busywait(uint16_t period) {
  TIM3_TimeBaseInit(TIM3_PRESCALER_16, period);
  TIM3_ClearFlag(TIM3_FLAG_UPDATE);
  // No interrupts
  TIM3_Cmd(ENABLE);
  // Wait for the Update flag to be set again
  while (RESET == TIM3_GetFlagStatus(TIM3_FLAG_UPDATE));
  // If the Update flag is set then the time requested has expired
  TIM3_Cmd(DISABLE);
  return;
}
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
/*==============================================================================
 * DESCRIPTION:
 * @param
 * @return
 * @precondition
 * @postcondition
 * @caution
 * @notes
 *============================================================================*/
void OT_TIMER_busywait_ms(uint16_t delay_ms) {
  uint16_t delay_256ms, delay_16ms;
  TIM3_DeInit();

  delay_256ms = (delay_ms >> 8); // divide by 256
  delay_16ms  = ((delay_ms & 0x00FF) >> 4); // divide what's left by 16
  delay_ms    = (delay_ms & 0x000F); // what's left in msec

  /* For each 256msec of delay left, execute a 256msec busywait */
  while (delay_256ms-- > 0) {
    /* Note: with a 2MHz master clock & prescalar of 16, a count of 33554 gives
       approximately 256msec worth of delay */
    ot_timer_busywait(33554);
  }

  /* For each 16msec of delay left, execute a 16msec busywait */
  while (delay_16ms-- > 0) {
    /* Note: with a 2MHz master clock & prescalar of 16, a count of 2097 gives
       approximately 16msec worth of delay */
    ot_timer_busywait(2097);
  }

  /* For each msec of delay left, execute a 1msec busywait */
  while (delay_ms-- > 0) {
    /* Note: with a 2MHz master clock & prescalar of 16, a count of 131 gives
       approximately 1msec worth of delay */
    ot_timer_busywait(131);
  }
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
void tim4_isr_ovf(void) __interrupt(ITC_IRQ_TIM4_OVF) {
  if (TIM4_GetITStatus(TIM4_IT_UPDATE)) {
    if ((void*)0 != ot_timer_cb) {
      (*ot_timer_cb)(ot_timer_cbarg);
    }
    TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
  }
  return;
}
/*============================================================================*/

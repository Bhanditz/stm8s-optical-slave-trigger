/*==============================================================================
 * MODULE: Timer
 * DESCRIPTION: Implementation of timer functions
 * @todo - remove 'magic' numbers from this module
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
static void ot_timer_busywait(void);
static void ot_timer_busywait16(uint16_t period);
static void ot_timer_busywait1(uint16_t period);
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
 * @precondition - Assumes TIMx_DeInit() has been done by the caller and that
 *                 the appropriate PRESCALER has been set.
 * @postcondition
 * @caution
 * @notes
 *============================================================================*/
static void ot_timer_busywait(void) {
  // No interrupts
  disableInterrupts();
#if defined(STM8S105)
  TIM3_ClearFlag(TIM3_FLAG_UPDATE);
  TIM3_Cmd(ENABLE);
  // Wait for the Update flag to be set again
  while (RESET == TIM3_GetFlagStatus(TIM3_FLAG_UPDATE));
  // If the Update flag is set then the time requested has expired
  TIM3_Cmd(DISABLE);
#elif defined(STM8S903)
  TIM5_ClearFlag(TIM5_FLAG_UPDATE);
  TIM5_Cmd(ENABLE);
  // Wait for the Update flag to be set again
  while (RESET == TIM5_GetFlagStatus(TIM5_FLAG_UPDATE));
  // If the Update flag is set then the time requested has expired
  TIM5_Cmd(DISABLE);
#else
  #error "ot_timer_busywait not implemented"
#endif
  enableInterrupts();
  return;
}
/*==============================================================================
 * DESCRIPTION: Busywait for msec delays. Sets the TIMx prescaler to 16. Each
 * 'tick' corresponds to ~7.63usec for a master clock of 2MHz
 * @param
 * @return
 * @precondition - Assumes TIMx_DeInit() has been done by the caller.
 * @postcondition
 * @caution
 * @notes
 *============================================================================*/
static void ot_timer_busywait16(uint16_t period) {
#if defined(STM8S105)
  TIM3_TimeBaseInit(TIM3_PRESCALER_16, period);
#elif defined(STM8S903)
  TIM5_TimeBaseInit(TIM5_PRESCALER_16, period);
#else
  #error "ot_timer_busywait16 not implemented"
#endif
  ot_timer_busywait();
  return;
}
/*==============================================================================
 * DESCRIPTION: Busywait for usec delays (more accurate for 100us and greater).
 * Sets the TIMx prescaler to 1. Each 'tick' corresponds to ~0.47usec for a
 * master clock of 2MHz.
 * @param
 * @return
 * @precondition - Assumes TIMx_DeInit() has been done by the caller.
 * @postcondition
 * @caution
 * @notes
 *============================================================================*/
static void ot_timer_busywait1(uint16_t period) {
#if defined(STM8S105)
  TIM3_TimeBaseInit(TIM3_PRESCALER_1, period);
#elif defined(STM8S903)
  TIM5_TimeBaseInit(TIM5_PRESCALER_1, period);
#else
  #error "ot_timer_busywait1 not implemented"
#endif
  ot_timer_busywait();
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
#if defined(STM8S105)
  // Set period
  TIM4_TimeBaseInit(TIM4_PRESCALER_16, 131); // 1msec period
  // Clear interrupts
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
  // Start timer
  TIM4_Cmd(ENABLE);
#elif defined(STM8S903)
  // Set period
  TIM6_TimeBaseInit(TIM6_PRESCALER_16, 131); // 1msec period
  // Clear interrupts
  TIM6_ClearFlag(TIM6_FLAG_UPDATE);
  TIM6_ITConfig(TIM6_IT_UPDATE, ENABLE);
  // Start timer
  TIM6_Cmd(ENABLE);
#else
  #error "OT_TIMER_start not implemented"
#endif
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
#if defined(STM8S105)
  TIM4_DeInit();
#elif defined(STM8S903)
  TIM6_DeInit();
#else
  #error "OT_TIMER_stop not implemented"
#endif
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
  uint16_t delay_256ms;
  uint16_t delay_16ms;

#if defined(STM8S105)
  TIM3_DeInit();
#elif defined(STM8S903)
  TIM5_DeInit();
#else
  #error "OT_TIMER_busywait_ms not implemented"
#endif

  delay_256ms = (delay_ms >> 8); // divide by 256
  delay_16ms  = ((delay_ms & 0x00FF) >> 4); // divide what's left by 16
  delay_ms    = (delay_ms & 0x000F); // what's left in msec

  /* For each 256msec of delay left, execute a 256msec busywait */
  while (delay_256ms-- > 0) {
    /* Note: with a 2MHz master clock & prescalar of 16, a count of 33554 gives
       approximately 256msec worth of delay */
    ot_timer_busywait16(33554);
  }

  /* For each 16msec of delay left, execute a 16msec busywait */
  while (delay_16ms-- > 0) {
    /* Note: with a 2MHz master clock & prescalar of 16, a count of 2097 gives
       approximately 16msec worth of delay */
    ot_timer_busywait16(2097);
  }

  /* Execute busywait for the rest of delay_ms (< 16msec) */
  /* Note: with a 2MHz master clock & prescalar of 16, a count of 131 gives
     approximately 1msec worth of delay */
  ot_timer_busywait16(131*delay_ms);

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
void OT_TIMER_busywait_us(uint16_t delay_us) {
  uint16_t delay_256us;
  uint16_t delay_16us;

#if defined(STM8S105)
  TIM3_DeInit();
#elif defined(STM8S903)
  TIM5_DeInit();
#else
  #error "OT_TIMER_busywait_us not implemented"
#endif

  delay_256us = (delay_us >> 8); // divide by 256
  delay_16us  = ((delay_us & 0x00FF) >> 4); // divide what's left by 16
  delay_us    = (delay_us & 0x000F); // what's left in usec

  /* For each 256usec of delay left, execute a 256usec busywait */
  while (delay_256us-- > 0) {
    /* Note: with a 2MHz master clock & prescalar of 1, a count of 537 gives
       approximately 256usec worth of delay */
    ot_timer_busywait1(537);
  }

  /* For each 16usec of delay left, execute a 16usec busywait */
  while (delay_16us-- > 0) {
    /* Note: with a 2MHz master clock & prescalar of 1, a count of 34 gives
       approximately 16usec worth of delay */
    ot_timer_busywait1(34);
  }

  /* Execute busywait for the rest of delay_us (< 16usec) */
  /* Note: with a 2MHz master clock & prescalar of 1, a count of 2 gives
     approximately 1usec worth of delay */
  ot_timer_busywait1(2*delay_us);

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
#if defined(STM8S105)
INTERRUPT_HANDLER(tim4_isr_ovf, ITC_IRQ_TIM4_OVF) {
  if (TIM4_GetITStatus(TIM4_IT_UPDATE)) {
    if ((void*)0 != ot_timer_cb) {
      (*ot_timer_cb)(ot_timer_cbarg);
    }
    TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
  }
  return;
}
#elif defined(STM8S903)
INTERRUPT_HANDLER(tim6_isr_ovf, ITC_IRQ_TIM6_OVFTRI) {
  if (TIM6_GetITStatus(TIM6_IT_UPDATE)) {
    if ((void*)0 != ot_timer_cb) {
      (*ot_timer_cb)(ot_timer_cbarg);
    }
    TIM6_ClearITPendingBit(TIM6_IT_UPDATE);
  }
  return;
}
#else
  #error "timx INTERRUPT_HANDLER not implemented"
#endif
/*============================================================================*/

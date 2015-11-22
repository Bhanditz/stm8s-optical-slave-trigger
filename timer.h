/*==============================================================================
 * MODULE: Timer
 * DESCRIPTION: Prototypes exported by the Timer module
 *============================================================================*/
#ifndef _OT_TIMER_H_
#define _OT_TIMER_H_

#ifdef __cplusplus
extern "C"
{
#endif
/*==============================================================================
 * INCLUDES
 *============================================================================*/
/*==============================================================================
 * CONSTANTS
 *============================================================================*/
/*==============================================================================
 * MACROS
 *============================================================================*/
/*==============================================================================
 * TYPEDEFs and STRUCTs
 *============================================================================*/
typedef void (OT_TIMER_CB_T)(void* cbarg);
/*==============================================================================
 * GLOBAL (extern) VARIABLES
 *============================================================================*/
/*==============================================================================
 * EXPORTED (GLOBAL) FUNCTIONS
 *============================================================================*/
void OT_TIMER_init(OT_TIMER_CB_T *cb, void* cbarg);
void OT_TIMER_start(void);
void OT_TIMER_stop(void);
void OT_TIMER_busywait_ms(uint16_t delay_ms);
void OT_TIMER_busywait_us(uint16_t delay_us);
#if defined(_SDCC_)
  // The SDCC compiler requires the main module to know interrupt prototypes
  #if defined(STM8S105)
    INTERRUPT_HANDLER(tim4_isr_ovf, ITC_IRQ_TIM4_OVF);
  #elif defined(STM8S903)
    INTERRUPT_HANDLER(tim6_isr_ovf, ITC_IRQ_TIM6_OVFTRI);
  #else
    #error "timx INTERRUPT_HANDLER not implemented"
  #endif
#endif // _SDCC_
/*============================================================================*/
#ifdef __cplusplus
}
#endif

#endif /* _OT_CONFIG_H_ */

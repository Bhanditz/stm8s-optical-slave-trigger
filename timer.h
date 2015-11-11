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
void tim4_isr_ovf(void) __interrupt(ITC_IRQ_TIM4_OVF);
/*============================================================================*/
#ifdef __cplusplus
}
#endif

#endif /* _OT_CONFIG_H_ */
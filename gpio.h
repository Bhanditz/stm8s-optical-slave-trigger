/*==============================================================================
 * MODULE: GPIO
 * DESCRIPTION: Prototypes exported by the GPIO module
 *============================================================================*/
#ifndef _OT_GPIO_H_
#define _OT_GPIO_H_

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
/*==============================================================================
 * MACROS
 *============================================================================*/
/*==============================================================================
 * TYPEDEFs and STRUCTs
 *============================================================================*/
typedef void (OT_GPIO_CB_T)(GPIO_TypeDef *port, void *cbarg);
/*==============================================================================
 * GLOBAL (extern) VARIABLES
 *============================================================================*/
/*==============================================================================
 * EXPORTED (GLOBAL) FUNCTIONS
 *============================================================================*/
void OT_GPIO_init(OT_GPIO_CB_T *cb, void *cbarg);
#if defined(_SDCC_)
  // The SDCC compiler requires the main module to know interrupt prototypes
  INTERRUPT_HANDLER(ot_gpiob_isr, ITC_IRQ_PORTB);
  INTERRUPT_HANDLER(ot_gpioc_isr, ITC_IRQ_PORTC);
#endif // _SDCC_
/*============================================================================*/
#ifdef __cplusplus
}
#endif

#endif /* _OT_GPIO_H_ */

/*==============================================================================
 * MODULE: 
 * DESCRIPTION:
 *============================================================================*/
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

// TRIGGER_IN Input Floating Interrupt enabled
#define TRIGGER_IN_PORT       GPIOB
#define TRIGGER_IN_PIN        GPIO_PIN_7
#define TRIGGER_IN_EXTI_PORT  EXTI_PORT_GPIOB

// TRIGGER_OUT Output Pushpull Low impedance Slow
#define TRIGGER_OUT_PORT      GPIOA
#define TRIGGER_OUT_PIN       GPIO_PIN_5

// GREEN_LED Output Pushpull Low impedance Slow
#define GREEN_LED_PORT        GPIOD
#define GREEN_LED_PIN         GPIO_PIN_0
/*==============================================================================
 * MACROS
 *============================================================================*/
// SENSOR_ENABLE is ActiveHigh
#define SENSOR_ENABLE_ON()    GPIO_WriteHigh(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN)
#define SENSOR_ENABLE_OFF()   GPIO_WriteLow(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN)

// TRIGGER_OUT is ActiveLow
#define TRIGGER_OUT_ON()      GPIO_WriteLow(TRIGGER_OUT_PORT, TRIGGER_OUT_PIN)
#define TRIGGER_OUT_OFF()     GPIO_WriteHigh(TRIGGER_OUT_PORT, TRIGGER_OUT_PIN)

// GREEN_LED is ActiveLow
#define GREEN_LED_OFF()       GPIO_WriteHigh(GREEN_LED_PORT, GREEN_LED_PIN)
#define GREEN_LED_TOGGLE()    GPIO_WriteReverse(GREEN_LED_PORT, GREEN_LED_PIN)
/*==============================================================================
 * TYPEDEFs and STRUCTs
 *============================================================================*/
typedef enum OT_STATE_E {
  OT_STATE_INIT,         // Transient
  //OT_STATE_ARMING,     // Unused
  OT_STATE_READY,
  OT_STATE_PROVISIONAL,
  OT_STATE_CONFIRMED,
  OT_STATE_MAX           // Not a real state
} OT_STATE_T;

typedef enum OT_EVENT_E {
  OT_EVENT_INIT_COMPLETE,
  OT_EVENT_FLASH_DETECTED,
  OT_EVENT_TIMEOUT,
  OT_EVENT_MAX              // Not a real event
} OT_EVENT_T;

typedef void (OT_STATE_ACTION_FUNC_T)(OT_EVENT_T);

// State Handlers
OT_STATE_ACTION_FUNC_T ot_state_init;
OT_STATE_ACTION_FUNC_T ot_state_ready;
OT_STATE_ACTION_FUNC_T ot_state_provisional;
OT_STATE_ACTION_FUNC_T ot_state_confirmed;
/*==============================================================================
 * LOCAL VARIABLES
 *============================================================================*/
static OT_STATE_T ot_state = OT_STATE_INIT;

// CAUTION: This is in the order of the OT_STATE_T enumeration
static OT_STATE_ACTION_FUNC_T *ot_handlers[OT_STATE_MAX] = {
  &ot_state_init,        // OT_STATE_INIT
  &ot_state_ready,       // OT_STATE_READY
  &ot_state_provisional, // OT_STATE_PROVISIONAL
  &ot_state_confirmed    // OT_STATE_CONFIRMED
};
/*==============================================================================
 * GLOBAL (extern) VARIABLES
 *============================================================================*/
/*==============================================================================
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/
static void ot_state_init(OT_EVENT_T event) {
  (void)event; // TBD - implement actions based on event
  return;
}

static void ot_state_ready(OT_EVENT_T event) {
  (void)event; // TBD - implement actions based on event
  return;
}

static void ot_state_provisional(OT_EVENT_T event) {
  (void)event; // TBD - implement actions based on event
  return;
}

static void ot_state_confirmed(OT_EVENT_T event) {
  (void)event; // TBD - implement actions based on event
  return;
}

static void ot_state_machine(OT_EVENT_T event) {
  if (event < OT_EVENT_MAX) {
    OT_STATE_ACTION_FUNC_T *handler = ot_handlers[ot_state];
    if ((void*)0 != handler) {
      (*handler)(event);
    }
  }
}

/*==============================================================================
 * LOCAL FUNCTIONS
 *============================================================================*/
static void ot_gpio_isr(void) __interrupt(ITC_IRQ_PORTB) {
  GREEN_LED_TOGGLE(); /* Toggle GREEN_LED every interrupt */
  return;
}

static void ot_gpio_config(void) {
  GPIO_Init(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  SENSOR_ENABLE_ON();

  GPIO_Init(TRIGGER_IN_PORT, TRIGGER_IN_PIN, GPIO_MODE_IN_FL_IT);
  // Enable TRIGGER_IN Sensitivity for Rising Edge
  EXTI_SetExtIntSensitivity(TRIGGER_IN_EXTI_PORT, EXTI_SENSITIVITY_RISE_ONLY);

  GPIO_Init(TRIGGER_OUT_PORT, TRIGGER_OUT_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  TRIGGER_OUT_OFF();

  GPIO_Init(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  GREEN_LED_OFF();

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
  /* Config Clock - N/A */
  ot_gpio_config();


  enableInterrupts();
  while (1) { wfi(); }
}

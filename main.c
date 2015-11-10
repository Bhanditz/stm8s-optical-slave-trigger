/*==============================================================================
 * MODULE: main
 * DESCRIPTION: The main module for the Optical Slave Trigger
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

// State Machine Transition/Event Handlers
typedef void (OT_STATE_ENTRY_FUNC_T)(void);
typedef void (OT_STATE_ACTION_FUNC_T)(OT_EVENT_T);
typedef void (OT_STATE_EXIT_FUNC_T)(void);

typedef struct OT_STATE_HANDLERS_S {
  OT_STATE_ENTRY_FUNC_T  *entryp;
  OT_STATE_ACTION_FUNC_T *actionp;
  OT_STATE_EXIT_FUNC_T   *exitp;
} OT_STATE_HANDLERS_T;
/*==============================================================================
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/
// State entry/action/exit handlers
static OT_STATE_ACTION_FUNC_T ot_state_init_action; // No entry, exit
static OT_STATE_ACTION_FUNC_T ot_state_ready_action; // No entry, exit
static OT_STATE_ENTRY_FUNC_T  ot_state_provisional_entry;
static OT_STATE_ACTION_FUNC_T ot_state_provisional_action;
static OT_STATE_EXIT_FUNC_T   ot_state_provisional_exit;
static OT_STATE_ENTRY_FUNC_T  ot_state_confirmed_entry;
static OT_STATE_ACTION_FUNC_T ot_state_confirmed_action;
static OT_STATE_EXIT_FUNC_T   ot_state_confirmed_exit;

static void ot_set_state(OT_STATE_T state_in);
static void ot_state_machine(OT_EVENT_T event);
static void ot_gpio_config(void);
/*==============================================================================
 * LOCAL VARIABLES
 *============================================================================*/
static OT_STATE_T volatile ot_state = OT_STATE_INIT;

// CAUTION: This array is in the order of the OT_STATE_T enumeration
static OT_STATE_HANDLERS_T ot_handlers[OT_STATE_MAX] = {
  // entry                       action                        exit
  { (void*)0,                    &ot_state_init_action,        (void*)0                   }, // OT_STATE_INIT
  { (void*)0,                    &ot_state_ready_action,       (void*)0                   }, // OT_STATE_READY
  { &ot_state_provisional_entry, &ot_state_provisional_action, &ot_state_provisional_exit }, // OT_STATE_PROVISIONAL
  { &ot_state_confirmed_entry,   &ot_state_confirmed_action,   &ot_state_confirmed_exit   }  // OT_STATE_CONFIRMED
};
/*==============================================================================
 * GLOBAL (extern) VARIABLES
 *============================================================================*/
/*==============================================================================
 * LOCAL FUNCTIONS
 *============================================================================*/
static void ot_set_state(OT_STATE_T state_in) {
  if (state_in < OT_STATE_MAX && ot_state < OT_STATE_MAX) {
    OT_STATE_ENTRY_FUNC_T *entryp;
    OT_STATE_EXIT_FUNC_T  *exitp;

    // First execute the exit function of the current ot_state, if any
    entryp = ot_handlers[ot_state].entryp;
    if ((void*)0 != entryp) (*entryp)();

    // Update our state
    ot_state = state_in;

    // Finally execute the entry function of the new ot_state, if any
    exitp = ot_handlers[ot_state].exitp;
    if ((void*)0 != exitp) (*exitp)();
  }
  return;
}

static void ot_state_init_action(OT_EVENT_T event) {
  if (OT_EVENT_INIT_COMPLETE == event) {
    ot_set_state(OT_STATE_READY);
  }
  // Ignore all other events and stay in the same state
  return;
}

static void ot_state_ready_action(OT_EVENT_T event) {
  if (OT_EVENT_FLASH_DETECTED == event) {
    ot_set_state(OT_STATE_PROVISIONAL);
  }
  // Ignore all other events and stay in the same state
  return;
}

static void ot_state_provisional_entry(void) {
  // @todo - set a 100msec timer
}

static void ot_state_provisional_action(OT_EVENT_T event) {
  if (OT_EVENT_FLASH_DETECTED == event) { // Possibly part of the 'red eye' reduction or 'pre-flashes'
    // @todo - reset our state timer back to 100msec
    // stay in this state
  }
  else if (OT_EVENT_TIMEOUT == event) {
    // We waited long enough after the last burst. No more 'red eye' or 'preflashes' are incoming.
    ot_set_state(OT_STATE_CONFIRMED);
  }
  // Ignore all other events and stay in the same state
  return;
}

static void ot_state_provisional_exit(void) {
  // @todo - cancel/stop state timer
}

static void ot_state_confirmed_entry(void) {
  // @todo - set a 10msec timer
  TRIGGER_OUT_ON(); // Trigger the slave flash
}

static void ot_state_confirmed_action(OT_EVENT_T event) {
  if (OT_EVENT_TIMEOUT == event) {
    // @todo - ideally should go to ARMING but that's not used in our current design
    ot_set_state(OT_STATE_READY);
  }
  // Ignore all other events and stay in the same state
  return;
}

static void ot_state_confirmed_exit(void) {
  TRIGGER_OUT_OFF(); // Release the slave flash trigger
  // @todo - cancel/stop state timer if it's active
}

static void ot_state_machine(OT_EVENT_T event) {
  if (event < OT_EVENT_MAX && ot_state < OT_STATE_MAX) {
    OT_STATE_ACTION_FUNC_T *actionp;
    actionp = ot_handlers[ot_state].actionp;
    if ((void*)0 != actionp) (*actionp)(event);
  }
}

static void ot_gpio_isr(void) __interrupt(ITC_IRQ_PORTB) {
  // This ISR detects a flash burst. Inform the state machine.
  ot_state_machine(OT_EVENT_FLASH_DETECTED);
  //GREEN_LED_TOGGLE(); /* Debug: Toggle GREEN_LED every interrupt */
  return;
}

static void ot_gpio_config(void) {
  GPIO_Init(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN, GPIO_MODE_OUT_PP_LOW_SLOW);
  SENSOR_ENABLE_ON();

  GPIO_Init(TRIGGER_IN_PORT, TRIGGER_IN_PIN, GPIO_MODE_IN_FL_IT);
  // TRIGGER_IN Sensitivity for Rising Edge
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
  // Send a dummy 'init done' message to the state machine so it's ready to handle Flash bursts
  ot_state_machine(OT_EVENT_INIT_COMPLETE);
  enableInterrupts();
  while (1) { wfi(); }
}

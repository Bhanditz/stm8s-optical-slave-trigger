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
// Feature Defines
#define TIMER_DEBUG    // Enabling to delay entry into READY state by 1 sec


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

// RED_LED Output Pushpull Low impedance Slow
#define RED_LED_PORT          GPIOE
#define RED_LED_PIN           GPIO_PIN_7

// Constants
#define OT_INIT_TIMEOUT_MS           1000 // Used only if TIMER_DEBUG is defined
#define OT_PROVISIONAL_TIMEOUT_MS    100
#define OT_CONFIRMED_TIMEOUT_MS      10


/*==============================================================================
 * MACROS
 *============================================================================*/
// SENSOR_ENABLE is ActiveHigh
#define SENSOR_ENABLE_ON()    \
  GPIO_WriteHigh(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN)
#define SENSOR_ENABLE_OFF()   \
  GPIO_WriteLow(SENSOR_ENABLE_PORT, SENSOR_ENABLE_PIN)

// TRIGGER_OUT is ActiveLow
#define TRIGGER_OUT_ON()      \
  GPIO_WriteLow(TRIGGER_OUT_PORT, TRIGGER_OUT_PIN)
#define TRIGGER_OUT_OFF()     \
  GPIO_WriteHigh(TRIGGER_OUT_PORT, TRIGGER_OUT_PIN)

// GREEN_LED is ActiveLow
#define GREEN_LED_ON()        \
  GPIO_WriteLow(GREEN_LED_PORT, GREEN_LED_PIN)
#define GREEN_LED_OFF()       \
  GPIO_WriteHigh(GREEN_LED_PORT, GREEN_LED_PIN)
#define GREEN_LED_TOGGLE()    \
  GPIO_WriteReverse(GREEN_LED_PORT, GREEN_LED_PIN)

// RED_LED is ActiveHigh
#define RED_LED_ON()          GPIO_WriteHigh(RED_LED_PORT, RED_LED_PIN)
#define RED_LED_OFF()         GPIO_WriteLow(RED_LED_PORT, RED_LED_PIN)
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

typedef struct OT_DATA_S {
  OT_STATE_T volatile ot_state;
  uint8_t    volatile burst_count;
  uint16_t   volatile timeout_ms;
} OT_DATA_T;
/*==============================================================================
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/
// State entry/action/exit handlers
static OT_STATE_ENTRY_FUNC_T  ot_state_init_entry;
static OT_STATE_ACTION_FUNC_T ot_state_init_action;
static OT_STATE_EXIT_FUNC_T   ot_state_init_exit;
static OT_STATE_ENTRY_FUNC_T  ot_state_ready_entry;
static OT_STATE_ACTION_FUNC_T ot_state_ready_action;
static OT_STATE_EXIT_FUNC_T   ot_state_ready_exit;
static OT_STATE_ENTRY_FUNC_T  ot_state_provisional_entry;
static OT_STATE_ACTION_FUNC_T ot_state_provisional_action;
static OT_STATE_EXIT_FUNC_T   ot_state_provisional_exit;
static OT_STATE_ENTRY_FUNC_T  ot_state_confirmed_entry;
static OT_STATE_ACTION_FUNC_T ot_state_confirmed_action;
static OT_STATE_EXIT_FUNC_T   ot_state_confirmed_exit;

static void ot_set_state(OT_STATE_T state_in);
static void ot_state_machine(OT_EVENT_T event);
static void ot_gpio_config(void);
static void ot_timer_stop(void);
static void ot_timer_start(void);
/*==============================================================================
 * LOCAL VARIABLES
 *============================================================================*/
// CAUTION: This array is in the order of the OT_STATE_T enumeration
static OT_STATE_HANDLERS_T ot_handlers[OT_STATE_MAX] = {
  // OT_STATE_INIT
  {
    &ot_state_init_entry,
    &ot_state_init_action,
    &ot_state_init_exit
  },
  // OT_STATE_READY
  {
    &ot_state_ready_entry,
    &ot_state_ready_action,
    &ot_state_ready_exit
  },
  // OT_STATE_PROVISIONAL
  {
    &ot_state_provisional_entry,
    &ot_state_provisional_action,
    &ot_state_provisional_exit
  },
  // OT_STATE_CONFIRMED
  {
    &ot_state_confirmed_entry,
    &ot_state_confirmed_action,
    &ot_state_confirmed_exit
  }
};

static OT_DATA_T ot_data = {
  .ot_state    = OT_STATE_INIT,
  .burst_count = 0,
  .timeout_ms  = 0
};
/*==============================================================================
 * GLOBAL (extern) VARIABLES
 *============================================================================*/
/*==============================================================================
 * LOCAL FUNCTIONS
 *============================================================================*/
static void ot_set_state(OT_STATE_T state_in) {
  if (state_in < OT_STATE_MAX && ot_data.ot_state < OT_STATE_MAX) {
    OT_STATE_ENTRY_FUNC_T *entryp;
    OT_STATE_EXIT_FUNC_T  *exitp;

    // Note that we allow ourselves to re-enter the current state

    // First execute the exit function of the current ot_state, if any
    entryp = ot_handlers[ot_data.ot_state].entryp;
    if ((void*)0 != entryp) (*entryp)();

    // Update our state
    ot_data.ot_state = state_in;

    // Finally execute the entry function of the new ot_state, if any
    exitp = ot_handlers[ot_data.ot_state].exitp;
    if ((void*)0 != exitp) (*exitp)();
  }
  return;
}

static void ot_state_init_entry(void) {
#if defined(TIMER_DEBUG)
  ot_data.timeout_ms = OT_INIT_TIMEOUT_MS;
  ot_timer_start(); // sends TIMEOUT events every ~1msec
#endif // TIMER_DEBUG
  return;
}

static void ot_state_init_action(OT_EVENT_T event) {
#if defined(TIMER_DEBUG)
  if (OT_EVENT_TIMEOUT == event) {
    if (0 == --ot_data.timeout_ms) {
      ot_set_state(OT_STATE_READY);
    }
  }
#else
  // Wait for the main function to send us a INIT_COMPLETE
  if (OT_EVENT_INIT_COMPLETE == event) {
    ot_set_state(OT_STATE_READY);
  }
#endif // TIMER_DEBUG
  // Ignore all other events and stay in the same state
  return;
}

static void ot_state_init_exit(void) {
#if defined(TIMER_DEBUG)
  // cancel/stop state timer
  ot_timer_stop();
  ot_data.timeout_ms = 0;
#endif // TIMER_DEBUG
  return;
}

static void ot_state_ready_entry(void) {
  GREEN_LED_ON(); // Turn ON GREEN LED to show we're in READY
  ot_data.burst_count = 0; // Reset our internal counters
  return;
}

static void ot_state_ready_action(OT_EVENT_T event) {
  if (OT_EVENT_FLASH_DETECTED == event) {
    ot_set_state(OT_STATE_PROVISIONAL);
  }
  // Ignore all other events and stay in the same state
  return;
}

static void ot_state_ready_exit(void) {
  GREEN_LED_OFF(); // Turn OFF GREEN LED to show we've exited READY
}

static void ot_state_provisional_entry(void) {
  // If we entered this state we just detected ONE flash burst
  ot_data.burst_count = 1;
  ot_data.timeout_ms = OT_PROVISIONAL_TIMEOUT_MS;
  ot_timer_start(); // sends TIMEOUT events every ~1msec
  return;
}

static void ot_state_provisional_action(OT_EVENT_T event) {
  if (OT_EVENT_FLASH_DETECTED == event) {
    // Possibly part of the 'red eye' reduction or 'pre-flashes'
    // Increment our count of flash bursts detected
    ++ot_data.burst_count;
    // reset our state timer
    ot_data.timeout_ms = OT_PROVISIONAL_TIMEOUT_MS;
    ot_timer_start(); // sends TIMEOUT events every ~1msec
    // stay in this state
  }
  else if (OT_EVENT_TIMEOUT == event) {
    // decrement out timeout_ms count
    if (0 == --ot_data.timeout_ms) { // Waiting period has expired
      // We waited long enough after the last burst.
      // No more 'red eye' or 'preflashes' are incoming.
      ot_set_state(OT_STATE_CONFIRMED);
    }
  }
  // Ignore all other events and stay in the same state
  return;
}

static void ot_state_provisional_exit(void) {
  // cancel/stop state timer
  ot_timer_stop();
  ot_data.timeout_ms = 0;
  return;
}

static void ot_state_confirmed_entry(void) {
  TRIGGER_OUT_ON(); // Trigger the slave flash
  // set a 10msec timer
  ot_data.timeout_ms = OT_CONFIRMED_TIMEOUT_MS;
  ot_timer_start();
}

static void ot_state_confirmed_action(OT_EVENT_T event) {
  if (OT_EVENT_TIMEOUT == event) {
    if (0 == --ot_data.timeout_ms) { // Waiting period has expired
      // We waited long enough for the slave flash to have fired.
      ot_set_state(OT_STATE_READY);
      // @todo - ideally should go to ARMING but that's not used in our
      //         current design
    }
  }
  // Ignore all other events and stay in the same state
  return;
}

static void ot_state_confirmed_exit(void) {
  ot_timer_stop();
  ot_data.timeout_ms = 0;
  TRIGGER_OUT_OFF(); // Release the slave flash trigger
  return;
}

static void ot_state_machine(OT_EVENT_T event) {
  if (event < OT_EVENT_MAX && ot_data.ot_state < OT_STATE_MAX) {
    OT_STATE_ACTION_FUNC_T *actionp;
    actionp = ot_handlers[ot_data.ot_state].actionp;
    if ((void*)0 != actionp) (*actionp)(event);
  }
  return;
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

static void tim4_isr_ovf(void) __interrupt(ITC_IRQ_TIM4_OVF) {
  if (TIM4_GetITStatus(TIM4_IT_UPDATE)) {
    ot_state_machine(OT_EVENT_TIMEOUT);
    TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
  }
  return;
}

static void ot_timer_stop(void) {
  TIM4_DeInit();
  return;
}

static void ot_timer_start(void) {
  // Stop currently running timer
  ot_timer_stop();
  // Set period
  TIM4_TimeBaseInit(TIM4_PRESCALER_16, 131); // 1msec period
  // Clear interrupts
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
  // Start timer
  TIM4_Cmd(ENABLE);
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
  ot_set_state(OT_STATE_INIT);
#if !defined(TIMER_DEBUG)
  // Send a 'init done' message to the state machine so it's ready to handle
  // Flash bursts
  ot_state_machine(OT_EVENT_INIT_COMPLETE);
#endif // TIMER_DEBUG
  enableInterrupts();
  while (1) { wfi(); }
}


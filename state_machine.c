/*==============================================================================
 * MODULE: State Machine (SM)
 * DESCRIPTION: Implementation of the SM module
 *============================================================================*/
/*==============================================================================
 * INCLUDES
 *============================================================================*/
#include "config.h"
#include "gpio.h"
#include "timer.h"
#include "adc.h"
#include "state_machine.h"
/*==============================================================================
 * CONSTANTS
 *============================================================================*/
#define OT_SM_INIT_TIMEOUT_MS           100
#define OT_SM_READY_TIMEOUT_MS          60000 // #if defined(WAKEUP_BUTTON)
#define OT_SM_PROVISIONAL_TIMEOUT_MS    100
#define OT_SM_CONFIRMED_TIMEOUT_MS      100
#define OT_SM_DEFAULT_BURSTS_TO_IGNORE  1
#define OT_SM_MAX_BURSTS_TO_IGNORE      ((0x1 << MAX_DIP_SWITCHES) - 1)
// @todo - What's the minimum duration for flash triggers?
#define OT_SM_TRIGGER_DURATION_uS       300

/* NOTE: On Canon, we need >75msec to be sure we've completely detected
   pre-flashes. Hence a default PROVISIONAL_TIMEOUT of 100msec is perfect. */
/*==============================================================================
 * MACROS
 *============================================================================*/
/*==============================================================================
 * TYPEDEFs and STRUCTs
 *============================================================================*/
// State Machine Transition/Event Handlers
typedef void (OT_SM_ENTRY_FUNC_T)(void);
typedef void (OT_SM_ACTION_FUNC_T)(OT_SM_EVENT_T);
typedef void (OT_SM_EXIT_FUNC_T)(void);

typedef struct OT_SM_HANDLERS_S {
  OT_SM_ENTRY_FUNC_T  *entryp;
  OT_SM_ACTION_FUNC_T *actionp;
  OT_SM_EXIT_FUNC_T   *exitp;
} OT_SM_HANDLERS_T;

typedef struct OT_SM_DATA_S {
  OT_SM_STATE_T volatile state;
  uint8_t       volatile bursts_to_ignore;
  uint8_t       volatile burst_count;
  uint8_t       volatile provisional_timeout_ms; // User set or default
  uint16_t      volatile state_timeout_ms; // Upto 65.536 sec
} OT_SM_DATA_T;
/*==============================================================================
 * LOCAL FUNCTION PROTOTYPES
 *============================================================================*/
// Core state-machine functions
static void ot_sm_set_state(OT_SM_STATE_T state_in);

// State-machine's entry/action/exit handlers
static OT_SM_ENTRY_FUNC_T  ot_sm_init_entry;
static OT_SM_ACTION_FUNC_T ot_sm_init_action;
static OT_SM_EXIT_FUNC_T   ot_sm_init_exit;
static OT_SM_ENTRY_FUNC_T  ot_sm_ready_entry;
static OT_SM_ACTION_FUNC_T ot_sm_ready_action;
static OT_SM_EXIT_FUNC_T   ot_sm_ready_exit;
static OT_SM_ENTRY_FUNC_T  ot_sm_provisional_entry;
static OT_SM_ACTION_FUNC_T ot_sm_provisional_action;
static OT_SM_EXIT_FUNC_T   ot_sm_provisional_exit;
static OT_SM_ENTRY_FUNC_T  ot_sm_confirmed_entry;
static OT_SM_ACTION_FUNC_T ot_sm_confirmed_action;
static OT_SM_EXIT_FUNC_T   ot_sm_confirmed_exit;
#if defined(WAKEUP_BUTTON)
static OT_SM_ENTRY_FUNC_T  ot_sm_sleeping_entry;
static OT_SM_ACTION_FUNC_T ot_sm_sleeping_action;
static OT_SM_EXIT_FUNC_T   ot_sm_sleeping_exit;
#endif // WAKEUP_BUTTON
/*==============================================================================
 * LOCAL VARIABLES
 *============================================================================*/
// CAUTION: This array is in the order of the OT_SM_STATE_T enumeration
static OT_SM_HANDLERS_T ot_sm_handlers[OT_SM_STATE_MAX] = {
  // OT_SM_STATE_INIT
  {
    &ot_sm_init_entry,
    &ot_sm_init_action,
    &ot_sm_init_exit
  },
  // OT_SM_STATE_READY
  {
    &ot_sm_ready_entry,
    &ot_sm_ready_action,
    &ot_sm_ready_exit
  },
  // OT_SM_STATE_PROVISIONAL
  {
    &ot_sm_provisional_entry,
    &ot_sm_provisional_action,
    &ot_sm_provisional_exit
  },
  // OT_SM_STATE_CONFIRMED
  {
    &ot_sm_confirmed_entry,
    &ot_sm_confirmed_action,
    &ot_sm_confirmed_exit
  }
#if defined(WAKEUP_BUTTON)
  ,
  // OT_SM_STATE_SLEEPING
  {
    &ot_sm_sleeping_entry,
    &ot_sm_sleeping_action,
    &ot_sm_sleeping_exit
  }
#endif // WAKEUP_BUTTON
};

static OT_SM_DATA_T ot_sm_data = {
  .state                  = OT_SM_STATE_MAX, // Invalid deliberately
  .bursts_to_ignore       = OT_SM_DEFAULT_BURSTS_TO_IGNORE,
  .burst_count            = 0,
  .provisional_timeout_ms = OT_SM_PROVISIONAL_TIMEOUT_MS,
  .state_timeout_ms       = 0
};
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
// Note that we allow ourselves to re-enter the current state
static void ot_sm_set_state(OT_SM_STATE_T state_in) {
  if (state_in < OT_SM_STATE_MAX) {
    OT_SM_ENTRY_FUNC_T *entryp;

    // First execute the exit function of the current ot_state, if any
    if (ot_sm_data.state < OT_SM_STATE_MAX) {
      OT_SM_EXIT_FUNC_T  *exitp;
      exitp = ot_sm_handlers[ot_sm_data.state].exitp;
      if ((void*)0 != exitp) (*exitp)();
    }

    // Update our state
    ot_sm_data.state = state_in;

    // Finally execute the entry function of the new ot_state, if any
    entryp = ot_sm_handlers[ot_sm_data.state].entryp;
    if ((void*)0 != entryp) (*entryp)();
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
static void ot_sm_init_entry(void) {
  ot_sm_data.bursts_to_ignore = OT_GPIO_bursts_to_ignore();

  // Check if we should use the DELAY_SENSE analog value to determine the
  // timeout to trigger the flash.
  if (OT_SM_MAX_BURSTS_TO_IGNORE == ot_sm_data.bursts_to_ignore) {
    ot_sm_data.provisional_timeout_ms = OT_ADC_read_delay_sense();
    // Ensure that provisional_timeout_ms is non-zero (precondition for the
    // implementation in the PROVISIONAL state)
    if (0 == ot_sm_data.provisional_timeout_ms) {
      ot_sm_data.provisional_timeout_ms = OT_SM_PROVISIONAL_TIMEOUT_MS;
    }
  }
  else { // Use default value
    ot_sm_data.provisional_timeout_ms = OT_SM_PROVISIONAL_TIMEOUT_MS;
  }

  GREEN_LED_ON(); // Turn ON GREEN LED to show we're starting
  ot_sm_data.state_timeout_ms = OT_SM_INIT_TIMEOUT_MS;
  OT_TIMER_start(); // sends TIMEOUT events every ~1msec
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
static void ot_sm_init_action(OT_SM_EVENT_T event) {
  if (OT_SM_EVENT_TIMEOUT == event) {
    if (0 == --ot_sm_data.state_timeout_ms) {
      ot_sm_set_state(OT_SM_STATE_READY);
    }
  }
  // Ignore all other events and stay in the same state
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
static void ot_sm_init_exit(void) {
  // cancel/stop state timer
  OT_TIMER_stop();
  ot_sm_data.state_timeout_ms = 0;
  GREEN_LED_OFF(); // Turn off GREEN LED to indicate we're moving to READY
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
static void ot_sm_ready_entry(void) {
  ot_sm_data.burst_count = 0; // Reset our internal counters
#if defined(WAKEUP_BUTTON)
  // Set a timer to enter sleep if there is no flash/user activity
  ot_sm_data.state_timeout_ms = OT_SM_READY_TIMEOUT_MS;
  OT_TIMER_start(); // sends TIMEOUT events every ~1msec  return;
  // Enable the Button Interrupt (in case user checks to see if we are awake or
  // requests us to re-read settings)
  BUTTON_ENABLE();
#endif // WAKEUP_BUTTON
  TRIGGER_IN_ENABLE(); // Enable Flash burst interrupt
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
static void ot_sm_ready_action(OT_SM_EVENT_T event) {
  if (OT_SM_EVENT_FLASH_DETECTED == event) {
    ++ot_sm_data.burst_count;
    // If we've exceeded the number of bursts to ignore we are done here
    // (which happens when bursts_to_ignore is 0)
    if (0 == ot_sm_data.bursts_to_ignore) {
      ot_sm_set_state(OT_SM_STATE_CONFIRMED);
    }
    else {
      // Go to PROVISIONAL; it will set the appropriate timeout
      ot_sm_set_state(OT_SM_STATE_PROVISIONAL);
    }
  }
#if defined(WAKEUP_BUTTON)
  else if (OT_SM_EVENT_TIMEOUT == event) {
    if (0 == --ot_sm_data.state_timeout_ms) { // Waiting period has expired
      // We waited long enough for flash/user action
      ot_sm_set_state(OT_SM_STATE_SLEEPING);
    }
  }
  else if (OT_SM_EVENT_BUTTON_PRESS == event) {
    // User checking if we are awake (or requesting us to re-read settings).
    // Go back to INIT to show the GREEN LED.
    ot_sm_set_state(OT_SM_STATE_INIT);
  }
#endif // WAKEUP_BUTTON
  // Ignore all other events and stay in the same state
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
static void ot_sm_ready_exit(void) {
  TRIGGER_IN_DISABLE(); // Disable Flash burst interrupt
#if defined(WAKEUP_BUTTON)
  BUTTON_DISABLE();
  // cancel/stop state timer
  OT_TIMER_stop();
  ot_sm_data.state_timeout_ms = 0;
#endif // WAKEUP_BUTTON
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
static void ot_sm_provisional_entry(void) {
  // If we entered this state we just detected ONE flash burst
  // The provisional_timeout_ms has been assigned the right value in INIT state
  ot_sm_data.state_timeout_ms = ot_sm_data.provisional_timeout_ms;
  OT_TIMER_start(); // sends TIMEOUT events every ~1msec
  TRIGGER_IN_ENABLE(); // Enable Flash burst interrupt
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
static void ot_sm_provisional_action(OT_SM_EVENT_T event) {
  if (OT_SM_EVENT_FLASH_DETECTED == event) {
    // Possibly part of the 'red eye' reduction or 'pre-flashes'
    // Increment our count of flash bursts detected
    ++ot_sm_data.burst_count;
    // If we've exceeded the number of bursts to ignore we are done here
    if ((ot_sm_data.bursts_to_ignore < OT_SM_MAX_BURSTS_TO_IGNORE) &&
        (ot_sm_data.burst_count > ot_sm_data.bursts_to_ignore)) {
      ot_sm_set_state(OT_SM_STATE_CONFIRMED);
    }
    else {
      // Whether the user has configured a timeout or not, the correct value
      // has ben assigned to provisional_timeout_ms in INIT state.
      // reset our state timer
      ot_sm_data.state_timeout_ms = ot_sm_data.provisional_timeout_ms;
      OT_TIMER_start(); // sends TIMEOUT events every ~1msec
      // stay in this state
    }
  }
  else if (OT_SM_EVENT_TIMEOUT == event) {
    if (0 == --ot_sm_data.state_timeout_ms) { // Waiting period has expired
      // No more 'red eye' or 'preflashes' are incoming.
      // Go to CONFIRMED if user had configured a timeout
      // Otherwise go to INIT as the right number of preflashes didn't arrive.
      if (OT_SM_MAX_BURSTS_TO_IGNORE == ot_sm_data.bursts_to_ignore) {
          ot_sm_set_state(OT_SM_STATE_CONFIRMED);
      }
      else {
          ot_sm_set_state(OT_SM_STATE_INIT);
      }
    }
  }
  // Ignore all other events and stay in the same state
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
static void ot_sm_provisional_exit(void) {
  TRIGGER_IN_DISABLE(); // Disable Flash burst interrupt
  // cancel/stop state timer
  OT_TIMER_stop();
  ot_sm_data.state_timeout_ms = 0;
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
static void ot_sm_confirmed_entry(void) {
  TRIGGER_OUT_ON(); // Trigger the slave flash
  OT_TIMER_busywait_us(OT_SM_TRIGGER_DURATION_uS);
  TRIGGER_OUT_OFF(); // Release the trigger

  RED_LED_ON(); // Signal that we triggered
  // set a state timer to turn off the RED LED
  ot_sm_data.state_timeout_ms = OT_SM_CONFIRMED_TIMEOUT_MS;
  OT_TIMER_start();
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
static void ot_sm_confirmed_action(OT_SM_EVENT_T event) {
  if (OT_SM_EVENT_TIMEOUT == event) {
    if (0 == --ot_sm_data.state_timeout_ms) { // Waiting period has expired
      ot_sm_set_state(OT_SM_STATE_INIT);
    }
  }
  // Ignore all other events and stay in the same state
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
static void ot_sm_confirmed_exit(void) {
  OT_TIMER_stop();
  ot_sm_data.state_timeout_ms = 0;
  RED_LED_OFF();
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
#if defined(WAKEUP_BUTTON)
static void ot_sm_sleeping_entry(void) {
  DIP_DISABLE(); // Remove pull-ups from the DIP switches
  SENSOR_OFF(); // Power down the Flash burst sensor
  BUTTON_ENABLE(); // Enable the Button Interrupt
  return;
}
#endif // WAKEUP_BUTTON
/*==============================================================================
 * DESCRIPTION:
 * @param
 * @return
 * @precondition
 * @postcondition
 * @caution
 * @notes
 *============================================================================*/
#if defined(WAKEUP_BUTTON)
static void ot_sm_sleeping_action(OT_SM_EVENT_T event) {
  if (OT_SM_EVENT_BUTTON_PRESS == event) {
    // User pressed button to wake us up
    ot_sm_set_state(OT_SM_STATE_INIT);
  }
  return;
}
#endif // WAKEUP_BUTTON
/*==============================================================================
 * DESCRIPTION:
 * @param
 * @return
 * @precondition
 * @postcondition
 * @caution
 * @notes
 *============================================================================*/
#if defined(WAKEUP_BUTTON)
static void ot_sm_sleeping_exit(void) {
  BUTTON_DISABLE(); // Disable the Button Interrupt
  SENSOR_ON(); // Power on the Flash burst sensor
  DIP_ENABLE(); // Add pull-ups to the DIP switches
  return;
}
#endif // WAKEUP_BUTTON
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
void OT_SM_init(void) {
  ot_sm_set_state(OT_SM_STATE_INIT);
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
void OT_SM_execute(OT_SM_EVENT_T event) {
  if (event < OT_SM_EVENT_MAX && ot_sm_data.state < OT_SM_STATE_MAX) {
    OT_SM_ACTION_FUNC_T *actionp;
    actionp = ot_sm_handlers[ot_sm_data.state].actionp;
    if ((void*)0 != actionp) (*actionp)(event);
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
OT_SM_STATE_T OT_SM_get_state(void) {
  return ot_sm_data.state;
}
/*============================================================================*/

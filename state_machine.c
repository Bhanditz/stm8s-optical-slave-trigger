/*==============================================================================
 * MODULE: State Machine (SM)
 * DESCRIPTION: Implementation of the SM module
 *============================================================================*/
/*==============================================================================
 * INCLUDES
 *============================================================================*/
#include "config.h"
#include "timer.h"
#include "state_machine.h"
/*==============================================================================
 * CONSTANTS
 *============================================================================*/
#define OT_SM_INIT_TIMEOUT_MS           1000 // #if defined(TIMER_DEBUG)
#define OT_SM_READY_TIMEOUT_MS          60000 // #if defined(WAKEUP_BUTTON)
#define OT_SM_PROVISIONAL_TIMEOUT_MS    100
#define OT_SM_CONFIRMED_TIMEOUT_MS      10 // @todo - What's the minimum duration for flash triggers?
#define OT_SM_NUM_BURSTS_TO_IGNORE      1 // @todo - use external DIP switch

/* NOTE: On Canon, we need >75msec to be sure we've completely detected
   pre-flashes. Hence a PROVISIONAL_TIMEOUT of 100msec is perfect when
   the feature IGNORE_PREFLASH is not defined */
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
  uint8_t       volatile burst_count;
  uint16_t      volatile timeout_ms; // Upto 65.536 sec
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
  .state       = OT_SM_STATE_MAX,    // Invalid deliberately
  .burst_count = 0,
  .timeout_ms  = 0
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
#if defined(TIMER_DEBUG)
  ot_sm_data.timeout_ms = OT_SM_INIT_TIMEOUT_MS;
  OT_TIMER_start(); // sends TIMEOUT events every ~1msec
#endif // TIMER_DEBUG
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
#if defined(TIMER_DEBUG)
  if (OT_SM_EVENT_TIMEOUT == event) {
    if (0 == --ot_sm_data.timeout_ms) {
      ot_sm_set_state(OT_SM_STATE_READY);
    }
  }
#else
  // Wait for the main module to send us an INIT_COMPLETE
  if (OT_SM_EVENT_INIT_COMPLETE == event) {
    ot_sm_set_state(OT_SM_STATE_READY);
  }
#endif // TIMER_DEBUG
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
#if defined(TIMER_DEBUG)
  // cancel/stop state timer
  OT_TIMER_stop();
  ot_sm_data.timeout_ms = 0;
#endif // TIMER_DEBUG
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
  GREEN_LED_ON(); // Turn ON GREEN LED to show we're in READY
  ot_sm_data.burst_count = 0; // Reset our internal counters
#if defined(WAKEUP_BUTTON)
  // Set a timer to enter sleep if there is no user activity
  ot_sm_data.timeout_ms = OT_SM_READY_TIMEOUT_MS;
  OT_TIMER_start(); // sends TIMEOUT events every ~1msec  return;
#endif // WAKEUP_BUTTON
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
#if defined(IGNORE_PREFLASH)
    // If we've exceeded the number of bursts to ignore we are done here
    // @todo - get NUM_BURSTS_TO_IGNORE from external DIP switch
    if (ot_sm_data.burst_count > OT_SM_NUM_BURSTS_TO_IGNORE) {
      ot_sm_set_state(OT_SM_STATE_CONFIRMED);
    }
    else
#endif // IGNORE_PREFLASH
      { ot_sm_set_state(OT_SM_STATE_PROVISIONAL); }
  }
#if defined(WAKEUP_BUTTON)
  else if (OT_SM_EVENT_TIMEOUT == event) {
    // decrement out timeout_ms count
    if (0 == --ot_sm_data.timeout_ms) { // Waiting period has expired
      // We waited long enough for user action
      ot_sm_set_state(OT_SM_STATE_SLEEPING);
    }
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
#if defined(WAKEUP_BUTTON)
  // cancel/stop state timer
  OT_TIMER_stop();
  ot_sm_data.timeout_ms = 0;
#endif // WAKEUP_BUTTON
  GREEN_LED_OFF(); // Turn OFF GREEN LED to show we've exited READY
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
  ot_sm_data.timeout_ms = OT_SM_PROVISIONAL_TIMEOUT_MS;
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
static void ot_sm_provisional_action(OT_SM_EVENT_T event) {
  if (OT_SM_EVENT_FLASH_DETECTED == event) {
    // Possibly part of the 'red eye' reduction or 'pre-flashes'
    // Increment our count of flash bursts detected
    ++ot_sm_data.burst_count;
#if defined(IGNORE_PREFLASH)
    // If we've exceeded the number of bursts to ignore we are done here
    // @todo - get NUM_BURSTS_TO_IGNORE from external DIP switch
    if (ot_sm_data.burst_count > OT_SM_NUM_BURSTS_TO_IGNORE) {
      ot_sm_set_state(OT_SM_STATE_CONFIRMED);
    }
    else {
#endif // IGNORE_PREFLASH
      // reset our state timer
      ot_sm_data.timeout_ms = OT_SM_PROVISIONAL_TIMEOUT_MS;
      OT_TIMER_start(); // sends TIMEOUT events every ~1msec
      // stay in this state
#if defined(IGNORE_PREFLASH)
    }
#endif // IGNORE_PREFLASH
  }
  else if (OT_SM_EVENT_TIMEOUT == event) {
    // decrement out timeout_ms count
    if (0 == --ot_sm_data.timeout_ms) { // Waiting period has expired
      // We waited long enough after the last burst.
      // No more 'red eye' or 'preflashes' are incoming.
#if defined(IGNORE_PREFLASH)
      // If we timed out waiting for pre-flashes something went wrong
      // go back to READY
      ot_sm_set_state(OT_SM_STATE_READY);
      // @todo - ideally should go to ARMING but that's not used in our
      //         current design
#else
      ot_sm_set_state(OT_SM_STATE_CONFIRMED);
#endif // IGNORE_PREFLASH
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
  // cancel/stop state timer
  OT_TIMER_stop();
  ot_sm_data.timeout_ms = 0;
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
#if defined(DEBUG)
  // Flash RED LED for burst_count times
  uint8_t i;
  for (i=0; i < ot_sm_data.burst_count; ++i) {
#define OT_SM_BUSYWAIT_DELAY_MS  500
    RED_LED_ON(); OT_TIMER_busywait_ms(OT_SM_BUSYWAIT_DELAY_MS);
    RED_LED_OFF(); OT_TIMER_busywait_ms(OT_SM_BUSYWAIT_DELAY_MS);
  }
#else
  TRIGGER_OUT_ON(); // Trigger the slave flash
#endif // DEBUG
  // set a 10msec timer
  ot_sm_data.timeout_ms = OT_SM_CONFIRMED_TIMEOUT_MS;
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
    if (0 == --ot_sm_data.timeout_ms) { // Waiting period has expired
      // We waited long enough for the slave flash to have fired.
      ot_sm_set_state(OT_SM_STATE_READY);
      // @todo - ideally should go to ARMING but that's not used in our
      //         current design
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
  ot_sm_data.timeout_ms = 0;
#if !defined(DEBUG)
  TRIGGER_OUT_OFF(); // Release the slave flash trigger
#endif // DEBUG
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
  // Enable the Button Interrupt
  BUTTON_ENABLE();
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
    ot_sm_set_state(OT_SM_STATE_READY);
    // @todo - ideally should go to ARMING but that's not used in our
    //         current design
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
  // Disable the Button Interrupt
  BUTTON_DISABLE();
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

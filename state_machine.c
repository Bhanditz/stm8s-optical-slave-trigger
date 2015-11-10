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
#define OT_SM_INIT_TIMEOUT_MS           1000 // Used when TIMER_DEBUG is defined
#define OT_SM_PROVISIONAL_TIMEOUT_MS    100
#define OT_SM_CONFIRMED_TIMEOUT_MS      10
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
  OT_SM_STATE_T volatile ot_sm_state;
  uint8_t       volatile burst_count;
  uint16_t      volatile timeout_ms;
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
};

static OT_SM_DATA_T ot_sm_data = {
  .ot_sm_state = OT_SM_STATE_INIT,
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
static void ot_sm_set_state(OT_SM_STATE_T state_in) {
  if (state_in < OT_SM_STATE_MAX && ot_sm_data.ot_sm_state < OT_SM_STATE_MAX) {
    OT_SM_ENTRY_FUNC_T *entryp;
    OT_SM_EXIT_FUNC_T  *exitp;

    // Note that we allow ourselves to re-enter the current state

    // First execute the exit function of the current ot_state, if any
    entryp = ot_sm_handlers[ot_sm_data.ot_sm_state].entryp;
    if ((void*)0 != entryp) (*entryp)();

    // Update our state
    ot_sm_data.ot_sm_state = state_in;

    // Finally execute the entry function of the new ot_state, if any
    exitp = ot_sm_handlers[ot_sm_data.ot_sm_state].exitp;
    if ((void*)0 != exitp) (*exitp)();
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
    ot_sm_set_state(OT_SM_STATE_PROVISIONAL);
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
static void ot_sm_ready_exit(void) {
  GREEN_LED_OFF(); // Turn OFF GREEN LED to show we've exited READY
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
  ot_sm_data.burst_count = 1;
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
    // reset our state timer
    ot_sm_data.timeout_ms = OT_SM_PROVISIONAL_TIMEOUT_MS;
    OT_TIMER_start(); // sends TIMEOUT events every ~1msec
    // stay in this state
  }
  else if (OT_SM_EVENT_TIMEOUT == event) {
    // decrement out timeout_ms count
    if (0 == --ot_sm_data.timeout_ms) { // Waiting period has expired
      // We waited long enough after the last burst.
      // No more 'red eye' or 'preflashes' are incoming.
      ot_sm_set_state(OT_SM_STATE_CONFIRMED);
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
  // @todo - flash RED LED for burst_count times
#else
  TRIGGER_OUT_ON(); // Trigger the slave flash
#endif
  // set a 10msec timer
  ot_sm_data.timeout_ms = OT_SM_CONFIRMED_TIMEOUT_MS;
  OT_TIMER_start();
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
  TRIGGER_OUT_OFF(); // Release the slave flash trigger
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
  if (event < OT_SM_EVENT_MAX && ot_sm_data.ot_sm_state < OT_SM_STATE_MAX) {
    OT_SM_ACTION_FUNC_T *actionp;
    actionp = ot_sm_handlers[ot_sm_data.ot_sm_state].actionp;
    if ((void*)0 != actionp) (*actionp)(event);
  }
  return;
}
/*============================================================================*/

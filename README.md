# stm8s-optical-slave-trigger
Optical Slave Trigger for Off Camera Flashes using the STM8S Discovery Board. Discrete components detect a 'master' flash going off and interrupt the STM8S MCU. The MCU then drives a 'slave' flash (via an optocoupler). See the doc/OpticalSlaveTriggerSchematic.png for component details.

## doc/OpticalSlaveTriggerSchematic.png:
- Original schematic intended for used with a ATTiny processor. However, this project uses the STM8S MCU. Schematic should be updated in due course.

## Functional Description
- Upon power-up or after wake-up from power-save (i.e. deep-sleep) mode, a sign-of-life indication (GREEN LED for 100msec) is displayed when the trigger is ready to detect flash bursts.
- The trigger enters power-save mode after 60sec of inactivity (i.e. neither flash bursts nor button presses detected).
- In power-save (deep-sleep) mode, pressing the button wakes-up the trigger.
- In non power-save mode, pressing the button just displays the sign-of-life indication and causes the trigger to re-read the user settings (see below regarding DIP switches).
- DIP[2:0] switch settings, interpreted as a binary number, determine how many pre-flashes (or red-eye bursts) to ignore before triggering the slave flash.
  - Valid values are 000b..110b.
  - By default the DIP GPIOs are 'high'. Turning 'ON' a switch sets the corresponding GPIO 'low'.
  - In this mode, the count of flash bursts detected is reset to 0 if more than 100msec passes since the last detected flash burst.
- DIP[2:0] when set to 111b (i.e. all switches 'OFF') is interpreted as 'infinite number of pre-flashes to ignore'.
  - In this mode, the relative voltage of the DELAY_SENSE signal (compared to Vdd) scaled to the range 0..255 determines the time (in msec) to wait since detecting the last flash burst and before triggering the slave flash.
    - If the relative voltage is 0, then the time (in msec) to wait automatically defaults to 100msec.
- Whenever the slave flash is triggered, a trigger indication (RED LED for 100msec) is displayed.

## User Experience
- User inserts the battery and powers-up the unit. Green LED flashes briefly and trigger reads the user settings.
- User configures the DIP[2:0] switches to a number between 0-6. User then presses the button to ensure that the setting is updated in the trigger. Green LED flashes briefly and trigger (re)updates settings.
- User employs the trigger as an optical slave for their main off-camera flash. Trigger ignores pre-flash bursts according to the number configured by the DIP[2:0] switches and triggers the slave flash on the next flash burst.
  - Each time the slave is triggered the Red LED flashes briefly. The trigger then flashes Green LED briefly and (re)updates the settings.
- User waits for more than 60 sec. The trigger enters power-save (deep-sleep) mode.
- User presses button to wake up the trigger from power-save mode. Green LED flashes briefly and trigger (re)updates the settings.
- User waits for less than 60 sec. Then presses button to ensure trigger is still awake. Green LED flases briefly and trigger (re)updates settings.
- User configures the DIP[2:0] switches to the max value (7) and then sets the trimmer (or potentiometer) to the required timeout value (between 0-255msec). User then presses the button to ensure that the settings are updated in the trigger. Green LED flashes briefly and trigger (re)updates settings.
- User employs the trigger as an optical slave for their main off-camera flash. Trigger resets the timer for each flash burst detected. If the timer expires (based on the timeout configured) it then triggers the slave flash.
  - Each time the slave is triggered the Red LED flashes briefly. The trigger then flashes Green LED briefly and (re)updates the settings.

## doc/OpticalSlaveStateMachine.png:
- Initial outline of the State Machine to implement on the STM8S.

## pinout.md
- Port/Pin assignments for different targets
- Currently the only supported target is the STM8S-DISCOVERY board (used for prototyping).

# stm8s-optical-slave-trigger
Optical Slave Trigger for Off Camera Flashes using the STM8S Discovery Board.
Discrete components detect a 'master' flash going off and interrupt the STM8S
MCU. The MCU then drives a 'slave' flash (via an optocoupler). See the 
OpticalSlaveTriggerSchematic.png for component details.

doc/OpticalSlaveTriggerSchematic.png:
- Original schematic intended for used with a ATTiny processor. However, this
project uses the STM8S MCU. Schematic should be updated in due course.

doc/OpticalSlaveStateMachine.png:
- Initial outline of the State Machine to implement on the STM8S.

pinout.txt - Port/Pin assignments for different targets
- Currently only supported target is the STM8S-DISCOVERY board (used for prototyping)

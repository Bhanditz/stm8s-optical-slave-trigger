# Pinout map

## STM8S903K3 (SDIP)

### Power
| Portx | Signal        | Pin #  |
|-------|---------------|--------|
| N/A   | VDD           | Pin 11 |
| N/A   | VSS           | Pin 09 |


### PortA
| Portx | Signal        | Pin #  |
|-------|---------------|--------|
| PA1   | OSCIN         | Pin 07 |
| PA2   | OSCOUT        | Pin 08 |
| PA3   |               | Pin 12 |

### PortB Input Sensitivity Rise-only
| Portx | Signal             | Pin #  |
|-------|--------------------|--------|
| PB0   | DELAY_SENSE (AIN0) | Pin 21 |
| PB1   | DIP0               | Pin 20 |
| PB2   | DIP1               | Pin 19 |
| PB3   | DIP2               | Pin 18 |
| PB4   |                    | Pin 17 |
| PB5   |                    | Pin 16 |
| PB6   | TRIGGER_IN         | Pin 15 |
| PB7   |                    | Pin 14 |

### PortC Input Sensitivity Fall-only
| Portx | Signal        | Pin #  |
|-------|---------------|--------|
| PC1   | BUTTON_DET    | Pin 23 |
| PC2   |               | Pin 24 |
| PC3   |               | Pin 25 |
| PC4   |               | Pin 26 |
| PC5   |               | Pin 27 |
| PC6   |               | Pin 28 |
| PC7   |               | Pin 29 |

### PortD
| Portx | Signal        | Pin #  |
|-------|---------------|--------|
| PD0   | GREEN_LED     | Pin 30 |
| PD1   | SWIM          | Pin 31 |
| PD2   | RED_LED       | Pin 32 |
| PD3   | SENSOR_ENABLE | Pin 01 |
| PD4   | TRIGGER_OUT   | Pin 02 | (Potentially move to PA3?)
| PD5   |               | Pin 03 |
| PD6   |               | Pin 04 |
| PD7   |               | Pin 05 |

### PortE
| Portx | Signal        | Pin #  |
|-------|---------------|--------|
| PE5   |               | Pin 22 |

### PortF (No Interrupt Capability)
| Portx | Signal        | Pin #  |
|-------|---------------|--------|
| PF4   |               | Pin 13 |
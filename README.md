# Power Measurement of Consumption
This project counts how many times the LED blinks within one hour.  
Each pulse from the power meter represents 1 Wh (1 watt-hour) of energy consumption.
## MCU
STM32 NUCLEO-L152RE  
Bare-metal development using CMSIS
## Components
- Light sensor: NSL-19M51  
- Resistor: 10kΩ  
- Connection: Serial configuration  
- Signal between components connected to PA0 (A0)
## Peripherals Used
- Timer5 (32-bit) – for time measurement and pulse counting  
- ADC – input from PA0 (A0)  
- GPIO output – PA10 (D2)  
- External interrupt – PB5 (D4)
- D2 is connected to D4.
## Functionality
The program measures the time (in seconds) when the LED is ON and counts how many pulses occur within one hour.
The system can accurately measure consumption from 0 to 3600 Wh.  
Measurements above 3600 Wh may no longer be accurate due to system limitations.

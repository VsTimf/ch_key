## ChibiOs KEY Library 

**This is a simple library for handling button presses, written in C for ChibiOs-compatible projects**



### MAIN FEATURES:
- Processing of up to 16 buttons with selection of active voltage level
- Supports 5 events: 
   - CLICK
   - HOLD
   - HOLD-REPEAT
   - RELEASE
   - HOLD DURING POWER UP
- Sends broadcast messages via the ChibiOs "event" mechanism
- Allows you to subscribe to specific buttons and click types from multiple threads



### HOW TO USE:

See [`ch_key_config.h`](./inc/ch_key_config.h) for "HOW TO USE" instructions.
  
&nbsp;
> [!WARNING]
> **The library tested only on stm32f4xx mcu's**
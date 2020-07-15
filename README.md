# SX126x Atmel SAM library
 A library to use the LoRa modules SX1261 and SX1262 on SAM microcontrollers by Atmel.

 The library itself is completely device independant, you should adapt your own functions in the `device specific implementation` file.

 The other two files contains:

    * sx126x_hal: write/read for commands, registers and buffer
    * sx126x_commands: all the commands present in library released by the manufacture.

The repo also includes a demo running on a Metro Gran Central board featuring a SAMD51 Cortex M4 processor.

Please note that the device speicif functions and the hal functions have been all tested, while not all commands have been tested. I try and did my best to provide a fully working library, but I take no responsability for errors and bugs that might be present.

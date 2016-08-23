# syringe_pump
An alternative implementation of the OpenSyringe Pump program.

This program drives a DIY syringe pump based heavily on that from the [OpenSyringe Pump][1].
Using the keypad, the flow rate can be set in mL/min and the rail driven back or forth manually.
The linear rail in this implementation is an assembled 300mm actuator kit from [MakerStore][2]

Updates to the program include:

 - The ADC millivolt readings LCD shield have been fixed
 - A timer interrupt is used to update the LCD
 - The number of required 3D printed parts has been reduced


[1]: https://hackaday.io/project/1838-open-syringe-pump
[2]: http://www.makerstore.com.au/product/300mm-actuator-kit/


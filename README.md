# syringe_pump
An alternative implementation of the OpenSyringe Pump program.

This program drives a DIY syringe pump based heavily on that from the [OpenSyringe Pump][1].
The user can set the flow rate in mL/min and then drive the pump back or forth manually.
The syringe pump rail in this implementation is an assembled 300mm actuator kit from [MakerStore][2]

Updates to the program include:

 - The ADC millivolt readings LCD shield have been fixed
 - A timer interrupt is used to update the LCD
 - The number of required 3D printed parts are reduced

[1]: https://hackaday.io/project/1838-open-syringe-pump
[2]: http://www.makerstore.com.au/product/300mm-actuator-kit/


# IR-Base
    This is the production version that does not use the 256kb chip.  IR-Base256 will use the chip.
    Ports 18 & 19 are required and available to use the chip.
   
    The circuit defined here will work with or without the chip installed. Recommend running this 
    script if the 256lb chip is not going to be used.

    This version also implements Radio support (RF24) as the base station of the IR Remote solution. 
    Its designed to be in proximity of the device to be controlled vi IR commands.

Running Modes:
    Base only using on board switches or wired switches
    IR-Sender using a disconnected sender.

    The base module will automaticly switch between the onboard
    LED sender and the wired LED sender that can be attaced to the TV.

   Currently is running on an Arduino project board in conjunction with
   IR-Sender
## Switch Fightstick Code (Atmega32U4)

This one is a working switch fightstick sketch
flashable via Arduino IDE. I personally use it on a pro micro clone microcontroller that can be bought for 2-3$ on aliexpress. My motivation was to have a working solution for dirt-cheap easy source-able parts. Well here it is. 

The Code itself has proper Button-Debounce, and 3 Modes of input.

You can switch seamlessly between the 3 modes by pressing START+SELECT. 

- Digital Only
- Fake Analog (Digital Movement is mapped to L-Analog)
- Smash Ultimate Mode (Most-left button on the second row of a fight stick is used as a modifier key to decrease the range of the Analog stick, if held)

## Building Instructions

- Download Arduino IDE, 

- Download the Bounce2 Library inside the Arduino IDE
- Download Arduino Lufa from https://github.com/Palatis/Arduino-Lufa and follow the instructions on it's repository
- Build and Flash for your ATMEGA32U4 Board
- Have Fun

### Credits

Special thanks to shinyquagsire's and progmem's reverseengineering work for the pokken tournament controller. (https://github.com/progmem/Switch-Fightstick)

Dean Camera for the LUFA Library

zlittell, msf-xinput was very helpful for starting.

thomasfredericks for the Bounce2Lib.

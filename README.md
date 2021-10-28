# FallOHelper
Full FallOHelper code. The device is intended to be worn in a case attached to a belt.
This project's goal is to create a device that will help elderly people when they fall down at home. 

This device automatically detects if the person wearing it falls down by making use of the gyrosensor and the accelerometer built in. When it detects it, the SIM card
module calls the phone number stored in the SD card (maybe a family member) and thus it notifies them of the fall. There is also an indicator LED that tells
the person wearing it the status of the device (on=calibrating; blinking = detecting a fall; blinking fast = fall detected, calling; off=device off/device on).

The device also has 2 buttons. The first button resets the progress of the fall detection in case it starts detecting by accident. If the reset button
is held down for 5s, the gyrosensor and accelerometer recalibrate. The second button sends the help signal and the SIM card calls the family member directly
without fall detection.

#Hitchhiker Laboratories Abattoir
##About
At its core, the abattoir is a vactrol based VCA with an attack and release envelope generator. In addition to the Attack and Release controls that set the envelope, there’s also a Mod control that affects both parameters at once. When Mod is turned fully counter-clockwise, the values of the Attack and Release knobs are used, but turning the Mod knob clockwise will gradually reduce the length of both the attack and release settings.

Pushing the purple button triggers the attack and releasing it triggers the release (left status LED light purple when the envelope is triggered). This can be used in one of two setting determined by the topmost toggle switch: 1) make (left) a sound or 2) break a sound. In the first setting, the audio sent to the pedal’s input is silenced unless the purple button is pressed, but in the second setting, the input audio is heard unless the button is pressed. In both cases, the envelope settings are used to control the vactrol VCA to make or break the sound. The abattoir’s envelope generator and vactrol VCA make it differ from other kill switch in that it can kill very smoothly. If you like the clicks, look elsewhere… the abattoir will never cause clicks of any kind.

The abattoir also allows the recording and playback of trigger sequences played on the purple button. This is the mechanically perfect repetitive killing action that gives the abattoir its name. When the right status LED is off, pressing the orange button once will arm the recording (LED turns red). The sequence will begin recording with the next press of the purple button and will end with the next press of the orange button. Playback of the sequence will start immediately after this second press of the orange button (LED turns green). Pressing the orange button during playback will pause the playback of the sequence (LED turns orange) and pressing it again will restart the sequence. A long press of the orange button at any time will clear the recorded sequence (LED turns off).

While a sequence is playing, the envelope settings can be manipulated to vary playback, but the abattoir also has a Speed control that makes it possible to vary the playback speed of the recorded sequence. Also, the break/make toggle switch can be used to play the “opposite” of what you recorded (the ground to your figure).

The bottom toggle switch is used to bypass the effect (left=off).

Video demonstration: https://youtu.be/3eOvrJEJpls

Abattoir can also be used to modulate a control voltage source to control pedals that accept CV (usually 5V). See this video for a demonstration CV modulation with the abattoir: https://youtu.be/8e8jrdDAC2Y

##Implementation
###Code
The Abattoir is implemented with Arduino and requires the metro library. Buttons are not debounced in software (see debouncing below).

Code: abattoir.ino

###Hardware
Arduino Pro Mini is the ideal platform for small guitar pedals.

Arcade buttons are ideal for quick pattern playing with either hands or feet.

Schematic shows two VCADSR7b envelope generators implemented by Electric Druid with PIC16F684: http://electricdruid.net/voltage-controlled-adsr-envelope-generator-vc-adsr-7b/ 

Suggested enclosure: http://www.hammondmfg.com/pdf/1590BB.pdf

Enclosure template: enclosure.pdf

Schematic: schematic.pdf

###Debouncing
Schematic shows debouncer implemented by Murat Uzam with PIC16F628: http://www.meliksah.edu.tr/muzam/ew_2006.zip

Feel free to use whatever debouncing strategy you like best, but be aware that the arduino code as written does not debounce the buttons at all.

##Licence
Abattoir by Sylvain Poitras is licensed under the GNU General Public License v.3: https://www.gnu.org/copyleft/gpl.html

##Thanks
Looping engine code adapted from Glenn Mossy's sequence recorder (Creative Commons CC-BY-SA) at http://littlebits.cc/projects/sequence-recorder

Interrupt setup code from: http://playground.arduino.cc/Main/PinChangeInterrupt

# topcase\_E470\_leds

Motorcycle Top Box Brake And Turn Signal Lights
V1.0  2025 Author: John Zonneveld

This sketch will run LED sequences that will add both brake lights and turn signals to my Kawasaki branded Givi
E470 Monolock top box. The only difference between the Kawa box and the Givi box is the lid panel. The Givi lid
panel already includes a lens for a center mounted light assembly. Because of this there is some spare space under
the panel lid in the Kawa box. in that spare space I will mount my arduino contraption which includes a buck converter
and an Arduino Nano.
In total I will mount 6 LED strips of various lengths of WS2812B with 60 LEDs/m, three on each side with 9, 8 and 6 LEDs.
The total mounted LEDs equals 2x9 + 2x8 + 2x6 = 46 LEDs. On white, with full brightness this would total to about 46 x
60mA = 2.76A. There is an accessoiry lead in the tail of the bike which is fused 5A, only problem is that it shares the
fuse with the accesoiry lead in the fairing that is in use for my Garmin. (will have to check about the cig lighter on
the right, if that is using the same fuse)
Also it doesn't really make sense to create white light when the additional colors are being filtered away by the lens,
because of this I will only use the Red section of the Neopixels. This would also reduce the power draw to about 46 x
20mA = 0.92A for the LEDs. The Arduino Nano takes about 40mA in worst case, but total current
draw would be at most 1.25A continous while braking.

![img](e470.PNG)

The connection to the strips will be on the outside left of the strips on the left and outside right for the strips on
the right.
The factory kit will only do brake lights, Admore is selling a kit that would do running/brake lights and turning signals
with a strip of 9 and 6 LEDs on each side.
This setup will provide:

* Running lights (dimmed, only outside LEDs)
* Brake light (both sides on, warning flash at start. All LEDs)
* Turn signal (one side all LEDs sequential flashing, other side running lights unless braking then other side is solid on
  (with start flash).)
* Hazard light (both sides sequential flashing from inside to outside)

This can obviously be customized as needed.  To convert the 12V signals to the Arduino Inputs 5V signal I used optocouplers.
This setup uses an Arduino Nano and a 12VDC buck converter to obtain a 5V power source to pwer the Arduino and the LED strips.
Because of this setup I can not use the 2 wire kit that Givi sells and uses contacts for easy removal of the box.
For this setup we need to bring 5 wires to the box:

* 12V
* Ground
* Left turn signal
* Right turnsignal
* Brake light
  I bought some water proof connecters but also a 5 pin trailer wiring harness. which I probably hide somewhere under the seat.

Timings used in this sketch still have to be adapted to the bike escpecially the times used in the turn signals to fill the
strip up and the pulse monitoring before dropping the turn signal

This will be the view when system is in rest (no inputs from signals or brake)

![img](e470_runninglights.PNG)



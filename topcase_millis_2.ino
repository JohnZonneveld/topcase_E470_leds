/*
Motorcycle Top Box Brake And Turn Signal Lights
V1.0  2025 Author: John Zonneveld
This sketch will run LED sequences that will add both brake lights and turn signals to my Kawasaki branded Givi
E470 Monolock top box. The only difference between the Kawa box and the Givi box is the lid panel. The Givi lid 
panel already includes a lens for a center mounted light assembly. Because of this there is some spare space under
the panel lid in the Kawa box. in that spare space I will mount my arduino contraption which includes a buck converter
and an Arduino Nano. 
In total I will mount 6 LED strips of various lengths of WS2812B with 60 LEDs/m, two of each with 9, 8 and 6 LEDs. 
The total mounted LEDs equals 2x9 + 2x8 + 2x6 = 46 LEDs. On white, with full brightness this would total to about 46 x
60mA = 2.76A. There is an accessoiry lead in the tail of the bike which is fused 5A, only problem is that it shares the
fuse with the accesoiry lead in the fairing that is in use for my Garmin. (will have to check about the cig lighter on 
the right, if that is using the same fuse)
Also it doesn't really make sense to create white light when the additional colors are being filtered away by the lens, 
because of this I will only use the Red section of the Neopixels. This would also reduce the power draw to about 46 x 
20mA = 0.92A for the LEDs. The Arduino Nano takes about 40mA in worst case, but total current
draw would be at most 1.25A continous while braking.

-------R-R-R-R-#-#-#-#-#            #-#-#-#-#-R-R-R-R-------   R = LEDSs that will function as running lights
---------R-R-R-#-#-#-#-#  KAWASAKI  #-#-#-#-#-R-R-R---------
-------------R-#-#-#-#-#            #-#-#-#-#-R-------------
    ^                                                   ^
  wire leads                                          wire leads

The connection to the strips will be on the outside left of the strips on the left and outside right for the strips on 
the right.
The factory kit will only do brake lights, Admore is selling a kit that would do running/brake lights and turning signals 
with a strip of 9 and 6 LEDs on each side.
This setup will provide:
- Running lights (dimmed, only outside LEDs)
- Brake light (both sides on, warning flash at start. All LEDs)
- Turn signal (one side all LEDs sequential flashing, other side running lights unless braking then other side is solid on 
  (with start flash).)
- Hazard light (both sides sequential flashing from inside to outside)

This can obviously be customized as needed.  To convert the 12V signals to the Arduino Inputs 5V signal I used optocouplers.
This setup uses an Arduino Nano and a 12VDC buck converter to obtain a 5V power source to pwer the Arduino and the LED strips.
Because of this setup I can not use the 2 wire kit that Givi sells and uses contacts for easy removal of the box.
For this setup we need to bring 5 wires to the box:
- 12V
- Ground
- Left turn signal
- Right turnsignal
- Brake light
I bought some water proof connecters but also a 5 pin trailer wiring harness. which I probably hide somewhere under the seat.

Timings used in this sketch still have to be adapted to the bike escpecially the times used in the turn signals to fill the 
strip up and the pulse monitoring before dropping the turn signal
*/

// Include FastLED library
#include <FastLED.h>

// Some definitions for the inputs
#define brakePin 8                       // Wire from the brakeLight
#define rightPin 9                       // Wire from the right turnsignal
#define leftPin 10                       // Wire from the left turnsignal

// Setting some definitions for the LEDs
#define LED_TYPE WS2812B                 // Type of NeoPixel strip used
#define NUM_LEDS 9                       // Define the number of LEDs in each strip.
#define NUM_LEDS_L 9                     // Define the number of LEDs in the long strips.
#define NUM_LEDS_M 8                     // Define the number of LEDs in the medium strips.
#define NUM_LEDS_S 6                     // Define the number of LEDs in the short strips.  
#define NUM_STRIPS 6                     // Define the number of LED strips being used.

// Settings for brightness levels
#define bright 250                       // Define the level for bright
#define dim 80                           // Define the level for dim

// Define Full brightness, max 255, all colors
#define r bright
#define g bright
#define b bright

// Define low brightness, all colors
#define r2 dim
#define g2 dim
#define b2 dim

// variables for color brightness calculations
byte rt = 0;               // Temp red
byte gt = 0;               // Temp green 
byte bt = 0;               // Temp blue

#define Color_high CRGB(r,0,0)        // Define high brightness state, will use 255 for red (bright)
#define Color_low CRGB(r2,0,0)        // Define dim brightness state, will use 80 for red (dim)

CRGB leds[NUM_STRIPS][NUM_LEDS];      // Sets up the array to be used for setting LED colors and outputs.
int current_led = NUM_LEDS;

// As I am using millis() to make the sketch non blocking I need some variable to keep track of times
// using unsigned longs as they can only be positive number but can grow pretty quick
// unsigned long is 4 byte big and can store o to 4,294,967,295
unsigned long brakeMillis = 0;
const unsigned long brakeFlashMillisQ = 67UL; // value isn't changing, hence the const
const unsigned long brakeFlashMillis = 267UL; // value isn't changing, hence the const
unsigned long lastBlinkRTime = 0;
unsigned long lastBlinkLTime = 0;
unsigned long lastPulseTime = 0;
unsigned long lastPulseTimeL = 0;
unsigned long lastPulseTimeR = 0;
unsigned long lastPulseTimeB = 0;
unsigned long blinkRStartAllOnTime = 0;
unsigned long blinkRStartAllOffTime = 0;
const unsigned long pulseHoldTime = 1000UL; // Time to hold the pulse after last input signal, for turnsignals.
unsigned long blinkInterval = 150;

// Some booleans for status
bool flash = false;
bool turnStateL = LOW;
bool turnStateR = LOW;
bool isRBlinking = false;
bool isLBlinking = false;
bool isBraking = false;
bool brakeSet = false;
bool runStateR = false;
bool runStateL= false;
bool runHasBeenOn = false;
bool blink;
bool allOffRPhase = false;
bool allOnRPhase = false;

//volatile unsigned long lastPulseTime = 0;
volatile unsigned long timeBetweenPulses = 0;

// using byte to preserve memory
byte brakeCounter;
  byte x = 0;

void setup() {
  Serial.begin(9600);
  //Defining LED-strips
  FastLED.addLeds<LED_TYPE, 2,GRB>(leds[0], NUM_LEDS_L);   // This sets up the left strip to use Pin 2 using the leds aray (defined earlier)
  FastLED.addLeds<LED_TYPE, 3,GRB>(leds[1], NUM_LEDS_L);   // This sets up the left strip to use Pin 3 using the leds aray (defined earlier)
  FastLED.addLeds<LED_TYPE, 4,GRB>(leds[2], NUM_LEDS_M);   // This sets up the left strip to use Pin 4 using the leds aray (defined earlier)
  FastLED.addLeds<LED_TYPE, 5,GRB>(leds[3], NUM_LEDS_M);   // This sets up the left strip to use Pin 5 using the leds aray (defined earlier)
  FastLED.addLeds<LED_TYPE, 6,GRB>(leds[4], NUM_LEDS_S);   // This sets up the left strip to use Pin 6 using the leds aray (defined earlier)
  FastLED.addLeds<LED_TYPE, 7,GRB>(leds[5], NUM_LEDS_S);   // This sets up the left strip to use Pin 7 using the leds aray (defined earlier)
  FastLED.clear ();                                        // Make sure all LEDs are turned off for initial setup.  

  // Some animation to show when the contact is turned on.
  delay(200);                                 // Wait before start animation.

  // Knight Rider animationstyle, but ending when it reaches to other end.
  for (int i = 0; i < NUM_LEDS; i++) {        // This will change the light for every LED going from the last to first.
    {
      leds[0][i] = Color_low;                 // set up LED[i] on strip 0 to Color_low (dim)
      leds[1][i] = Color_low;                 // set up LED[i] on strip 1 to Color_low (dim)
      if (i>0) {                              // For strips with less LEDs we need to offset the LED number.
        leds[2][i-1] = Color_low;             // set up LED[i-1] on strip 2 to Color_low (dim)  
        leds[3][i-1] = Color_low;             // set up LED[i-1] on strip 3 to Color_low (dim)
      }
      if (i >3) {                             // For strips with less LEDs we need to offset the LED number.
        leds[4][i-3] = Color_low;             // set up LED[i-3] on strip 4 to Color_low (dim)
        leds[5][i-3] = Color_low;             // set up LED[i-3] on strip 5 to Color_low (dim)
      }
    }
    FastLED.show();                           // Send the data to LEDs and turn them on
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[0][i] = CRGB::Black;               // Turn off LED in prep for next cycle  
      leds[1][i] = CRGB::Black;               // Turn off LED in prep for next cycle
      leds[2][i] = CRGB::Black;               // Turn off LED in prep for next cycle
      leds[3][i] = CRGB::Black;               // Turn off LED in prep for next cycle
      leds[4][i] = CRGB::Black;               // Turn off LED in prep for next cycle
      leds[5][i] = CRGB::Black;               // Turn off LED in prep for next cycle
    }
    delay(75);                                // Wait before next LED lights up.
  }
  FastLED.show();                             // Output the last LED color to turn off remaining LED.    
  delay(100);
  // This sequence will solidly light up the strip one LED at a time in the opposite direction.  
  for (int i = NUM_LEDS-1; i > -1; i--) {     // This will change the light for every LED going from the last to first.
    leds[0][i] = Color_low;                   // Set strip LED color to the chosen color.
    leds[1][i] = Color_low;                   // Set strip LED color to the chosen color.
    leds[2][i] = Color_low;                   // Set strip LED color to the chosen color.
    leds[3][i] = Color_low;                   // Set strip LED color to the chosen color.
    leds[4][i-1] = Color_low;                 // Set strip LED color to the chosen color.
    leds[5][i-1] = Color_low;                 // Set strip LED color to the chosen color.
    FastLED.show(); 
    delay(150);                               // Show the set colors. 
  }
  for (int i = 0; i < 9; i++) {               // This will change the light for every LED going from the last to first.
    leds[0][i] = Color_high;                  // Set strip LED color to the chosen color.
    leds[1][i] = Color_high;                  // Set strip LED color to the chosen color.
    leds[2][i] = Color_high;                  // Set strip LED color to the chosen color.
    leds[3][i] = Color_high;                  // Set strip LED color to the chosen color.
    leds[4][i] = Color_high;                  // Set strip LED color to the chosen color.
    leds[5][i] = Color_high;                  // Set strip LED color to the chosen color.   
    FastLED.show();                           // Show the set colors. 
    
  }
  delay(400);
  for (int i = 0; i < 256; i++) {              // Gradually fade the light to off.  
    rt = r - i;
    gt = g - i;
    bt = b - i;
    for (int i = 0; i < 9; i++) {              // This will change the light for every LED going from the last to first.
      leds[0][i] = CRGB( rt, 0, 0);            // Set strip 0 LED color to the chosen color.
      leds[1][i] = CRGB( rt, 0, 0);            // Set strip 0 LED color to the chosen color.
      leds[2][i] = CRGB( rt, 0, 0);            // Set strip 0 LED color to the chosen color.
      leds[3][i] = CRGB( rt, 0, 0);            // Set strip 0 LED color to the chosen color.
      leds[4][i] = CRGB( rt, 0, 0);            // Set strip 0 LED color to the chosen color.
      leds[5][i] = CRGB( rt, 0, 0);            // Set strip 0 LED color to the chosen color.
    }
    FastLED.show();
    delay(5);
  }
}

void loop() {

  // Here we read the input pin for the right turn signal and have a wait period to see if signal is still active.
  // This to bridge the off-time of the turnsignal.
  if (digitalRead(rightPin) == HIGH){          // If right turn signal is active
    lastPulseTimeR = millis();                 // Update the last pulse time to current time
    isRBlinking = true;                        // Set the right blinking state to true
    runStateR =false;                          // Set running light right state to false
  } else if (isRBlinking) {
    blinkInterval = millis() -  lastPulseTimeR;
  }
  if (millis() - lastPulseTimeR >= pulseHoldTime){ // Check if the time since last pulse exceeds the hold time
    isRBlinking = false;                       // If exceeded, set right blinking state to false
  }

  // Here we read the input pin for the left turn signal and have a wait period to see if signal is still active. 
  if (digitalRead(leftPin) == HIGH){           // If left turn signal is active
    lastPulseTimeL = millis();                 // Update the last pulse time to current time
    isLBlinking = true;                        // Set the left blinking state to true
    runStateL =false;                          // Set running light left state to false
  } else if (isLBlinking) {
    blinkInterval = millis() - lastPulseTimeL;
  }
  if (millis() - lastPulseTimeL >= pulseHoldTime){ // Check if the time since last pulse exceeds the hold time
    isLBlinking = false;                       // If exceeded, set left blinking state to false
  }

  // Here we read the input pin for the brake light  
  if (digitalRead(brakePin) == HIGH){
    isBraking = true;
  } else {
    isBraking = false;
    brakeCounter = 0;                          // Reset brake counter when brake no longer applied
    resetBrake();                              // Reset brake lights when brake is released
  }

  // Combine the input states into a single variable for easier handling
  // Bit 0 = Brake, Bit 1 = Right turn, Bit 2 = Left turn
  // This will create a byte variable with values from 0 to 7.
  x = digitalRead(brakePin) | (isRBlinking << 1) | (isLBlinking << 2);
  
  // Here we use a switch case to handle the different combinations of inputs
  switch(x){
  case 0:                                      // Running lights -- no signals present
    runRight();                                // Run right side running lights
    runLeft();                                 // Run left side running lights
    current_led = 9;                           // Reset current_led for turn signals so they start from beginning when activated
    break;
  case 1:                                      // Brake only 
    brake();
    break;
  case 2:                                      // Right turn only
    blinkRelay();
    rightTurn();                               // Call rightTurn function
    runLeft();                                 // Run left side running lights
    break;
  case 3:                                      // Brake and RightTurn
    blinkRelay();
    brake();                                   // Call brake function
    rightTurn();                               // Call rightTurn function
    break;
  case 4:                                      // Left turn
    blinkRelay();
    leftTurn();                                // Call leftTurn function
    runRight();                                // Run right side running lights
    break;
  case 5:                                      // Brake and Left turn
    blinkRelay();
    brake();                                   // Call brake function
    leftTurn();                                // Call leftTurn function
    break;
  case 6:                                      // Hazards
    blinkRelay();
    hazards();
    break;
  case 7:                                      // Brake and Hazards
    blinkRelay();
    hazards();                                 // Call hazards function, brake will be ignored as hazards override brake
    break;
  } // END CASE


 
}// END LOOP

// void blink(){
  
// }

void resetBrake(){
  if (brakeSet){
    fill_solid(leds[1],9,CRGB::Black);
    fill_solid(leds[3],8,CRGB::Black);
    fill_solid(leds[5],6,CRGB::Black);  
    fill_solid(leds[0],9,CRGB::Black);
    fill_solid(leds[2],8,CRGB::Black);
    fill_solid(leds[4],6,CRGB::Black);
    FastLED.show();
    brakeSet = false;
  }
  brakeCounter=0;
}

void brake() {
  brakeSet = true;
  // 5 flashes then stays on till brake is released
  Serial.println(brakeCounter);
  unsigned long currentMillis = millis();           // Get the current time, local variable
  if (isBraking && (isRBlinking||isLBlinking)) {
    if (isRBlinking) {
      fill_solid(leds[1],9,CRGB::Color_high);       // Set left side brake lights on
      fill_solid(leds[3],8,CRGB::Color_high);       // Set left side brake lights on
      fill_solid(leds[5],6,CRGB::Color_high);       // Set left side brake lights on
    }
    if (isLBlinking) {
      fill_solid(leds[0],9,CRGB::Color_high);       // Set right side brake lights on
      fill_solid(leds[2],8,CRGB::Color_high);       // Set right side brake lights on
      fill_solid(leds[4],6,CRGB::Color_high);       // Set right side brake lights on
    }
  } else if (isBraking){                                  // If brake is active
    if ((currentMillis - brakeMillis > brakeFlashMillisQ && brakeCounter <= 8)) { // Check elapsed time since last flash
      flash = ! flash;                                    // Toggle the flash state  
      brakeCounter++;                                   // Increment the brake flash counter  
      brakeMillis = currentMillis;                   // Update last flash time to current time  
    }
    if ((currentMillis - brakeMillis > brakeFlashMillis && brakeCounter > 8)) { // Check elapsed time since last flash
      Serial.println(brakeCounter);
      flash = ! flash;                                    // Toggle the flash state  
      if(brakeCounter < 17) {                              // Check if we are still in the flashing phase, number is double the number of flashes
        brakeCounter++;                                   // Increment the brake flash counter  
      }
      brakeMillis = currentMillis;                   // Update last flash time to current time  
    }
    if (flash == 1) {                                // If flash state is on, turn on all LEDs
        fill_solid(leds[0],9,CRGB::Color_high);
        fill_solid(leds[2],8,CRGB::Color_high);
        fill_solid(leds[4],6,CRGB::Color_high);
        fill_solid(leds[1],9,CRGB::Color_high);
        fill_solid(leds[3],8,CRGB::Color_high);
        fill_solid(leds[5],6,CRGB::Color_high);
      } else {
      if (brakeCounter <= 16) {                       // During flashing phase, turn off all LEDs until we exceed the counter
        if (isLBlinking){                            // If left turn is active, only turn off righ side LEDs 
          fill_solid(leds[0],9,CRGB::Black);
          fill_solid(leds[2],8,CRGB::Black);
          fill_solid(leds[4],6,CRGB::Black);
        } else if (isRBlinking){                     // If right turn is active, only turn off left side LEDs
          fill_solid(leds[1],9,CRGB::Black);
          fill_solid(leds[3],8,CRGB::Black);
          fill_solid(leds[5],6,CRGB::Black);
          } else {
          fill_solid(leds[0],9,CRGB::Black);         // If no turn signals active, turn off all LEDs
          fill_solid(leds[2],8,CRGB::Black);
          fill_solid(leds[4],6,CRGB::Black);
          fill_solid(leds[1],9,CRGB::Black);
          fill_solid(leds[3],8,CRGB::Black);
          fill_solid(leds[5],6,CRGB::Black);
        }
      }
    }
    FastLED.show();
  }
}  

void blinkRelay(){
  if (digitalRead(rightPin) || digitalRead(leftPin)) {
    blink = true;
  } else {
    blink = false;
  }
  
}

void rightTurn() { 
  Serial.print("blink: ");Serial.println(blink);
  if (blink) {
    if (current_led == 9) {
      //FastLED.clear();
      fill_solid(leds[0],9,CRGB::Black);
      fill_solid(leds[2],8,CRGB::Black);
      fill_solid(leds[4],6,CRGB::Black);
    }
    unsigned long currentMillis = millis();          // Get the current time, local variable
    if (currentMillis - lastBlinkRTime >= blinkInterval/12){ // Check elapsed time since last blink
      lastBlinkRTime = currentMillis;                // Update last blink time to current time
      current_led--;
      if (current_led >= 0){
        leds[0][current_led]=CRGB::Red;
      if (current_led <= 7){
        leds[2][current_led]=CRGB::Red;
      }
      if (current_led <= 6){
        if (current_led -1 >= 0){
          leds[4][current_led-1]=CRGB::Red;
        }
      }
      FastLED.show();
      } else if (current_led < 0){              // If all LEDs have been lit turn them off and reset
        current_led=NUM_LEDS;
        fill_solid(leds[0],9,CRGB::Black);
        fill_solid(leds[2],8,CRGB::Black);
        fill_solid(leds[4],6,CRGB::Black);
        FastLED.show();
      }
    }
  } else {
    fill_solid(leds[0],9,CRGB::Black);
    fill_solid(leds[2],8,CRGB::Black);
    fill_solid(leds[4],6,CRGB::Black);
    current_led=NUM_LEDS;
    FastLED.show();
  }
}

void leftTurn() { //by passing a bit this could work left and right
  
  if (blink){
    if (current_led == 9) {
      //FastLED.clear();
      fill_solid(leds[1],9,CRGB::Black);
      fill_solid(leds[3],8,CRGB::Black);
      fill_solid(leds[5],6,CRGB::Black);
    }
    unsigned long currentMillis = millis();
    if (currentMillis - lastBlinkLTime >= blinkInterval/12){
      lastBlinkLTime = currentMillis;
      current_led--;
      if (current_led >= 0){
        leds[1][current_led]=CRGB::Red;
      
      if (current_led <= 7){
        leds[3][current_led]=CRGB::Red;
      }
      if (current_led <= 6){
        if (current_led -1 >= 0){
          leds[5][current_led-1]=CRGB::Red;
        }
      }
      FastLED.show();
    }
    else if (current_led < 0){
      current_led=NUM_LEDS;
      fill_solid(leds[1],9,CRGB::Black);
      fill_solid(leds[3],8,CRGB::Black);
      fill_solid(leds[5],6,CRGB::Black);
      FastLED.show();
    }

    }
  } else {
    fill_solid(leds[1],9,CRGB::Black);
    fill_solid(leds[3],8,CRGB::Black);
    fill_solid(leds[5],6,CRGB::Black);
    current_led=NUM_LEDS;
    FastLED.show();
  } 
}

void hazards() { 
  if (!turnStateR) {
    fill_solid(leds[0],9,CRGB::Black);
    fill_solid(leds[2],8,CRGB::Black);
    fill_solid(leds[4],6,CRGB::Black);
    fill_solid(leds[1],9,CRGB::Black);
    fill_solid(leds[3],8,CRGB::Black);
    fill_solid(leds[5],6,CRGB::Black);
    FastLED.show();
    turnStateR = true;
    current_led = 9;
  }

  if (isRBlinking){
    unsigned long currentMillis = millis();
    if (currentMillis - lastBlinkRTime >= blinkInterval){
      lastBlinkRTime = currentMillis;
      current_led--;
      if (current_led >= 0){
        leds[0][current_led]=CRGB::Red;
        leds[2][current_led]=CRGB::Red;
        leds[4][current_led-1]=CRGB::Red;
      if (current_led <= 8){
        leds[1][current_led]=CRGB::Red;
        leds[3][current_led]=CRGB::Red;
        leds[5][current_led-1]=CRGB::Red;
      }
      FastLED.show();
    }
    else if (current_led < 0){
      current_led=NUM_LEDS;
      fill_solid(leds[0],9,CRGB::Black);
      fill_solid(leds[1],9,CRGB::Black);
      fill_solid(leds[2],8,CRGB::Black);
      fill_solid(leds[3],8,CRGB::Black);
      fill_solid(leds[4],6,CRGB::Black);
      fill_solid(leds[5],6,CRGB::Black);
      FastLED.show();
    }

    }
  } else {
    fill_solid(leds[0],9,CRGB::Black);
    fill_solid(leds[1],9,CRGB::Black);
    fill_solid(leds[2],8,CRGB::Black);
    fill_solid(leds[3],8,CRGB::Black);
    fill_solid(leds[4],6,CRGB::Black);
    fill_solid(leds[5],6,CRGB::Black);
    current_led=NUM_LEDS;
    FastLED.show();
  }
}

void runLeft(){
  static unsigned long rightRunningMillis;
  if (millis() - lastPulseTime >= pulseHoldTime){
  if (!runStateL||!runHasBeenOn){
    fill_solid(leds[1],9,CRGB::Black);
    fill_solid(leds[3],8,CRGB::Black);
    fill_solid(leds[5],6,CRGB::Black);
    FastLED.show();
    runStateL =true;
    runHasBeenOn = true;
  }
  else {
    fill_solid(leds[1], 4,CRGB::Color_low);
    fill_solid(leds[3], 3,CRGB::Color_low);
    fill_solid(leds[5], 1,CRGB::Color_low);
  }
  // }
  rightRunningMillis = millis();
  FastLED.show();}
}

void runRight(){
  static unsigned long rightRunningMillis;
  if (millis() - lastPulseTime >= pulseHoldTime){
  if (!runStateR||!runHasBeenOn){
    fill_solid(leds[0],9,CRGB::Black);
    fill_solid(leds[2],8,CRGB::Black);
    fill_solid(leds[4],6,CRGB::Black);
    FastLED.show();
    runStateR =true;
  }
  else {
    fill_solid(leds[0], 4,CRGB::Color_low);
    fill_solid(leds[2], 3,CRGB::Color_low);
    fill_solid(leds[4], 1,CRGB::Color_low);
  }
  // }
  rightRunningMillis = millis();
  FastLED.show();}
}

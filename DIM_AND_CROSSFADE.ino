
const boolean COMMON_ANODE = false; 


// colors of the light bulb
const int RGB_RED_PIN = 6;
const int RGB_GREEN_PIN  = 5;
const int RGB_BLUE_PIN  = 3;

const int DELAY_MS = 20; // delay in ms between changing colors
const int MAX_COLOR_VALUE = 255; // color can be inbetween 0 and 255


// in order for the photocell to react to light
const int MIN_PHOTOCELL_VAL = 200; 
const int MAX_PHOTOCELL_VAL = 800; 
const boolean PHOTOCELL_IS_R2_IN_VOLTAGE_DIVIDER = true; // set false if photocell is R1
const int PHOTOCELL_INPUT_PIN = A5;


// in order for the button to change modes
const int INPUT_BUTTON_PIN = 10;
unsigned long lastTimeButtonStateChanged = millis();
unsigned long debounceDuration = 50;
byte ledState = LOW;
byte lastButtonState;
bool crossfadeRunning = false;
bool slider = false;
const int LOFI_INPUT = A0;

// for the force resistor
const int FORCE_RESISTOR = A2;



enum RGB{
  RED,
  GREEN,
  BLUE,
  NUM_COLORS
};

int _rgbLedValues[] = {255, 0, 0}; // Red, Green, Blue
enum RGB _curFadingUpColor = GREEN;
enum RGB _curFadingDownColor = RED;
const int FADE_STEP = 5;  


void setup() {
  // Set the RGB pins to output
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);

  pinMode(LOFI_INPUT, INPUT); //setup the button to input
  pinMode(PHOTOCELL_INPUT_PIN, INPUT); // setup the photocell to input

  lastButtonState = digitalRead(INPUT_BUTTON_PIN);
  pinMode(INPUT_BUTTON_PIN, INPUT); //set up the button to input

  // Turn on Serial so we can verify expected colors via Serial Monitor
  Serial.begin(9600); 
  Serial.println("Red, Green, Blue");

  // Set initial color
  setColor(0, 0, 0);
  delay(DELAY_MS);
}

void loop() {

  


  if(millis()- lastTimeButtonStateChanged >= debounceDuration) {
    // read the state of the pushbutton value
    byte buttonState = digitalRead(INPUT_BUTTON_PIN);
    
    if(buttonState != lastButtonState) {
      lastTimeButtonStateChanged = millis();
      lastButtonState = buttonState;
      if(buttonState == LOW) { // released
        
        if (ledState == HIGH) {
          ledState = LOW;
          slider = !slider;
          if(slider) {
            // call mode 2
            slideColor();
          }
          
        } else {
          ledState = HIGH;
          crossfadeRunning = !crossfadeRunning;
          if(crossfadeRunning) {
            // call mode 1
            callCrossfade();
          }
          
        }
        
      } 
      
    }

   
    
  }

  

}

void goDawgs() {
  unsigned long startTime = millis();
  while (millis() - startTime < 3000) { // Run for 3 seconds
    // Purple color (red + blue)
    setColor(200, 30, 255); // Full intensity for red and blue
    delay(200); // Delay between color changes

    // Gold color (red + green)
    setColor(0, 75, 255); // Full intensity for red, 215 for green (approximately gold)
    delay(200); // Delay between color changes
  }
}

// mode 2. Changes the color depending on the current variable resisistance
void slideColor() {
  
  while(slider) {
    int analogReading = analogRead(FORCE_RESISTOR);
    //Serial.println(analogReading);
    if (analogReading > 230) {
      goDawgs();
      //setColor(255, 30, 255);
    } else {
      int val = analogRead(LOFI_INPUT); // current variable resistance
      Serial.println(val);
      delay(10);
      if(val < 550) {
        setColor(255, 0, 0);
        delay(DELAY_MS);

      } else if (val >= 550 && val < 680) {
        setColor(0, 255, 0);
        delay(DELAY_MS);
      } else {
        setColor(0, 0, 255);
        delay(DELAY_MS);
      }
    }
    

    byte buttonState = digitalRead(INPUT_BUTTON_PIN);

    if(buttonState != lastButtonState) {
      lastTimeButtonStateChanged = millis();
      lastButtonState = buttonState;
      slider = false;
    }
  }
}


// method to create color crossfade
void callCrossfade() {
  while(crossfadeRunning) {
    int analogReading = analogRead(FORCE_RESISTOR);
    Serial.println(analogReading);
    if (analogReading > 230) {
      goDawgs();
      //setColor(255, 30, 255);
    } else {
      // updates the fade each call
      updateFade();

      // reacts to photocell
      int ledVal = getPhotocell();


      int red = _rgbLedValues[RED] * ledVal / MAX_COLOR_VALUE;
      int green = _rgbLedValues[GREEN] * ledVal / MAX_COLOR_VALUE;
      int blue = _rgbLedValues[BLUE] * ledVal / MAX_COLOR_VALUE;

      setColor(red, green, blue);
      delay(DELAY_MS);
      
    }

    byte buttonState = digitalRead(INPUT_BUTTON_PIN);
    if(buttonState != lastButtonState) {
      lastTimeButtonStateChanged = millis();
      lastButtonState = buttonState;
      crossfadeRunning = false;
    }

  }
}

// increments and decrements RBG LED values as needed.
void updateFade() {
  
  _rgbLedValues[_curFadingUpColor] += FADE_STEP;
  _rgbLedValues[_curFadingDownColor] -= FADE_STEP;

  if(_rgbLedValues[_curFadingUpColor] > MAX_COLOR_VALUE){
    _rgbLedValues[_curFadingUpColor] = MAX_COLOR_VALUE;
    _curFadingUpColor = (RGB)((int)_curFadingUpColor + 1);

    if(_curFadingUpColor > (int)BLUE){
      _curFadingUpColor = RED;
    }
  }

  if(_rgbLedValues[_curFadingDownColor] < 0){
    _rgbLedValues[_curFadingDownColor] = 0;
    _curFadingDownColor = (RGB)((int)_curFadingDownColor + 1);

    if(_curFadingDownColor > (int)BLUE){
      _curFadingDownColor = RED;
    }
  }
}


// reads from photo-sensitive resistor value and gets the current ledVal that is needed
int getPhotocell() {
 
  int photocellVal = analogRead(PHOTOCELL_INPUT_PIN);

  int ledVal = map(photocellVal, MIN_PHOTOCELL_VAL, MAX_PHOTOCELL_VAL, 0, 255);

  ledVal = constrain(ledVal, 0, 255);

  if(PHOTOCELL_IS_R2_IN_VOLTAGE_DIVIDER == true){
    // We need to invert the LED (it should be brighter when environment is darker)
    ledVal = 255 - ledVal;
  }

  
  Serial.print(photocellVal);
  Serial.print(",");
  Serial.println(ledVal);

  return ledVal;
}


// Takes in the three values of the colors from 0-255 and sets the color accordingly.
// blue, green, red
void setColor(int red, int green, int blue)
{
  Serial.print(red);
  Serial.print(", ");
  Serial.print(green);
  Serial.print(", ");
  Serial.println(blue);

  // If a common anode LED, invert values
  if(COMMON_ANODE == true){
    red = MAX_COLOR_VALUE - red;
    green = MAX_COLOR_VALUE - green;
    blue = MAX_COLOR_VALUE - blue;
  }
  analogWrite(RGB_RED_PIN, red);
  analogWrite(RGB_GREEN_PIN, green);
  analogWrite(RGB_BLUE_PIN, blue);  
}

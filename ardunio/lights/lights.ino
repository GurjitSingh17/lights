#include <stdlib.h>

// Sound activated LEDs with the Trinket and NeoPixels
#include <Adafruit_NeoPixel.h>

#define MIC_PIN A1// Microphone
#define LED_PIN 6 // NeoPixel LED strand
#define N_PIXELS 60 // number of pixels in LED strand
#define N 50 // Number of samples to take each time readSamples is called
#define fadeDelay 15 // delay time for each fade amount
#define noiseLevel 12 // slope level of average mic noise without sound

const int BOTTOM_PIXEL_END = 17; // index of the last pixel on the bottom row
const int SIDE_PIXEL_END = 45; // index of the last side pixel
const int MAX_BRIGHTNESS = 100; // max brightness of pixels
const float DEFAULT_DIM_BRIGHTNESS_PERCENTAGE = .20;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

int samples[N]; // storage for a sample collection set

//vars for music reactive mode, readSamples
const int numReadings = 10;
int readings[numReadings];      // the readings from the analog input

uint8_t mode = 0;
uint8_t prevMode = 0;

int currBrightness = MAX_BRIGHTNESS;
uint32_t prevColor = strip.Color(0,0,0); //black
uint32_t currColor = strip.Color(0,0,0); //black

bool lightsOn = false;
void (*noArgFn[3])() = {musicReactive, rainbowWithMusic, fire};
void (*colorOnlyFn[2]) (uint32_t c) = {showColor, bigPixels};
void (*delayFn[4]) (uint8_t delay) = {rainbow, rainbowCycle, theaterChaseRainbow, rainbowBreathing};
void (*delayColorFn[4]) (uint8_t delay, uint32_t c) = {strobe, theaterChase, breathing, dither};

struct Reading {
  int sample_index = 0;
  int total = 0;
  int average = 0;
  int last_reading = 0;
  int new_reading = 0;
} reading;

// Arduino setup Method
void setup() {
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0;
  Serial.begin(9600);
  strip.begin();
  ledsOff();
  delay(500);
}

// Arduino loop Method
void loop() {
  int bytes_availabe = Serial.available();
  if (bytes_availabe > 0){
    uint8_t mode = Serial.read();
    Serial.println("---" + mode);
    uint8_t index = 0;
    if (mode <= 20){
      index = mode - 1;
      resetLights();
      (*noArgFn[index])();
    } else if (mode <= 40){
      if (waitForBytes(3, 80)){
        index = mode - 21;
        uint8_t r = Serial.read();
        uint8_t g = Serial.read();
        uint8_t b = Serial.read();
        resetLights();
        (*colorOnlyFn[index])(strip.Color(r,g,b));
      }
    } else if (mode <= 60){
      if (waitForBytes(1, 20)){
        uint8_t delayTime = Serial.read();
        delayTime = delayTime ? delayTime : fadeDelay;
        index = mode - 41;
        resetLights();
        (*delayFn[index])(delayTime);
      }
    } else if (mode <= 80){
      if (mode == 65){
        if (waitForBytes(5, 90)){
          uint8_t r = Serial.read();
          uint8_t g = Serial.read();
          uint8_t b = Serial.read();
          uint8_t delayTime = Serial.read();
          uint8_t width = Serial.read();
          delayTime = delayTime ? delayTime : fadeDelay;
          resetLights();
          wave(delayTime, width, strip.Color(r,g,b));
        }
      } else if (waitForBytes(4, 80)){
        uint8_t r = Serial.read();
        uint8_t g = Serial.read();
        uint8_t b = Serial.read();
        uint8_t delayTime = Serial.read();
        delayTime = delayTime ? delayTime : fadeDelay;
        index = mode - 61;
        resetLights();
        (*delayColorFn[index])(delayTime, strip.Color(r,g,b));
      }
    } else if (mode <= 100){
      if (mode == 81){
        if (waitForBytes(9, 200)){
          uint32_t colors[3];
          for(int i = 0; i < 3; i++){
            uint8_t r = Serial.read();
            uint8_t g = Serial.read();
            uint8_t b = Serial.read();
            colors[i] = strip.Color(r,g,b);
          }
          resetLights();
          threeZone(colors[0], colors[1], colors[2]);
        }
      } else if (mode == 82){
        if (waitForBytes(2, 40)){
          uint8_t delayTime = Serial.read();
          uint8_t numColors = Serial.read();
          delayTime = delayTime ? delayTime : fadeDelay;
          if(waitForBytes(numColors*3, min(40*numColors, 500))){
            uint32_t * colors = new uint32_t[numColors];
            for(int i = 0; i < numColors; i++){
              uint8_t r = Serial.read();
              uint8_t g = Serial.read();
              uint8_t b = Serial.read();
              colors[i] = strip.Color(r,g,b);
            }
            resetLights();
            breathing(delayTime, colors, numColors);
          }
        }
      }
    } else {
      //flushBuffers();
    }
  }
}

void resetLights(){
  strip.clear();
  setBrightness(MAX_BRIGHTNESS);
}

bool waitForBytes(uint8_t numBytes, uint16_t timeout){
  unsigned long waitTill = millis() + timeout;
  while (millis() < waitTill){
    if (Serial.available() >= numBytes){
      return true;
    }
  }
  return false;
}

void flushBuffers(){
  while(Serial.available()){
    Serial.read();
  }
  Serial.flush();
}

void dimLights(int percentage){
  int brightness;
  //Serial.println(currBrightness);
  if (percentage == -1){
    brightness = currBrightness * DEFAULT_DIM_BRIGHTNESS_PERCENTAGE;
  } else {
    brightness =   currBrightness * ( (float) percentage / 100);
  }
  //Serial.println(brightness);
  setBrightness(brightness);
  strip.show();
  lightsOn = false;
}

void setBrightness(int brightness){
  currBrightness = max(2, min(brightness, MAX_BRIGHTNESS));
  strip.setBrightness(currBrightness);
}

uint32_t getColor(int period) {
  if(period == -1)
    return Wheel(0);
  else if(period > 400)
    return Wheel(5);
  else
    return Wheel(map(-1*period, -400, -1, 50, 255));
}

void fadeOut(int delayTime)
{
  while ((currBrightness - 10) >= 0) {
    setBrightness(currBrightness - 10);
    strip.show(); // Update strip
    if(delayAndCheck(delayTime)) return;
  }
  setBrightness(0);
  strip.show();
  lightsOn = false;
}

void fadeIn(int delayTime) {
  // fade color in
  while ((currBrightness + 10) < MAX_BRIGHTNESS){
    setBrightness(currBrightness + 10);
    strip.show(); // Update strip
    if(delayAndCheck(delayTime)) return;
  }
  setBrightness(MAX_BRIGHTNESS);
  strip.show();
  lightsOn = true;
}

void ledsOff() {
  fadeOut(fadeDelay);
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
  lightsOn = false;
}

void ledsOffFast(){
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
  lightsOn = false;
}

void showColor(uint32_t color) {
  lightsOn = true;
  setBrightness(MAX_BRIGHTNESS);
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
  return;
}

void setColor(uint32_t color){
  // set the color at current brightness and does not show it , this differs from showColor
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
}

void makeReading(){
  reading.last_reading = reading.new_reading;
  int val = analogRead(MIC_PIN) - 350;
  reading.total -= readings[reading.sample_index];         
  // read from the sensor:  
  readings[reading.sample_index] = val;
  reading.total += readings[reading.sample_index];
  reading.sample_index++;
  if (reading.sample_index > numReadings)
    reading.sample_index = 0;
  reading.new_reading = reading.total / numReadings;
}

// Read and Process Sample Data from Mic
void musicReactive() {
  while (true){
    makeReading();
    int slope = reading.new_reading - reading.last_reading;
    //Serial.println(new_reading);
    // Check if Slope greater than noiseLevel - sound that is not at noise level detected
    if(abs(slope) > noiseLevel) {
      threeZone(
        Wheel((int) ( min(255, (3.2 * (reading.new_reading - 60)) + (rand()  % 25) - (rand()  % 10)))),
        Wheel((int) ( (1.2 * (reading.new_reading - 60)) + (rand()  % 40) - (rand()  % 10)) %255),
        Wheel((int) ( min(255, (3.2 * (reading.new_reading - 60)) + (rand()  % 25) - (rand()  % 10)))));
      makeReading();
    }
    else {
      if (lightsOn){
        dimLights(-1);
      }
    }
    if(delayAndCheck(10)) return;
  }
}

void rainbowWithMusic() {
  uint8_t wait = 10;
  uint16_t i;
  uint16_t j;
  while (true){
    for(j=0; j<256; j++) {
      for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel((i+j) & 255));
      }
      strip.show();
      if (delayAndCheckMusic(wait)) return;
      setBrightness(MAX_BRIGHTNESS);
    }
  }
}

void fire(){
  uint8_t r = 255;
  uint8_t g = 120;
  uint8_t b = 35;
  while (true){
    for(int i=BOTTOM_PIXEL_END; i<SIDE_PIXEL_END; i++){
      int flicker = random(0,150);
      int r1 = max(0, r-flicker);
      int g1 = max(0, g-flicker);
      int b1 = max(0, b-flicker);
      strip.setPixelColor(i,r1,g1, b1);
    }
    strip.show();
    if (delayAndCheck(random(50,150))) return;
  }
}

void bigPixels(uint32_t color){
  uint32_t rgbColors[3] = {
    strip.Color((uint8_t) (color >> 16), 0 , 0),
    strip.Color(0 , (uint8_t) (color >>  8) , 0),
    strip.Color(0 , 0, (uint8_t) color)
  };
  lightsOn = true;
  setBrightness(MAX_BRIGHTNESS);
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, rgbColors[i % 3]);
  }
  return;
}

void threeZone(uint32_t bottom, uint32_t side, uint32_t top) {
  lightsOn = true;
  setBrightness(MAX_BRIGHTNESS);
  for(int i=0; i<strip.numPixels(); i++) {
    if (i < BOTTOM_PIXEL_END){
      strip.setPixelColor(i, bottom);
    } else if (i < SIDE_PIXEL_END){
      strip.setPixelColor(i, side);
    } else {
      strip.setPixelColor(i, top);
    }
  }
  strip.show();
  return;
}

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } 
  else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } 
  else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void colorWipe(uint8_t wait, uint32_t c) {
  wait = wait ? wait : 20;
  uint16_t i;
  while (true){
    for(i = 0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      if (delayAndCheck(wait)) return;
    }
  }
}

void strobe(uint8_t ratePerSecond, uint32_t color){
  ratePerSecond = ratePerSecond ? ratePerSecond : 50;
  ratePerSecond = max(ratePerSecond, 1);
  int sleepTime = 1000.0 / (ratePerSecond * 2);
  int timePerCycle = 1000.0 / ratePerSecond;
  while (true){
    showColor(color);
    if(delayAndCheck(sleepTime)) return;
    ledsOffFast();
    if(delayAndCheck(sleepTime)) return;
  }
}

void rainbow(uint8_t wait) {
  uint16_t i;
  uint16_t j;
  wait = wait ? wait : 20;
  while (true){
    for(j = 0; j<256; j++) {
      for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel((i+j) & 255));
      }
      strip.show();
      if (delayAndCheck(wait)) return;
    }
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i;
  uint16_t j;
  wait = wait ? wait : 20;
  while (true){
    for(j = 0; j<256*5; j++) { // 5 cycles of all colors on wheel
      for(i=0; i< strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      }
      strip.show();
      if (delayAndCheck(wait)) return;
    }
  }
}

void theaterChase(uint8_t wait, uint32_t c) {
  wait = wait ? wait : 20;
  while (true){
    for (int j = 0; j<10; j++) {  //do 10 cycles of chasing
      for (int q = 0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, c);    //turn every third pixel on
        }
        strip.show();
        if (delayAndCheck(wait)) return;
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
      }
    }
  }
}

void theaterChaseRainbow(uint8_t wait) {
  wait = wait ? wait : 20;
  while (true){
    for (int j = 0; j < 256; j++) {     // cycle all 256 colors in the wheel
      for (int q = 0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
        if (delayAndCheck(wait)) return;
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
      }
    }
  }
}

void breathing(uint8_t delayTime, uint32_t color){
  delayTime = delayTime ? delayTime : 20;
  delayTime = min(100, delayTime);
  while (true){
    fadeOut(delayTime);
    if(Serial.available() > 0){
      return;
    }
    setColor(color);
    fadeIn(delayTime);
    if(Serial.available() > 0){
      return;
    }
  }
}

void breathing(uint8_t delayTime, uint32_t *colors, int size){
  delayTime = delayTime ? delayTime : 20;
  delayTime = min(100, delayTime);
  while (true){
    for(int i = 0; i < size; i++){
      fadeOut(delayTime);
      if(Serial.available() > 0){
        return;
      }
      setColor(colors[i]);
      fadeIn(delayTime);
      if(Serial.available() > 0){
        return;
      }
    }
  }
}

void rainbowBreathing(uint8_t delayTime){
  delayTime = delayTime ? delayTime : 20;
  delayTime = min(100, delayTime);
  while (true){
    for(int i = 0; i < 255; i += 30){
      fadeOut(delayTime);
      if(Serial.available() > 0){
        return;
      }
      setColor(Wheel(i));
      fadeIn(delayTime);
      if(Serial.available() > 0){
        return;
      }
    }
  }
}

void wave(uint8_t delayTime, int width, uint32_t color){
  uint8_t
      r = (uint8_t)(color >> 16),
      g = (uint8_t)(color >>  8),
      b = (uint8_t) color;
  setBrightness(MAX_BRIGHTNESS);
  delayTime = delayTime ? delayTime : 20;
  width = width ? width : 10;
  while (true){
    for(int i = 0; i<strip.numPixels(); i++) {
      for(int j = 0; j <strip.numPixels();  j++){
        strip.setPixelColor(j, (r * 25) >> 8, (g * 25)  >> 8, (b * 25) >> 8);
      }
      for(int j = max(0, i - (width/2 - 3)); j < min(strip.numPixels(), i + (width/2 + 3));  j++){
        if (j < (i - width/2) || j > (i + width/2)){
          int brightness = MAX_BRIGHTNESS/2;
          int diff = abs((i - width/2) - j);
          strip.setPixelColor(j, (r * brightness - diff) >> 8, (g * brightness - diff)  >> 8, (b * brightness - diff) >> 8);
        }
        strip.setPixelColor(j, color);
      }
      strip.show();
      if (delayAndCheck(delayTime)) return;
    }
  }
}

void dither(uint8_t delayTime, uint32_t color) {

  // Determine highest bit needed to represent pixel index
  int hiBit = 0;
  int n = strip.numPixels() - 1;
  for(int bit=1; bit < 0x8000; bit <<= 1) {
    if(n & bit) hiBit = bit;
  }

  int bit, reverse;
  for(int i=0; i<(hiBit << 1); i++) {
    // Reverse the bits in i to create ordered dither:
    reverse = 0;
    for(bit=1; bit <= hiBit; bit <<= 1) {
      reverse <<= 1;
      if(i & bit) reverse |= 1;
    }
    strip.setPixelColor(reverse, color);
    strip.show();
    delayAndCheck(delayTime);
  }
  delayAndCheck(250);
}

bool delayAndCheck(float delayTime){
  unsigned long waitTill =  millis() + delayTime;
  while (millis() <= waitTill){
    if (Serial.available()){
      return true;
    }
  }
  return false;
}

//---------------- Experimental ----------------//

bool delayAndCheckMusic(float delayTime){
  unsigned long waitTill =  millis() + delayTime;
  while (millis() <= waitTill){
    if (Serial.available()){
      return true;
    }
    bool shouldReturn = false;
    uint8_t count = 0; 
    while (!shouldReturn){
      makeReading();
      int slope = reading.new_reading - reading.last_reading;
      if(abs(slope) > noiseLevel) {
        threeZone(
          Wheel((int) ( min(255, (3.2 * (reading.new_reading - 60)) + (rand()  % 25) - (rand()  % 10)))),
          Wheel((int) ( (1.2 * (reading.new_reading - 60)) + (rand()  % 40) - (rand()  % 10)) %255),
          Wheel((int) ( min(255, (3.2 * (reading.new_reading - 60)) + (rand()  % 25) - (rand()  % 10)))));
        makeReading();
        count = 0;
      }
      else {
        if (count++ > 3){
          shouldReturn = true;
        }
        if (lightsOn){
          dimLights(-1);
        }
      }
      if(delayAndCheck(10)) return true;
    }
  }
  return false;
}

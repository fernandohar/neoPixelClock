#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_NeoPixel.h>

byte
  BRIGHTNESS = 5,
  BRIGHTNESS_MULTIPLIER = 32;
  
  
Adafruit_NeoPixel strip = Adafruit_NeoPixel(256, 6, NEO_GRB + NEO_KHZ800);
const uint32_t BLACK  = Adafruit_NeoPixel::Color(0 ,0, 0);

RTC_DS3231 rtc;


byte DIGIT1 = 255;
byte DIGIT2 = 255;
byte DIGIT3 = 255;
byte DIGIT4 = 255;
byte SECOND = 255;
uint32_t digit1Color, digit2Color, digit3Color, digit4Color, secondColor;
unsigned long previousMillis = 0;
int updateInterval = 1000;

uint16_t DIGIT_FONT1[3][10]={
  { //X = 0
    ((B11111111) << 8)+ B11000000,
    ((B11111111) << 8)+ B11000000,
    ((B10111111) << 8)+ B11000000,
    ((B10001110) << 8)+ B11000000,
    ((B10101110) << 8)+ B11000000,
    ((B10101110) << 8)+ B11000000,
    ((B10101010) << 8)+ B11000000,
    ((B10111110) << 8)+ B10000000,
    ((B11110110) << 8)+ B11000000,
    ((B11110110) << 8)+ B11000000
  },{ //X = 1
    ((B11110111) << 8)+ B11000000,
    ((B11110111) << 8)+ B11000000,
    ((B01000000) << 8)+ B00000000,
    ((B01000000) << 8)+ B00000000,
    ((B01110110) << 8)+ B10000000,
    ((B01110110) << 8)+ B11000000,
    ((B01001000) << 8)+ B01000000,
    ((B01001000) << 8)+ B00000000,
    ((B11110000) << 8)+ B10000000,
    ((B11110110) << 8)+ B11000000
  },{ //X = 2
    ((B10111111) << 8)+ B11000000,
    ((B10111111) << 8)+ B11000000,
    ((B10111011) << 8)+ B11000000,
    ((B10111001) << 8)+ B11000000,
    ((B10101111) << 8)+ B11000000,
    ((B10101111) << 8)+ B11000000,
    ((B10011111) << 8)+ B11000000,
    ((B10011111) << 8)+ B11000000,
    ((B11111111) << 8)+ B11000000,
    ((B11111111) << 8)+ B11000000
  }
};

byte currentSecondIndicator = 0;
//from X,Y coordinate to Strip index
byte getPixelIndex(byte x, byte y){
  byte index;
  if (y == 0){
     index = x;
    //index = 15 - x;
  }else if (y % 2 != 0){
    index = (y * 16 + 15) - x;
  }else{
    index = y * 16 + x;
  }
  return index;
}

//When e.g. passing arrays as parameters, the compiler no longer has the size of the array. Instead the array gets passed as a pointer, and in writeData the sizeof operation gets the size of the pointer and not the array (or whatever) it points to.
boolean elementInArray (const byte * array, size_t size, byte digit){
  byte i = -1; 
  byte iMax = size;
  while(++i < iMax){
  if(array[i] == digit){
    return true;
  }
  }
  return false;
}

void displaySecond(byte second, uint32_t color){//, boolean updatePosition){
  if (second <= 15){
    strip.setPixelColor( second, color); 
  }else if (second <= 30){
    strip.setPixelColor(getPixelIndex(15, second - 15), color);
  }else if (second <= 45){
    strip.setPixelColor(getPixelIndex( 15 - (second - 30), 15) , color);
  }else{
    strip.setPixelColor(getPixelIndex(0, 15 - (second - 45)) , color);
  }
  Serial.println(second);
}

void displayDigit(byte digit, byte offsetX, byte offsetY, uint32_t color){ //must call strip.show() else where to display the digit
  byte pixelIndex;
  for(int x = 0; x < 3; x++){
    for(int y = 0; y < 10; y++){
      if(bitRead(DIGIT_FONT1[x][y], 15 - digit)){
        pixelIndex = getPixelIndex(x + offsetX, y + offsetY);
        strip.setPixelColor( pixelIndex, color); 
      }
    }
  }
}

void updateClock(){
  fillScreen(BLACK);
  DateTime  now = rtc.now();
  uint8_t hour = now.hour();
  uint8_t minute = now.minute();
  
  byte tempDigit1  = hour / 10;
  byte tempDigit2 = hour % 10;
  byte tempDigit3 = minute / 10;
  byte tempDigit4 = minute %10;
  uint8_t tempSecond = now.second();
  if(hour > 6 && hour <= 9){
     
    setBrightness(4);
  }else if(hour <= 12){
    
    setBrightness(5);
  }else if(hour <= 15){
   
    setBrightness(6);
  }else if(hour <= 18){
   
    setBrightness(7);
  }else if (hour <= 21){
    
    setBrightness(2);
  }else{
    
    setBrightness(1);
  }
 
  if (tempDigit1 != DIGIT1){
  DIGIT1 = tempDigit1;
  digit1Color = getRandomColor(); 
  }
  if (tempDigit2 != DIGIT2){
  DIGIT2 = tempDigit2;
  digit2Color = getRandomColor(); 
  }
  if (tempDigit3 != DIGIT3){
  DIGIT3 = tempDigit3;
  digit3Color = getRandomColor(); 
  }
  if (tempDigit4 != DIGIT4){
  DIGIT4 = tempDigit4;
  digit4Color = getRandomColor(); 
  }
  
  displayDigit(DIGIT1, 0, 3, digit1Color);
  displayDigit(DIGIT2, 4, 3, digit2Color);
  displayDigit(DIGIT3, 9, 3, digit3Color);
  displayDigit(DIGIT4, 13, 3, digit4Color);
  
  if (tempSecond != SECOND){
    SECOND = tempSecond;
    secondColor = getRandomColor();
  }
  
  displaySecond(SECOND, secondColor);
  
  strip.show();
}

void setup() {
  Serial.begin(9600);
  strip.begin();
  setBrightness(BRIGHTNESS);
   if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    // This line sets the RTC with an explicit date & time
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
 
  for (int i = 0; i <= 9; ++i){
    fillScreen(BLACK);
    displayDigit(i, i, i,  getRandomColor());
    strip.show();
    delay(1000);
  }
  
  Serial.print("Setup completed");
}


uint32_t getRandomColor(){
  return Wheel((byte) random(0, 255)) ;
}


void setBrightness(int level){
  strip.setBrightness(  (level * BRIGHTNESS_MULTIPLIER) -1);
}

void fillScreen(const uint32_t color){
  for(int i = 0; i < 256; ++i){
    strip.setPixelColor( i, color);
  }
}

void loop() {
  unsigned long currentMillis = millis();
  if((currentMillis - previousMillis) > updateInterval ){
    previousMillis = currentMillis;
    //perform actual update
    updateClock();
  }
  strip.setPixelColor(0, getRandomColor());
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}






/*
    MXP-BOARD

    This code establishes the UART connection between the Arduino Nano mounted in the MXP board and the roboRIO.

    The MXP board will power the Arduino as soon as the roboRIO gets power. The Arduino should use this time to
    initialize three things:
    1) the serial port to the roboRIO
    2) the BNO055 sensor
    3) the NeoPixel LED strip

    The Arduino should then start reading from the BNO055 and initialize the NeoPixels. Once the Arduino reads
    data coming from the roboRIO (i.e. "START"), the Arduino will then start returning BNO055 sensor data back
    to the roboRIO.

    Created 2019
    By Nestor Sanchez

    This example code is in the public domain.
*/

/********************************/
/*  LIBRARIES                   */
/********************************/

#include <Wire.h>                         // I2C Arduino library
#include <Adafruit_Sensor.h>              // Universal Adafruit Sensor Interface
#include <Adafruit_BNO055.h>              // Adafruit API for BNO055
#include <utility/imumaths.h>             // Vector and math for IMU handling

#include <Adafruit_NeoPixel.h>            // Adafruit API for NeoPixels
#ifdef __AVR__                            
  #include <avr/power.h>                  // Supporting library for NeoPixel processing
#endif

/********************************/
/*  VARIABLES                   */
/********************************/

#define BNO055_SAMPLERATE_DELAY_MS 25     // Overall sensor sample rate
#define NEOPIXELDATAPIN   6               // Arduino pin D6 in MXP board
#define NEOPIXELCOUNT     60              // Number of NeoPixels in the strip

/* Constructors for sensor and LED strip objects */
Adafruit_BNO055 bno = Adafruit_BNO055();
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIXELCOUNT, NEOPIXELDATAPIN, NEO_GRB + NEO_KHZ800);

/* Helper variable for feedback LED */
bool userLED = false;

/********************************/
/*  SETUP                       */
/********************************/
void setup() {
  /* Start serial port */
  Serial.begin(115200);

  /* Initialize BNO055 */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.println("ERROR: Ooops, no BNO055 detected!");
    while(1);
  }
  delay(1000);

  /* Initialize BNO055 parameters */
  bno.setExtCrystalUse(true);

  /* Initialize NeoPixels */
  strip.begin();
  strip.setBrightness(90);
  strip.show();                   // Initialize all pixels to 'off' 

  /* Initialize built-in LED as an output */
  pinMode(LED_BUILTIN, OUTPUT);
}

/********************************/
/*  LOOP                        */
/********************************/
void loop() {

  /* Call NeoPixel routines, these include the call
   * for sensor data retrieval, serial and delay.
   */
  colorWipe(strip.Color(0, 0, 255));          // Blue
  colorWipe(strip.Color(255, 255, 0));        // Yellow
  colorWipe(strip.Color(255, 255, 255));      // White
  theaterChase(strip.Color(0, 0, 255));       // Blue
  theaterChase(strip.Color(255, 255, 0));     // Yellow
  theaterChase(strip.Color(255, 255, 255));   // White
}

/********************************/
/*  FUNCTIONS                   */
/********************************/

/* colorWipe
 *  
 * Fill the dots one after the other with a color.
 * 
 * @params
 * c: receives a 32-bit encoded RGB color
 */
void colorWipe(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    getBNO055Data();            // Instead of a delay we process sensor data here
  }
}

/* theaterChase
 *  
 * Theatre-style crawling lights.
 * 
 * @params
 * c: receives a 32-bit encoded RGB color
 */
void theaterChase(uint32_t c) {
  for (int j=0; j<10; j++) {                              //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);                      //turn every third pixel on
      }
      strip.show();

      getBNO055Data();        // Instead of a delay we process sensor data here

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);                      //turn every third pixel off
      }
    }
  }
}

/* getBNO055Data()
 * 
 * This function is used for reading the temperature and other
 * sensor data from the BNO055. It will format the data into
 * JSON and send via serial (UART) to the MXP interface. The 
 * delay is set to control timing.
 * In addition, the on-board user led is toggled for visual
 * feedback.
 * 
 * @params
 * No parameters are needed
 */
void getBNO055Data() {
  /* Get sensor data */
  int8_t temp = bno.getTemp();
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

  /* Send data via UART in JSON format */
  Serial.print("{\"Temp\":");
  Serial.print(temp);
  Serial.print(",\"X\":");
  Serial.print(euler.x());
  Serial.print(",\"Y\":");
  Serial.print(euler.y());
  Serial.print(",\"Z\":");
  Serial.print(euler.z());
  Serial.print("}\r\n");

  /* Loop delay */
  delay(BNO055_SAMPLERATE_DELAY_MS);

  /* Toggle feedback LED */
  if (userLED) digitalWrite(LED_BUILTIN, HIGH);
  else         digitalWrite(LED_BUILTIN, LOW);
  userLED = !userLED;
}

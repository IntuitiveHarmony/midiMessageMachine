
#include <Wire.h>
// #include <Adafruit_SSD1306.h>
#include <MIDI.h>

#include "SSD1306Ascii.h"        // for display
#include "SSD1306AsciiAvrI2c.h"  // for display
#define I2C_ADDRESS 0x3C         // display address 0X3C+SA0 - 0x3C or 0x3D
#define RST_PIN -1               // Define proper display RST_PIN if required.


#define encoderPinA 5  // Encoder Pins
#define encoderPinB 6


SSD1306AsciiAvrI2c oled;  // Instance of the display

MIDI_CREATE_DEFAULT_INSTANCE();


uint8_t messageCount = 1;  // Keep track of how many messages are on the screen
int startRow = 0;          // Variable to control the starting row for displaying messages
int aState;
int aLastState = 0;  // Keep track of any encoder movements




void setup() {
  Serial.begin(9600);
// Initialize display based on reset pin
#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else   // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif  // RST_PIN >= 0
  // Call oled.setI2cClock(frequency) to change from the default frequency.


  MIDI.begin(MIDI_CHANNEL_OMNI);

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  aLastState = digitalRead(encoderPinA); // Initial read of the encoder state

  oled.setFont(Adafruit5x7);  // Set font for display

  oled.set2X();
  oled.println(F("MIDI"));
  oled.set1X();
  oled.println(F("Message Machine"));

  delay(3000);
  oled.clear();
}

void loop() {

  Serial.print("startRow: ");
  Serial.println(startRow);

  aState = digitalRead(encoderPinA);
  if(aState != aLastState) {
    if (digitalRead(encoderPinB) != aState) {
      startRow++;
    } else {
      startRow--;
    }
    aLastState = aState;
  }

  // Check if there is any MIDI input
  if (MIDI.read()) {
    // Print the received MIDI message to the OLED display
    oled.print(messageCount);
    oled.print(": ");
    // Use bitwise OR to get the full status byte
    byte fullStatusByte = MIDI.getType() | MIDI.getChannel();
    oled.print(fullStatusByte, HEX);
    oled.print(" ");
    oled.print(MIDI.getData1(), HEX);
    oled.print(" ");
    oled.println(MIDI.getData2(), HEX);

    Serial.print("Status: 0x");
    Serial.print(MIDI.getType(), HEX);
    Serial.print(" Channel: ");
    Serial.print(MIDI.getChannel());
    Serial.print(" Data1: ");
    Serial.print(MIDI.getData1());
    Serial.print(" Data2: ");
    Serial.println(MIDI.getData2());

    messageCount++;
  }
  // Scroll up when the screen is filled
  if (messageCount >= 9) {
    oled.setScroll(1);
  }
}

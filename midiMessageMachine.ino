#include <Wire.h>
#include <MIDI.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

#define I2C_ADDRESS 0x3C
#define RST_PIN -1
#define encoderPinA 5
#define encoderPinB 6

SSD1306AsciiAvrI2c oled;

MIDI_CREATE_DEFAULT_INSTANCE();

const int maxMessages = 100;
struct MidiMessage {
  byte status;
  byte data1;
  byte data2;
};

MidiMessage midiMessages[maxMessages];  // Store MIDI messages so they can be recalled
int messageIndex = 0;

// For displaying stored messages
int displayedMessages = 0;
bool displayUpdated = false;
int startIndex = 0;

// Encoder Settings
int aState;
int aLastState = 0;

bool headerDisplayed = false;  // Header Flag

void setup() {
  Serial.begin(9600);

#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif

  MIDI.begin(MIDI_CHANNEL_OMNI);

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  aLastState = digitalRead(encoderPinA);

  oled.setFont(Adafruit5x7);

  oled.setRow(1);
  oled.println(F("INTUITIVE HARMONY"));
  oled.println(F(""));
  oled.set2X();
  oled.println(F("MIDI"));
  oled.set1X();
  oled.println(F("Message Machine"));

  delay(3000);
  oled.clear();
}

void loop() {
  Serial.print("startIndex: ");
  Serial.println(startIndex);
  Serial.print("messageIndex: ");
  Serial.println(messageIndex);
  Serial.print("displayedMessages: ");
  Serial.println(displayedMessages);

  // Handle encoder input
  handleEncoder();

  // Display MIDI messages below the header
  if (MIDI.read()) {
    // Store the received MIDI message in the array
    midiMessages[messageIndex].status = MIDI.getType() | MIDI.getChannel();
    midiMessages[messageIndex].data1 = MIDI.getData1();
    midiMessages[messageIndex].data2 = MIDI.getData2();

    // Limit to the max messages and reset if reached
    if (messageIndex < maxMessages) {
      messageIndex++;
    } else {
      messageIndex = 0;
      startIndex = 0;
    }
    // Limit Displayed Messages to 6
    if (displayedMessages < 7) {
      displayedMessages++;
    } else {
      displayedMessages = 0;
    }
  }
  // Check to see if the screen should be reset
  if (displayedMessages >= 7 && messageIndex > 0 && displayUpdated == false) {
    handleUpdateDisplay();
  }
  // hacky way to keep the display from flickering
  if (displayedMessages != 7 && messageIndex > 0) {
    displayUpdated = false;
  }


  if (!headerDisplayed) {
    printHeader(messageIndex);
  }
  // Print stored messages
  oled.setRow(2);
  printStoredMessages();
}

void handleEncoder() {
  aState = digitalRead(encoderPinA);
  if (aState != aLastState) {
    if (startIndex >= 0) {
      if (digitalRead(encoderPinB) != aState) {
        startIndex++;
      } else {
        if (startIndex > 0) {
          startIndex--;
        }
      }
    }
    aLastState = aState;
  }
}

// Will adjust the placement of the headers based on the number printed for the index (eventually)
void printHeader(int messageIndex) {
  // Print the header
  if (messageIndex < 10) {
    oled.println("   St D1 D2");
    oled.println("  ~~~~~~~~~~");
  } else if (messageIndex > 10) {
    oled.println("    St D1 D2");
    oled.println("   ~~~~~~~~~~");
  }
  headerDisplayed = true;
}

void printStoredMessages() {
  // Print the stored MIDI messages
  for (int i = startIndex; i < messageIndex; i++) {
    oled.print(i + 1);
    oled.print(": ");
    oled.print(midiMessages[i].status, HEX);
    oled.print(" ");
    oled.print(midiMessages[i].data1, HEX);
    oled.print(" ");
    oled.println(midiMessages[i].data2, HEX);
  }
}

// Clear display after 6 messages and print the next 6 messages
void handleUpdateDisplay() {
  oled.clear();
  startIndex = startIndex + 6;
  displayUpdated = true;
  headerDisplayed = false;  // reset header flag
  displayedMessages = 1;
}

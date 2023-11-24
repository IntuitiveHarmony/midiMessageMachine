#include <MIDI.h>
#include <Wire.h>
#include "SSD1306Ascii.h"  // https://github.com/greiman/SSD1306Ascii
#include "SSD1306AsciiAvrI2c.h"

#define I2C_ADDRESS 0x3C
#define RST_PIN -1
#define encoderPinA 5
#define encoderPinB 6

#define freezePin 7
#define freezeLED 8

SSD1306AsciiAvrI2c oled;

MIDI_CREATE_DEFAULT_INSTANCE();

bool freezeInput = false;

const int maxMessages = 300;
struct MidiMessage {
  byte status;
  byte data1;
  byte data2;
};

MidiMessage
    midiMessages[maxMessages];  // Store MIDI messages so they can be recalled
int messageIndex = 0;

// For displaying stored messages
int displayedMessages = 0;
bool displayUpdated = false;
int encoder = 0;
int startIndex = 0;

// Encoder Settings
int aState;
int aLastState = 0;

bool headerDisplayed = false;  // Header Flag

void setup() {
  Serial.begin(9600);

// Initialize display
#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif

  MIDI.begin(MIDI_CHANNEL_OMNI);  // Initialize MIDI

  // Setup pins
  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);

  pinMode(freezePin, INPUT_PULLUP);
  pinMode(freezeLED, OUTPUT);

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
  Serial.print("freeze: ");
  Serial.print(freezeInput);
  Serial.print(" encoder: ");
  Serial.print(encoder);
  Serial.print(" startIndex: ");
  Serial.println(startIndex);
  // Serial.print("messageIndex: ");
  // Serial.println(messageIndex);
  // Serial.print("displayedMessages: ");
  // Serial.println(displayedMessages);

  // Handle encoder input
  handleEncoder();

  // Handle input from freeze button
  handleFreeze();

  // Display MIDI messages below the header
  if (MIDI.read() && !freezeInput) {
    // Store the received MIDI message in the array
    midiMessages[messageIndex].status = MIDI.getType() | MIDI.getChannel();
    midiMessages[messageIndex].data1 = MIDI.getData1();
    midiMessages[messageIndex].data2 = MIDI.getData2();

    handleDisplayedMessageTabulation(1);

    // Limit to the max messages and reset if reached
    if (messageIndex < maxMessages) {
      messageIndex++;
    } else {
      messageIndex = 0;
      handleResetHeader();  // reset header after reseting the message array
      startIndex = 0;
      displayedMessages = 0;
    }
  }
  // Check to see if the screen should be reset
  if (displayedMessages >= 7 && messageIndex > 0 && displayUpdated == false) {
    handleResetHeader();
    startIndex = startIndex + 6;
    displayedMessages = 1;
  }
  // hacky way to reset the display after the first 6 notes
  if (displayedMessages != 7 && messageIndex > 0) {
    displayUpdated = false;
  }

  // print the header
  if (!headerDisplayed) {
    printHeader(messageIndex);
  }

  // Print stored messages
  oled.setRow(2);
  printStoredMessages();
}

void handleFreeze() {
  static bool buttonState = HIGH;  // HIGH means not pressed

  if (digitalRead(freezePin) == LOW && buttonState == HIGH) {
    // Button was pressed
    freezeInput = !freezeInput;  // Toggle freezeInput
    buttonState = LOW;  // Update buttonState to indicate button is pressed
  } else if (digitalRead(freezePin) == HIGH && buttonState == LOW) {
    // Button was released
    buttonState = HIGH;  // Update buttonState to indicate button is not pressed
  }
}

void handleEncoder() {
  // Read the current state of the A channel of the encoder
  aState = digitalRead(encoderPinA);
  // Check if the state of the A channel has changed
  if (aState != aLastState) {
    // Ensure encoder is non-negative
    if (encoder >= 0) {
      // Check if the B channel is different from the current A channel state
      // (encoder turned clockwise)
      if (digitalRead(encoderPinB) != aState) {
        // Increment encoder
        encoder++;
      }
      // Encoder turned counterclockwise
      else {
        // Check if encoder is greater than 0
        if (encoder > 0) {
          // Decrement encoder
          encoder--;
        }
      }
    }
    aLastState = aState;
  }
  // Encoder will increment by two, this will increment the startIndex by one
  // based off of the encoder
  if (encoder < 0) {
    encoder = 0;
  } else {
    encoder = encoder / 2;
  }
}

// Will adjust the placement of the headers based on the number printed for the
// message index
void printHeader(int messageIndex) {
  // Print the header
  if (messageIndex < 10) {
    oled.println("   ST D1 D2");
    oled.println("  ~~~~~~~~~~");
  } else if (messageIndex > 100) {
    oled.println("     ST D1 D2");
    oled.println("    ~~~~~~~~~~");
  } else if (messageIndex > 10) {
    oled.println("    ST D1 D2");
    oled.println("   ~~~~~~~~~~");
  }
  headerDisplayed = true;
}

void printStoredMessages() {
  // Print the stored MIDI messages
  for (int i = startIndex; i < messageIndex; i++) {
    // highlight most current message
    if (i + 1 == messageIndex) {
      oled.setInvertMode(1);
    }
    oled.print(i + 1);
    oled.print(": ");
    oled.print(midiMessages[i].status, HEX);
    oled.print(" ");
    oled.print(midiMessages[i].data1, HEX);
    oled.print(" ");
    oled.println(midiMessages[i].data2, HEX);
  }
  // reset highlight
  oled.setInvertMode(0);
}

// Clear display after 6 messages and print the next 6 messages
void handleResetHeader() {
  oled.clear();
  displayUpdated = true;
  headerDisplayed = false;  // reset header flag
}

void handleDisplayedMessageTabulation(int increment) {
  // Limit Displayed Messages to 6
  if (displayedMessages < 7) {
    displayedMessages += increment;
  } else {
    displayedMessages = 0;
  }
}

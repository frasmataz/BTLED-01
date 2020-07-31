#include <SoftwareSerial.h>

SoftwareSerial hc05(3,2); // RX, TX pins

const int BLUE_PIN = 9;
const int RED_PIN = 10;
const int GREEN_PIN = 11;

const int TIMEOUT = 2000;

struct Colour {
  int r;
  int g;
  int b;

  Colour(int r_, int g_, int b_) {
    r = r_;
    g = g_;
    b = b_;
  }
};


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  hc05.begin(9600);
  
  log_("Starting up");
  test_leds();
  log_("Setup done");
}

void loop() {
  if(hc05.available()) {
    char startByte = hc05.read();
    if(startByte == '#') {
      log_("Caught start byte");

      // Get command byte
      if (pauseForNBytes(1)) {
        char command = hc05.read();

        // Run command
        if (command == '1') {
          // Set single colour
          // Takes three ints
          log_("Set colour");
          if (hc05.available() == 3) {
            int r_ = hc05.read();
            int g_ = hc05.read();
            int b_ = hc05.read();
            log_("Setting " + String(r_) + " " + String(g_) + " " + String(b_));

            setLeds(Colour(r_, g_, b_));
          } else {
            log_("incorrect " + String(hc05.available()) + " bytes available");
          }
        } else {
          log_("Unknown command: " + command);
        }
      }
      
    } else {
      log_("Expected start byte, got " + startByte);
      hc05.flush();
    }
    log_("Done");
  }
}

boolean pauseForNBytes(int minBytes) {
  // Give time for next bytes to come in
  int timeout = millis() + TIMEOUT;
  while(hc05.available() < minBytes) {
    if(millis() < timeout) {
      delay(50);
    } else {
      log_("Timeout");
      return false;
    }
  }

  return true;
}

void test_leds() {
  test_colour("red", Colour(255, 0, 0));
  test_colour("green", Colour(0, 255, 0));
  test_colour("blue", Colour(0, 0, 255));
  test_colour("magenta", Colour(255, 0, 255));
  test_colour("cyan", Colour(0, 255, 255));
  test_colour("yellow", Colour(255, 255, 0));
  test_colour("white", Colour(255, 255, 255));
}

void test_colour(String name, Colour colour) {
  log_("test " + name);
  setLeds(colour);
  delay(100);
}

void setLeds(Colour colour) {
  analogWrite(RED_PIN, colour.r);
  analogWrite(GREEN_PIN, colour.g);
  analogWrite(BLUE_PIN, colour.b);
}

void log_(String message) {
  Serial.println(message);
  //hc05.println(message);
}


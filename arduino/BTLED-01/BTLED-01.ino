#include <SoftwareSerial.h>

SoftwareSerial hc05(2,3); // RX, TX pins

struct Colour {
  int r;
  int g;
  int b;
  long duration;

  Colour() {
    r = 0;
    g = 0;
    b = 0;
    duration = 0;
  }

  Colour(int r_, int g_, int b_) {
    r = r_;
    g = g_;
    b = b_;
    duration = 0;
  }

  Colour(int r_, int g_, int b_, long duration_) {
    r = r_;
    g = g_;
    b = b_;
    duration = duration_;
  }
};

const int BLUE_PIN = 9;
const int RED_PIN = 10;
const int GREEN_PIN = 11;

const int TIMEOUT = 2000;
enum State { IDLE, SOLID_COLOUR, FADE_LIST };
const int FADE_LIST_MAX_SIZE = 10;

Colour prevColour = Colour(0,0,0);
long animationStartTime = 0;
long animationEndTime = 0;
Colour fadeList[FADE_LIST_MAX_SIZE];
int fadeListIndex = 0;
int fadeListSize = 0;
State state = IDLE;

void setup() {
  Serial.begin(9600);
  hc05.begin(9600);
  
  log_("Starting up");
  test_leds();
  log_("Setup done");
}

void loop() {
  getCommand();
  progressAnimations();
}

void progressAnimations() {
  if (state == FADE_LIST) {
    if (millis() < animationEndTime) {
      setLeds( interpolateColour(
         fadeList[fadeListIndex],
         fadeList[(fadeListIndex + 1) % fadeListSize],
         float(millis() - animationStartTime) / float(animationEndTime - animationStartTime)
      ) );
    } else {
      fadeListIndex = (fadeListIndex + 1) % fadeListSize;
      setLeds(fadeList[fadeListIndex]);
      animationStartTime = millis();
      animationEndTime = animationStartTime + fadeList[fadeListIndex].duration;
    }
  }
}

void getCommand() {
  if(hc05.available()) {
    log_("command");
    char startByte = hc05.read();
    if(startByte == '#') {
      log_("Caught start byte");

      // Get command byte
      if (pauseForNBytes(1)) {
        char command = hc05.read();

        // Run command
        if (command == '1') {
          setSingleColour();
        } else if (command == '2') {
          setFadeList();
        } else {
          log_("Unknown command: " + command);
        }
      }
      
    } else {
      log_("Expected start byte, got " + startByte);
      hc05.flush();
    }
    log_("Done");
    hc05.flush();
  }
}

void setSingleColour() {
  // Set single colour
  // Takes three ints: R, G, B
  log_("Set colour");
  if (hc05.available() == 3) {
    int r_ = hc05.read();
    int g_ = hc05.read();
    int b_ = hc05.read();
    log_("Setting " + String(r_) + " " + String(g_) + " " + String(b_));

    setLeds(Colour(r_, g_, b_));
    state = SOLID_COLOUR;
  } else {
    log_("only " + String(hc05.available()) + " bytes available");
  }
}

void setFadeList() {
  // Fade list
  // Takes groups of four bytes:
  // R, G, B, followed by either a ';' if there is more to come, or any other byte to end.
  log_("Fade list");

  int i = 0;
  boolean continue_ = true;

  // Read in list of colours
  while (continue_ && i < FADE_LIST_MAX_SIZE) {
    pauseForNBytes(4);
    int r_ = hc05.read();
    int g_ = hc05.read();
    int b_ = hc05.read();
    long duration = hc05.read() * 100; // Byte in tenths of a second
    int signal = hc05.read();

    log_("Adding " + String(r_) + " " + String(g_) + " " + String(b_) + " " + String(duration));
    fadeList[i] = Colour(r_, g_, b_, duration);
    setLeds(Colour(r_, g_, b_));
    continue_ = (signal == 7 ? true : false);
    log_(String(continue_));
    i++;
  }

  fadeListSize = i;
  fadeListIndex = 0;
  animationStartTime = millis();
  animationEndTime = 0;
  state = FADE_LIST;
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

long interpolate(long minVal, long maxVal, float scale) {
  //scale is how far between minVal and maxVal to interpolate, between 0.0 and 1.0
  return minVal + ( (maxVal - minVal) * scale );
}

Colour interpolateColour(Colour colour1, Colour colour2, float scale) {
  return Colour(
    interpolate(colour1.r, colour2.r, scale),
    interpolate(colour1.g, colour2.g, scale),
    interpolate(colour1.b, colour2.b, scale)
  );
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
  prevColour = colour;
}

void log_(String message) {
  Serial.println(message);
  //hc05.println(message);
}

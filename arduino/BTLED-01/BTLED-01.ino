#include <SimplexNoise.h>
#include <SoftwareSerial.h>

SoftwareSerial hc05(2,3); // RX, TX pins
SimplexNoise sn;

struct Colour {
  int r;
  int g;
  int b;
  long duration;
  int flickerAmplitude;
  int flickerFrequency;

  Colour() {
    r = 0;
    g = 0;
    b = 0;
    duration = 0;
    flickerAmplitude = 0;
    flickerFrequency = 0;
  }

  Colour(int r_, int g_, int b_) {
    r = r_;
    g = g_;
    b = b_;
    duration = 0;
    flickerAmplitude = 0;
    flickerFrequency = 0;
  }

  Colour(int r_, int g_, int b_, long duration_, int flickerAmplitude_, int flickerFrequency_) {
    r = r_;
    g = g_;
    b = b_;
    duration = duration_;
    flickerAmplitude = flickerAmplitude_;
    flickerFrequency = flickerFrequency_;
  }
};

const int BLUE_PIN = 9;
const int RED_PIN = 10;
const int GREEN_PIN = 11;

const int TIMEOUT = 2000;
enum State { IDLE, SOLID_COLOUR, FADE_LIST };
const int FADE_LIST_MAX_SIZE = 8;

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
      Colour nextColour = interpolateColour(
         fadeList[fadeListIndex],
         fadeList[(fadeListIndex + 1) % fadeListSize],
         float(millis() - animationStartTime) / float(animationEndTime - animationStartTime)
        );

      setLeds( applyFlickerEffect( nextColour ));
      prevColour = nextColour;
    } else {
      fadeListIndex = (fadeListIndex + 1) % fadeListSize;
      setLeds(applyFlickerEffect(fadeList[fadeListIndex]));
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
    pauseForNBytes(7);
    int r_ = hc05.read();
    int g_ = hc05.read();
    int b_ = hc05.read();
    long duration = hc05.read() * 100; // Byte in tenths of a second
    int flickerAmplitude = hc05.read();
    int flickerFrequency = hc05.read();
    int signal = hc05.read();
    Colour colour = Colour(r_, g_, b_, duration, flickerAmplitude, flickerFrequency);
    //logColour(colour);
    fadeList[i] = colour;
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

void logColour(Colour c) {
  String msg = "";
  msg += "=====";
  msg += "Red: " + String(c.r);
  msg += "Green: " + String(c.g);
  msg += "Blue: " + String(c.b);
  msg += "=====;";
  Serial.println(msg);

  msg = "";
  msg += "Duration: " + String(c.duration);
  msg += "Amp: " + String(c.flickerAmplitude);
  msg += "Freq: " + String(c.flickerFrequency);
  msg += "=====;";
  Serial.println(msg);
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
    interpolate(colour1.b, colour2.b, scale),
    colour1.duration,
    colour1.flickerAmplitude,
    colour1.flickerFrequency
  );
}

Colour applyFlickerEffect(Colour input) {
  float frequency = input.flickerFrequency / 100;
  int amplitude = input.flickerAmplitude;

  Serial.println("amp: " + String(amplitude) + " freq: " + String(frequency));

  float m = millis();
  float x = m / 1000.0f * frequency;

  float offset = (sn.noise(x, 0.5f) * amplitude) - (amplitude / 2);
  offset += (sn.noise(x*2, 1.5f) * amplitude/2) - (amplitude / 4);
  offset += (sn.noise(x*4, 2.5f) * amplitude/4) - (amplitude / 8);
  offset += (sn.noise(x*8, 3.5f) * amplitude/8) - (amplitude / 16);

  return Colour(
      max(min(input.r + (offset * (input.r/255.0f)), 255), 0),
      max(min(input.g + (offset * (input.g/255.0f)), 255), 0),
      max(min(input.b + (offset * (input.b/255.0f)), 255), 0),
      input.duration,
      input.flickerAmplitude,
      input.flickerFrequency
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
}

void log_(String message) {
  Serial.println(message);
  //hc05.println(message);
}

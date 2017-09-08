// This sketch reads from a pulse sensor
//
// https://pulsesensor.com/
// https://www.adafruit.com/product/1093
//
// By analysing the signal for peaks it aims to detect heartbeats.
// An LED flashes whenever a heartbeat is detected.


// Pulse Sensor PURPLE WIRE connected to ANALOG PIN 0
#define MICROPHONE_PIN A0
//#define LAMP_PIN D2


// Samples per second
#define SAMPLING_RATE 1000

// Upper limit of blink frequency
#define MIN_MILLIS_BETWEEN_BEATS 190

// Ensure that blinks never overlap (and that we ignore the audible click made by the closing relay)
#define BLINK_DURATION (MIN_MILLIS_BETWEEN_BEATS-20)


float rawInput;              // Current input, assumed to be a rectified wave [0..<1023]
float envSlow;               // Envelope over positive peaks (slow decay)
float envMedium;             // Envelope over positive peaks (medium decay)
float envFast;               // Envelope over positive peaks (fast decay)

unsigned long millisOfLastBeat;

static unsigned long frameCount;

bool envFastHasDippedSinceLastBlink;


void setup() {
  pinMode(MICROPHONE_PIN, INPUT);
  //  pinMode(LED_BUILTIN, OUTPUT);
  //  pinMode(LAMP_PIN, OUTPUT);
  //    Serial.begin(9600);

  Serial.begin(115200);
}



void readInput() {
  rawInput = analogRead(MICROPHONE_PIN);
  envSlow   = maxf((envSlow * 0.9995), rawInput);
  envMedium = maxf((envMedium * 0.999), rawInput);
  envFast   = maxf((envFast * 0.995), rawInput);
}


void loop() {
  readInput();


  if (++frameCount % 10 == 0) {
    //        Serial.println(rawInput);
      Serial.println(envSlow);
    // Serial.println(envMedium);
      Serial.println(envFast);
  }


  envFastHasDippedSinceLastBlink = envFastHasDippedSinceLastBlink || envFast < envMedium;

  if (envSlow > 40 &&
      envFast > envSlow * 0.9 &&
      envFastHasDippedSinceLastBlink &&
      hasEnoughTimePassedSinceTheLastDetectedBeat()) {
    registerBeat();
  }
  bool lampState = millisOfLastBeat > millis() - BLINK_DURATION ? HIGH : LOW;
  //  digitalWrite(LED_BUILTIN, lampState);
  //  digitalWrite(LAMP_PIN, lampState);

  delayMicroseconds(1000000 / SAMPLING_RATE);
}


void registerBeat() {
  millisOfLastBeat = millis();
  envFastHasDippedSinceLastBlink = false;
  Serial.println(1000);
}


bool hasEnoughTimePassedSinceTheLastDetectedBeat() {
  return millis() - MIN_MILLIS_BETWEEN_BEATS > millisOfLastBeat; // http://black-electronics.com/blog/worried-about-millis-timer-overflow
}


float maxf(float a, float b) {
  return a > b ? a : b;
}


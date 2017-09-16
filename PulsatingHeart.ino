// This sketch reads from a pulse sensor
//
// https://pulsesensor.com/
// https://www.adafruit.com/product/1093
//
// By analysing the signal for peaks it aims to detect heartbeats.
// An LED flashes whenever a heartbeat is detected.


// Pulse Sensor PURPLE WIRE connected to ANALOG PIN 0
#define AUDIO_PIN A0
#define PULSE_PIN A4
#define LAMP_PIN 6


// Samples per second
#define SAMPLING_RATE 1000

// Upper limit of blink frequency
#define MIN_MILLIS_BETWEEN_BEATS 190

// Ensure that blinks never overlap (and that we ignore the audible click made by the closing relay)
#define BLINK_DURATION (MIN_MILLIS_BETWEEN_BEATS-20)

float rawAudioInput;
float rawPulseInput;
float rawInput;              // Current input, assumed to be a rectified wave [0..<1023]
float envAudioPresence;          // Envelope over positive audio peaks (slow decay) - to test whether audio is coming in


float envPulsePresenceUpper;
float envPulsePresenceLower;

float envSlow;               // Envelope over positive signal peaks (slow decay)
float envMedium;             // Envelope over positive signal peaks (medium decay)
float envFast;               // Envelope over positive signal peaks (fast decay)

unsigned long millisOfLastBeat;

static unsigned long frameCount;

bool envFastHasDippedSinceLastBlink;

bool lampState;


void setup() {
  pinMode(AUDIO_PIN, INPUT);
  pinMode(LAMP_PIN, OUTPUT);
  //  Serial.begin(9600);

//  Serial.begin(115200);
}



void readInput() {
  rawAudioInput = analogRead(AUDIO_PIN);
  envAudioPresence   = maxf(envAudioPresence * 0.998, rawAudioInput);

  rawPulseInput = analogRead(PULSE_PIN);
  envPulsePresenceUpper   = maxf(envPulsePresenceUpper * 0.9995, rawPulseInput);
  envPulsePresenceLower   = minf(envPulsePresenceLower + 0.3, rawPulseInput);

  rawInput = haveAudioSignal() ? rawAudioInput : rawPulseInput; // use audio if available, otherwise use pulse
  envSlow   = maxf(envSlow * 0.9995, rawInput);
  envMedium = maxf(envMedium * 0.999, rawInput);
  envFast   = maxf(envFast * 0.995, rawInput);
}


void loop() {
  readInput();

  if (++frameCount % 10 == 0) {
    //  Serial.println(rawInput);
//          Serial.println(rawPulseInput);
    //          Serial.println(envSlow);
    //          Serial.println(envAudioPresence);
    //          Serial.println(haveAudioSignal());
    //            Serial.println(havePulseSignal());
    //            Serial.println();
    //       Serial.println(envPulsePresenceUpper);
    //       Serial.println(envPulsePresenceLower);
    //       Serial.println(envPulsePresenceUpper - envPulsePresenceLower);
    //      Serial.println(envFast);
//  Serial.println(shouldLampBeOn() * 100);
  }

  digitalWrite(LAMP_PIN, shouldLampBeOn() ? HIGH : LOW);


  delayMicroseconds(1000000 / SAMPLING_RATE);
}


void registerBeat() {
  millisOfLastBeat = millis();
  envFastHasDippedSinceLastBlink = false;
  //    Serial.println(1000);
}


bool hasEnoughTimePassedSinceTheLastDetectedBeat() {
  return millis() - MIN_MILLIS_BETWEEN_BEATS > millisOfLastBeat; // http://black-electronics.com/blog/worried-about-millis-timer-overflow
}


float maxf(float a, float b) {
  return a > b ? a : b;
}

float minf(float a, float b) {
  return a < b ? a : b;
}

bool haveAudioSignal() {
  return envAudioPresence > 20;
}

bool havePulseSignal() {
  return envPulsePresenceUpper - envPulsePresenceLower > 60;
}

bool haveAnyInputSignal() {
  return haveAudioSignal() || havePulseSignal();
}

bool shouldLampBeOn() {
  if (haveAnyInputSignal()) {
    envFastHasDippedSinceLastBlink = envFastHasDippedSinceLastBlink || envFast < envMedium;
    if (envSlow > 20 &&
        envFast > envSlow * 0.9 &&
        envFastHasDippedSinceLastBlink &&
        hasEnoughTimePassedSinceTheLastDetectedBeat()) {
      registerBeat();
    }
    return millisOfLastBeat > millis() - BLINK_DURATION;
  } else {
    return millis() % 1200 < 380;
  }
}

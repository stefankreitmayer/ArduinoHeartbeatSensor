// This sketch reads from a pulse sensor
//
// https://pulsesensor.com/
// https://www.adafruit.com/product/1093
//
// By analysing the signal for peaks it aims to detect heartbeats.
// An LED flashes whenever a heartbeat is detected.


// Pulse Sensor PURPLE WIRE connected to ANALOG PIN 0
#define PULSE_SENSOR_PIN 0
#define LED_PIN LED_BUILTIN

// Samples per second
#define SAMPLING_RATE 40

// Milliseconds per sample
#define SAMPLING_DELAY 1000 / SAMPLING_RATE

// Upper heart rate limit
#define MIN_MILLIS_BETWEEN_BEATS 280


int rawInput;                   // Incoming raw data. Signal value can range from 0-1023
int previousRawInput;           // Used for DC filter
int previousFilteredInput;      // Used for DC filter
int filteredInput;              // The pulse signal with the DC offset removed

int peakEnvelope;               // Remembers the amplitude of the most recent peak (slow decay)

unsigned long millisOfLastBeat;
unsigned long millisOfLastBlink;


void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(9600);
}


void loop() {
  readInput();

  peakEnvelope = max((int)(peakEnvelope * 0.99), filteredInput);
  if (peakEnvelope > 20 &&
      hasEnoughTimePassedSinceTheLastDetectedBeat() &&
      isPeakInSignal()) {
    registerBeat();
    triggerBlink();
  }
  digitalWrite(LED_PIN, millisOfLastBlink > millis() - 200 ? HIGH : LOW);

  delay(SAMPLING_DELAY);
}


void readInput() {
  // raw input
  previousRawInput = rawInput;
  rawInput = analogRead(PULSE_SENSOR_PIN);

  // filtered input
  previousFilteredInput = filteredInput;
  filteredInput = (int) (rawInput - previousRawInput + 0.8 * previousFilteredInput); // remove DC offset
}


bool isPeakInSignal() {
  return previousFilteredInput > peakEnvelope * 0.8 &&
         filteredInput < previousFilteredInput;
}


void registerBeat() {
  millisOfLastBeat = millis();
}


bool triggerBlink() {
  millisOfLastBlink = millis();
}


bool hasEnoughTimePassedSinceTheLastDetectedBeat() {
  return millis() - MIN_MILLIS_BETWEEN_BEATS > millisOfLastBeat; // http://black-electronics.com/blog/worried-about-millis-timer-overflow
}


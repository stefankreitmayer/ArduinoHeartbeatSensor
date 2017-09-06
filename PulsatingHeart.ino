// This sketch reads from a pulse sensor
//
// https://pulsesensor.com/
// https://www.adafruit.com/product/1093
//
// By analysing the signal for peaks it aims to detect heartbeats.
// An LED flashes whenever a heartbeat is detected.


// Pulse Sensor PURPLE WIRE connected to ANALOG PIN 0
#define MICROPHONE_PIN A0
#define LAMP_PIN D2


// Samples per second
#define SAMPLING_RATE 24000

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
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LAMP_PIN, OUTPUT);
  Serial.begin(9600);
}


void loop() {
  readInput();
  //  Serial.println(rawInput);
  //  Serial.println(filteredInput);

  peakEnvelope = max((int)(peakEnvelope * 0.999), filteredInput);
  //  Serial.println(peakEnvelope);

  if (peakEnvelope > 200 &&
      hasEnoughTimePassedSinceTheLastDetectedBeat() &&
      isPeakInSignal()) {
    registerBeat();
    triggerBlink();
  }
  bool lampState = millisOfLastBlink > millis() - 150 ? LOW : HIGH;
  digitalWrite(LED_BUILTIN, lampState);
  digitalWrite(LAMP_PIN, lampState);

  delay(SAMPLING_DELAY);
}


void readInput() {
  // raw input
  previousRawInput = rawInput;
  rawInput = analogRead(MICROPHONE_PIN);

  // filtered input
  previousFilteredInput = filteredInput;
  filteredInput = rawInput;
  //  filteredInput = (int) (rawInput - previousRawInput + 0.8 * previousFilteredInput); // remove DC offset
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
  Serial.println("peak!");
}


bool hasEnoughTimePassedSinceTheLastDetectedBeat() {
  return millis() - MIN_MILLIS_BETWEEN_BEATS > millisOfLastBeat; // http://black-electronics.com/blog/worried-about-millis-timer-overflow
}


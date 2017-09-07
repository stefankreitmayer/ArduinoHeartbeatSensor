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
#define SAMPLING_RATE 1000

// Milliseconds per sample
#define SAMPLING_DELAY 1000 / SAMPLING_RATE

// Upper limit of blink frequency
#define MIN_MILLIS_BETWEEN_BEATS 360

// Ensure that blinks never overlap (and that we ignore the audible click made by the closing relay)
#define BLINK_DURATION (MIN_MILLIS_BETWEEN_BEATS-20)


float rawInput;                   // Incoming raw data. Signal value can range from 0-1023
float previousRawInput;           // Used for DC filter
float previousFilteredInput;      // Used for DC filter
float filteredInput;              // The pulse signal with the DC offset removed
float amplitude;

float peakEnvelope;               // Remembers the amplitude of the most recent peak (slow decay)

unsigned long millisOfLastBeat;
unsigned long millisOfLastBlink;

static unsigned long frameCount;


void setup() {
  pinMode(MICROPHONE_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LAMP_PIN, OUTPUT);
    Serial.begin(9600);

//  Serial.begin(115200);
}


void loop() {
  readInput();
  peakEnvelope = maxf((peakEnvelope * 0.999), amplitude);


  if (++frameCount % 3 == 0) {
      Serial.println(rawInput);
   //    Serial.println(amplitude);
   //    Serial.println(filteredInputL);
//    Serial.println(peakEnvelope);
  }

  if (peakEnvelope > 40 &&
      hasEnoughTimePassedSinceTheLastDetectedBeat() &&
      isPeakInSignal()) {
    registerBeat();
    triggerBlink();
  }
  bool lampState = millisOfLastBlink > millis() - BLINK_DURATION ? HIGH : LOW;
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
  filteredInput = (int) (rawInput - previousRawInput + 0.99999 * previousFilteredInput); // remove DC offset

  amplitude = abs(filteredInput);
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


float maxf(float a, float b) {
  return a > b ? a : b;
}


const int inputPin = 0;               // Pulse Sensor PURPLE WIRE connected to ANALOG PIN 0
const int ledPin = LED_BUILTIN;                // The on-board Arduion LED

const int SAMPLING_RATE = 20;   // Hz
const int SAMPLE_DURATION = 1000 / SAMPLING_RATE; // ms

const int BUFSIZE = 50;         // Depends on update rate
const int halfBufsize = BUFSIZE / 2; // convenience variable

int rawInput;                   // Holds the incoming raw data. Signal value can range from 0-1024
int previousRawInput;           // Used for DC filter
int filteredInput;              // Used for DC filter
int previousFilteredInput;      // Used for DC filter

long buf[BUFSIZE];              // Used for autocorrelation
long correlation[halfBufsize];
int tauStart = BUFSIZE / 6;     // No need to start from 0 because we can ignore very high frequencies
int writeIndexInBuf = 0;

float pulseFrequency;           // Hz

float deltaEnvelope;            // Used for detecting heartbeat onset

float blinkPhase;               // [0...1] looping


void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}


void loop() {
  // raw input
  previousRawInput = rawInput;
  rawInput = analogRead(inputPin);
    Serial.println(rawInput);

  // filtered input
  previousFilteredInput = buf[writeIndexInBuf];
  writeIndexInBuf = (writeIndexInBuf + 1) % BUFSIZE;
  filteredInput = (int) (rawInput - previousRawInput + 0.8 * previousFilteredInput); // remove DC offset
  buf[writeIndexInBuf] = filteredInput;

  if (writeIndexInBuf == BUFSIZE - 1) {
    calcPulseFrequency();
  }

  blinkPhase += SAMPLE_DURATION / 1000.0 * pulseFrequency;
  if (blinkPhase > 1 || isStrongHeartbeatOnset()) {
    blinkPhase = 0;
  }
  digitalWrite(ledPin, blinkPhase < 0.2 ? HIGH : LOW);
  delay(SAMPLE_DURATION);
//  Serial.println(filteredInput);
}


void calcPulseFrequency() {
  long sum;
  int tau, i;
  for (tau = tauStart; tau < halfBufsize; tau++) {
    sum = 0;
    for (i = 0; i < halfBufsize; i++) {
      sum += buf[i] * buf[tau + i];
    }
    correlation[tau] = sum;
  }
  pulseFrequency = 1000.0 / (correlationPeakIndex() * SAMPLE_DURATION);
//  Serial.println(pulseFrequency);
}


int correlationPeakIndex() {
  long peakLevel = 0;
  int peakIndex = tauStart;
  for (int tau = tauStart; tau < halfBufsize; tau++) {
    if (correlation[tau] > peakLevel) {
      peakLevel = correlation[tau];
      peakIndex = tau;
    }
  }
  return peakIndex;
}

int isStrongHeartbeatOnset() {
  int delta = filteredInput - previousFilteredInput;
  deltaEnvelope = max(deltaEnvelope, delta) * 0.99; // follow the peaks in delta with a smooth decay
  return delta > deltaEnvelope;
}


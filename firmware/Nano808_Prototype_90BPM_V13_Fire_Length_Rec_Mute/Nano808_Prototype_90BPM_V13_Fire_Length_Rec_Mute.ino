/*
  Nano 808 Eurorack Drum Machine - prototype V13
  -------------------------------------------------
  Hardware:
    Arduino Nano / ATmega328P
    Mozzi 2.x
    MAX7219 8x8 matrix
    Internal clock fixed at 90 BPM for prototype
    Optional external RESET
    6 pots:
      A0 instrument select
      A1 parameter 1
      A2 parameter 2
      A3 parameter 3
      A4 step select
      A5 pattern length (1..16)
    Buttons:
      D7  step ON/OFF
      D8  FIRE (short press); MUTE selected instrument (long press)
      D11 REC/VARIATION for selected step
      D12 START/STOP transport
      D3  RESET to step 1
    Matrix:
      DIN D4
      CLK D5
      CS  D6
    D2 reserved for future external CLOCK IN
    Optional reset input D3
    Audio PWM D9 (Mozzi default on Nano)

  Instruments:
    0 Kick
    1 Snare
    2 Closed Hat
    3 Open Hat
    4 Clap
    5 Tom/Conga

  Matrix display (logical orientation):
    col 0 = selected instrument (Kick at bottom, then Snare/CHH/OHH/Clap/Tom)
    col 2 = parameter 1 bargraph
    col 4 = parameter 2 bargraph
    col 6 = parameter 3 bargraph
    row 6 = steps 1..8, left to right
    row 7 = steps 9..16, left to right

  Notes:
    - Prototype clock is generated internally at fixed 90 BPM.
    - 16-step sequencer runs as 16th notes: 4 steps per beat.
    - Timing is centralised so external variable clock can later update the
      measured step/beat period and keep the selected-step blink locked to it.
    - D2 is reserved for future external CLOCK IN.
    - A per-step variation/accent uses only a 16-bit mask per instrument.
    - Swing timing infrastructure is present but defaults to 0%.
    - Optional RESET on D3 still uses the protected/inverted input,
      therefore it uses a FALLING interrupt.
    - Patterns are stored as 16-bit masks, one per instrument.
*/

#include <Arduino.h>
#include <MozziConfigValues.h>
#define MOZZI_AUDIO_RATE 16384
#include <Mozzi.h>
#include <avr/pgmspace.h>
#include <tables/sin2048_int8.h>

#define CONTROL_RATE 128

// Forward declaration required by the Arduino sketch preprocessor.
// Some IDE versions generate function prototypes before the full Voice
// structure definition and otherwise fail on metallicHat(Voice &...).
struct Voice;

// ---------------- Pins ----------------
const uint8_t PIN_CLOCK_FUTURE = 2; // external CLOCK IN, protected 5 V logic
const uint8_t PIN_RESET = 3;

const uint8_t PIN_MAX_DIN = 4;
const uint8_t PIN_MAX_CLK = 5;
const uint8_t PIN_MAX_CS  = 6;

const uint8_t PIN_STEP_BUTTON = 7;
const uint8_t PIN_FIRE_BUTTON = 8;
// D9 = Mozzi PWM audio out. Leave D10 unused/spare for Mozzi/Timer1 headroom.
const uint8_t PIN_REC_BUTTON = 11;
const uint8_t PIN_START_STOP_BUTTON = 12;

const uint8_t PIN_POT_INSTR = A0;
const uint8_t PIN_POT_P1    = A1;
const uint8_t PIN_POT_P2    = A2;
const uint8_t PIN_POT_P3    = A3;
const uint8_t PIN_POT_STEP   = A4;
const uint8_t PIN_POT_LENGTH = A5;

// ---------------- Global UI/sequencer ----------------
enum Instrument : uint8_t {
  KICK = 0,
  SNARE,
  CHH,
  OHH,
  CLAP,
  TOM,
  NUM_INSTRUMENTS
};

volatile bool resetPending = false;

// ---------------- Clock / timing ----------------
// Prototype: fixed internal 90 BPM, 16th notes (4 steps per beat).
// Later, an external clock handler only has to update baseStepPeriodUs and
// beatPeriodUs; the display blink derives from those same timing values.
const uint16_t INTERNAL_BPM = 90;
uint32_t baseStepPeriodUs = 166667UL;
uint32_t beatPeriodUs = 666667UL;
uint32_t nextStepDueUs = 0;
uint32_t clockPhaseOriginUs = 0;
volatile uint32_t externalClockLastEdgeUs = 0;
volatile uint32_t externalClockPeriodUs = 0;
volatile bool externalClockPulsePending = false;
bool externalClockActive = false;

// 0..30 is a sensible future range. 0 = straight timing.
// Infrastructure only in V13; no front-panel swing control yet.
uint8_t swingPercent = 0;

uint8_t currentStep = 0;
bool sequencerRunning = true;
uint8_t selectedInstrument = 0;
uint8_t selectedStep = 0;
uint8_t patternLength = 16;

// 16-bit rhythm pattern, bit 0 = step 1
uint16_t pattern[NUM_INSTRUMENTS] = {
  // BASIC 16-STEP TEST PATTERN
  // Kick  : steps 1, 5, 9, 13  = four-on-the-floor
  // Snare : steps 5 and 13     = beats 2 and 4
  0b0001000100010001, // Kick
  0b0001000000010000, // Snare
  0b0000000000000000, // Closed Hat OFF for test
  0b0000000000000000, // Open Hat OFF for test
  0b0000000000000000, // Clap OFF for test
  0b0000000000000000  // Tom OFF for test
};

// Per-step variation/accent: 16 bits x 6 instruments = only 12 bytes RAM.
// REC toggles the bit for the selected instrument/step.
uint16_t variationPattern[NUM_INSTRUMENTS] = {0, 0, 0, 0, 0, 0};

// Mute affects sequencer playback only; FIRE can still audition a muted voice.
bool instrumentMuted[NUM_INSTRUMENTS] = {false, false, false, false, false, false};

// per-instrument character settings, 0..255
uint8_t param[NUM_INSTRUMENTS][3] = {
  {145, 175, 180}, // Kick: tune, decay, punch
  {125, 145, 185}, // Snare: tune, decay, noise
  {165,  70, 190}, // CHH: tone, decay, metal
  {165, 190, 190}, // OHH: tone, decay, metal
  {160, 155, 165}, // Clap: tone, decay, spread
  {130, 145, 140}  // Tom: tune, decay, sweep
};

// Buttons / debounce
bool oldStepButton = HIGH;
bool oldFireButton = HIGH;
bool oldRecButton = HIGH;
bool oldStartStopButton = HIGH;
uint32_t lastStepButtonEdgeUs = 0;
uint32_t lastFireButtonEdgeUs = 0;
uint32_t lastRecButtonEdgeUs = 0;
uint32_t lastStartStopButtonEdgeUs = 0;
uint32_t firePressStartedUs = 0;
bool fireLongPressHandled = false;
const uint32_t BUTTON_DEBOUNCE_US = 20000UL;
const uint32_t FIRE_LONG_PRESS_US = 650000UL;

// ---------------- MAX7219 ----------------
uint8_t matrixRows[8] = {0};

void maxSend(uint8_t reg, uint8_t data) {
  digitalWrite(PIN_MAX_CS, LOW);
  shiftOut(PIN_MAX_DIN, PIN_MAX_CLK, MSBFIRST, reg);
  shiftOut(PIN_MAX_DIN, PIN_MAX_CLK, MSBFIRST, data);
  digitalWrite(PIN_MAX_CS, HIGH);
}

void maxInit() {
  pinMode(PIN_MAX_DIN, OUTPUT);
  pinMode(PIN_MAX_CLK, OUTPUT);
  pinMode(PIN_MAX_CS, OUTPUT);
  digitalWrite(PIN_MAX_CS, HIGH);

  maxSend(0x0F, 0x00); // display test off
  maxSend(0x0C, 0x01); // shutdown off
  maxSend(0x0B, 0x07); // scan all 8 rows
  maxSend(0x09, 0x00); // no decode
  maxSend(0x0A, 0x01); // low intensity: reduces LED current/noise during prototype

  for (uint8_t r = 1; r <= 8; r++) maxSend(r, 0x00);
}

void matrixFlush() {
  for (uint8_t r = 0; r < 8; r++) {
    maxSend(r + 1, matrixRows[r]);
  }
}

// Raw MAX7219 pixel access.
void setPixelRaw(uint8_t row, uint8_t col, bool on) {
  if (row > 7 || col > 7) return;
  uint8_t mask = (1 << col);
  if (on) matrixRows[row] |= mask;
  else    matrixRows[row] &= ~mask;
}

// Logical display coordinates -> physical matrix coordinates.
// V6: display rotated a further 180 degrees compared with V5.
// Net orientation is now 90 degrees clockwise relative to the raw matrix.
void setPixel(uint8_t row, uint8_t col, bool on) {
  if (row > 7 || col > 7) return;

  // CW rotation: (row, col) -> (col, 7-row)
  uint8_t physicalRow = col;
  uint8_t physicalCol = 7 - row;
  setPixelRaw(physicalRow, physicalCol, on);
}

// 6-level vertical bargraph, ONE column wide.
// Value grows upward from row 5 toward row 0.
void drawBargraph(uint8_t col, uint8_t value) {
  uint8_t levels = ((uint16_t)value * 6 + 254) / 255; // 0..6

  for (uint8_t row = 0; row < 6; row++) {
    bool on = row >= (6 - levels);
    setPixel(row, col, on);
  }
}

void drawDisplay() {
  for (uint8_t i = 0; i < 8; i++) matrixRows[i] = 0;

  // Column 0: selected instrument.
  // Bottom to top:
  // row 5 Kick, 4 Snare, 3 CHH, 2 OHH, 1 Clap, 0 Tom.
  uint8_t instrumentRow = 5 - selectedInstrument;
  bool selectedMuteBlink = true;
  if (instrumentMuted[selectedInstrument]) {
    uint32_t phase = (uint32_t)(micros() - clockPhaseOriginUs);
    uint32_t halfBeat = beatPeriodUs >> 1;
    if (halfBeat == 0) halfBeat = 1;
    selectedMuteBlink = ((phase / halfBeat) & 1UL) == 0;
  }
  setPixel(instrumentRow, 0, selectedMuteBlink);

  // Three character parameters, one column each,
  // with one empty column between them.
  drawBargraph(2, param[selectedInstrument][0]);
  drawBargraph(4, param[selectedInstrument][1]);
  drawBargraph(6, param[selectedInstrument][2]);

  // Columns 1, 3, 5 and 7 intentionally left empty.

  // Bottom two logical rows: 16-step pattern.
  // Row 6 = steps 1..8, left to right.
  // Row 7 = steps 9..16, left to right.
  uint16_t p = pattern[selectedInstrument];

  for (uint8_t s = 0; s < 16; s++) {
    bool active = (p >> s) & 1;
    bool cursor = (s == currentStep);
    bool edit   = (s == selectedStep);

    uint8_t row = (s < 8) ? 6 : 7;

    // After the display rotation, reverse only the sequencer columns
    // so steps still read from LEFT to RIGHT.
    uint8_t col = 7 - (s & 7);

    bool on = active || cursor;

    // Selected step ALWAYS blinks, even if active.
    // Blink derives from beatPeriodUs, so when the BPM later comes from an
    // external clock, the blink follows that measured tempo automatically.
    uint32_t phase = (uint32_t)(micros() - clockPhaseOriginUs);
    uint32_t halfBeat = beatPeriodUs >> 1;
    if (halfBeat == 0) halfBeat = 1;
    bool bpmBlink = ((phase / halfBeat) & 1UL) == 0;

    if (edit) on = bpmBlink;

    setPixel(row, col, on);
  }

  matrixFlush();
}

// ---------------- Clock / reset ----------------
void resetISR() {
  resetPending = true;
}

// D2 external clock input. The ISR only timestamps the edge; all timing and
// voice work remains in updateControl(). One rising edge advances one step.
void externalClockISR() {
  uint32_t now = micros();
  uint32_t previous = externalClockLastEdgeUs;
  externalClockLastEdgeUs = now;

  if (previous != 0) {
    uint32_t period = now - previous;
    if (period >= 30000UL && period <= 500000UL) {
      externalClockPeriodUs = period;
      externalClockPulsePending = true;
    }
  }
}

// ---------------- Lightweight sound engine ----------------
//
// Uses manual 32-bit phases + sine table.
// Envelopes use 0..32767.
// This deliberately avoids ADSR objects to keep Nano RAM usage low.

const uint32_t PHASE_SCALE = (uint32_t)(4294967296ULL / MOZZI_AUDIO_RATE);

static inline uint32_t hzToInc(uint16_t hz) {
  return (uint32_t)hz * PHASE_SCALE;
}

static inline int8_t sineFromPhase(uint32_t phase) {
  uint16_t idx = phase >> 21; // top 11 bits => 0..2047
  return (int8_t)pgm_read_byte_near(SIN2048_DATA + idx);
}

uint16_t lfsr = 0xACE1u;
static inline int8_t noise8() {
  uint16_t bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
  lfsr = (lfsr >> 1) | (bit << 15);
  return (int8_t)(lfsr >> 8);
}

struct Voice {
  uint32_t phase1;
  uint32_t phase2;
  uint32_t phase3;
  uint16_t env;
  uint16_t age;
  uint16_t aux;
  bool active;
};

Voice v[NUM_INSTRUMENTS];

void triggerInstrument(uint8_t inst, bool variation) {
  Voice &x = v[inst];
  x.phase1 = 0;
  x.phase2 = 0;
  x.phase3 = 0;
  x.env = 32767;
  x.age = 0;
  x.aux = variation ? 1 : 0; // reuse existing field: no extra RAM
  x.active = true;
}

void triggerCurrentStep() {
  for (uint8_t i = 0; i < NUM_INSTRUMENTS; i++) {
    if (instrumentMuted[i]) continue;
    if (((pattern[i] >> currentStep) & 1U) == 0) continue;

    bool variation = ((variationPattern[i] >> currentStep) & 1U) != 0;
    triggerInstrument(i, variation);
  }
}

uint32_t stepDurationUs(uint8_t step) {
  if (swingPercent == 0) return baseStepPeriodUs;

  uint32_t offset = (baseStepPeriodUs * swingPercent) / 100UL;
  // Even-numbered logical steps are lengthened, odd-numbered shortened.
  return (step & 1U) ? (baseStepPeriodUs - offset)
                     : (baseStepPeriodUs + offset);
}

void setTimingFromBPM(uint16_t bpm) {
  if (bpm < 30) bpm = 30;
  if (bpm > 300) bpm = 300;

  beatPeriodUs = 60000000UL / bpm;
  baseStepPeriodUs = beatPeriodUs / 4UL;
}

// Future external-clock hook. When D2 clock input is implemented, pass its
// measured 16th-note period here; display blink and sequencing then share it.
void setTimingFromExternalStepPeriod(uint32_t measuredStepUs) {
  if (measuredStepUs < 30000UL) measuredStepUs = 30000UL;
  if (measuredStepUs > 500000UL) measuredStepUs = 500000UL;
  baseStepPeriodUs = measuredStepUs;
  beatPeriodUs = measuredStepUs * 4UL;
}

void advanceSequencer() {
  currentStep++;
  if (currentStep >= patternLength) currentStep = 0;
  triggerCurrentStep();
}

void resetSequencer(bool triggerStepOne) {
  currentStep = 0;
  uint32_t now = micros();
  clockPhaseOriginUs = now;
  nextStepDueUs = now + stepDurationUs(currentStep);
  if (triggerStepOne) triggerCurrentStep();
}

// ---------------- Voice synthesis ----------------
int16_t synthKick() {
  Voice &x = v[KICK];
  if (!x.active) return 0;

  uint8_t tune  = param[KICK][0];
  uint8_t decay = param[KICK][1];
  uint8_t punch = param[KICK][2];
  bool variation = x.aux != 0;

  uint16_t baseHz = 38 + ((uint16_t)tune * 42 >> 8); // 38..79
  if (variation) baseHz += 7;
  uint16_t ageSweep = (x.age >> 2);
  if (ageSweep > 145) ageSweep = 145;
  uint16_t sweep = 150 - ageSweep;
  uint16_t hz = baseHz + sweep;

  x.phase1 += hzToInc(hz);
  x.phase2 += hzToInc(hz * 2U);

  int16_t s = sineFromPhase(x.phase1);
  int16_t harmonic = sineFromPhase(x.phase2);
  // A small second harmonic gives the kick more body on small speakers while
  // retaining the fundamental for a clean low-end pulse.
  int16_t body = s + (harmonic >> 2);
  int32_t out = ((int32_t)body * x.env) >> 8;

  // short click/punch
  if (x.age < 90) {
    int16_t click = noise8();
    uint16_t effectivePunch = punch + (variation ? 55 : 0);
    if (effectivePunch > 255) effectivePunch = 255;
    out += ((int32_t)click * effectivePunch * (90 - x.age)) >> 8;
  }

  if (variation) out += out >> 2;

  uint16_t dec = 28 + ((255 - decay) >> 2);
  if (x.env > dec) x.env -= dec;
  else { x.env = 0; x.active = false; }

  x.age++;
  return constrain(out, -12000, 12000);
}

int16_t synthSnare() {
  Voice &x = v[SNARE];
  if (!x.active) return 0;

  uint8_t tune  = param[SNARE][0];
  uint8_t decay = param[SNARE][1];
  uint8_t noiseAmt = param[SNARE][2];
  bool variation = x.aux != 0;

  uint16_t f1 = 135 + ((uint16_t)tune * 80 >> 8);
  uint16_t f2 = 210 + ((uint16_t)tune * 95 >> 8);
  if (variation) { f1 += 16; f2 += 23; }

  x.phase1 += hzToInc(f1);
  x.phase2 += hzToInc(f2);

  int16_t body = (sineFromPhase(x.phase1) + sineFromPhase(x.phase2)) >> 1;
  int16_t n = noise8();

  uint16_t effectiveNoise = noiseAmt + (variation ? 28 : 0);
  if (effectiveNoise > 255) effectiveNoise = 255;
  int16_t mix = ((int32_t)body * (255 - effectiveNoise) +
                 (int32_t)n * effectiveNoise) >> 8;
  int32_t out = ((int32_t)mix * x.env) >> 7;
  // Short noise transient improves the attack without extending the decay.
  if (x.age < 42) out += ((int32_t)n * (42 - x.age)) >> 2;
  if (variation) out += out >> 2;

  uint16_t dec = 50 + ((255 - decay) >> 1);
  if (x.env > dec) x.env -= dec;
  else { x.env = 0; x.active = false; }

  x.age++;
  return constrain(out, -10000, 10000);
}

int16_t metallicHat(Voice &x, uint8_t inst, bool openHat) {
  if (!x.active) return 0;

  uint8_t tone  = param[inst][0];
  uint8_t decay = param[inst][1];
  uint8_t metal = param[inst][2];
  bool variation = x.aux != 0;

  uint16_t f1 = 820  + ((uint16_t)tone * 900 >> 8);
  uint16_t f2 = 1230 + ((uint16_t)tone * 1300 >> 8);
  uint16_t f3 = 1710 + ((uint16_t)tone * 1700 >> 8);
  if (variation) {
    f1 += f1 >> 4;
    f2 += f2 >> 4;
    f3 += f3 >> 4;
  }

  x.phase1 += hzToInc(f1);
  x.phase2 += hzToInc(f2);
  x.phase3 += hzToInc(f3);

  // "metal" from square-like phase MSBs
  int16_t m = (((x.phase1 >> 31) ? 90 : -90) +
               ((x.phase2 >> 31) ? 70 : -70) +
               ((x.phase3 >> 31) ? 55 : -55));

  // A small ring-like component breaks up the static square blend and gives
  // the hats a more recognisable metallic, inharmonic character.
  int16_t ring = (((x.phase1 >> 31) ^ (x.phase2 >> 30)) ? 42 : -42);
  m += ring;

  int16_t n = noise8();
  int16_t sig = ((int32_t)m * metal + (int32_t)n * (255 - metal)) >> 8;

  int32_t out = ((int32_t)sig * x.env) >> 7;
  if (variation) out += out >> 2;

  uint16_t decBase = openHat ? 12 : 95;
  uint16_t dec = decBase + ((255 - decay) >> (openHat ? 3 : 1));
  if (x.env > dec) x.env -= dec;
  else { x.env = 0; x.active = false; }

  x.age++;
  return constrain(out, -8500, 8500);
}

int16_t synthClap() {
  Voice &x = v[CLAP];
  if (!x.active) return 0;

  uint8_t tone   = param[CLAP][0];
  uint8_t decay  = param[CLAP][1];
  uint8_t spread = param[CLAP][2];
  bool variation = x.aux != 0;

  int16_t n = noise8();

  // pseudo high-pass: subtract a slowly changing noise component
  static int16_t clapLP = 0;
  clapLP += (n - clapLP) >> (3 + (tone >> 7));
  int16_t hp = n - clapLP;

  // classic-ish multi-burst clap envelope
  uint16_t burst = 0;
  uint16_t gap = 80 + ((uint16_t)spread >> 1);

  if (x.age < 70) burst = 32767 - x.age * 300;
  else if (x.age > gap && x.age < gap + 65)
    burst = 28000 - (x.age - gap) * 320;
  else if (x.age > gap * 2 && x.age < gap * 2 + 65)
    burst = 24000 - (x.age - gap * 2) * 300;
  else
    burst = x.env >> 2;

  int32_t out = ((int32_t)hp * burst) >> 8;
  if (variation) out += out >> 2;

  uint16_t dec = 22 + ((255 - decay) >> 2);
  if (x.env > dec) x.env -= dec;
  else { x.env = 0; x.active = false; }

  x.age++;
  if (x.age > 5000) x.active = false;
  return constrain(out, -9000, 9000);
}

int16_t synthTom() {
  Voice &x = v[TOM];
  if (!x.active) return 0;

  uint8_t tune  = param[TOM][0];
  uint8_t decay = param[TOM][1];
  uint8_t sweep = param[TOM][2];
  bool variation = x.aux != 0;

  uint16_t baseHz = 80 + ((uint16_t)tune * 180 >> 8);
  if (variation) baseHz += 18;
  uint16_t drop = 10 + ((uint16_t)sweep * 100 >> 8);
  uint16_t sweepNow = 0;
  if (x.age < 500) sweepNow = drop - ((uint32_t)drop * x.age / 500);

  x.phase1 += hzToInc(baseHz + sweepNow);
  x.phase2 += hzToInc((baseHz + sweepNow) * 2U);
  int16_t s = sineFromPhase(x.phase1);
  int16_t harmonic = sineFromPhase(x.phase2);
  int32_t out = ((int32_t)(s + (harmonic >> 3)) * x.env) >> 8;
  if (variation) out += out >> 2;

  uint16_t dec = 32 + ((255 - decay) >> 2);
  if (x.env > dec) x.env -= dec;
  else { x.env = 0; x.active = false; }

  x.age++;
  return constrain(out, -10000, 10000);
}

int32_t softClip(int32_t sample) {
  // Compress only the upper part of the mix. This keeps normal hits linear
  // and avoids the harsh edge of a hard digital clip when voices overlap.
  const int32_t threshold = 4800;
  const int32_t limit = 7000;
  if (sample > threshold) {
    sample = threshold + ((sample - threshold) >> 2);
  } else if (sample < -threshold) {
    sample = -threshold + ((sample + threshold) >> 2);
  }
  return constrain(sample, -limit, limit);
}

// ---------------- UI ----------------
uint8_t adcToByte(int v) {
  if (v < 0) v = 0;
  if (v > 1023) v = 1023;
  return (uint8_t)(v >> 2);
}

// Instrument selector with hysteresis.
// Six equal zones across A0.
uint16_t instrumentFilteredRaw = 0;
bool instrumentFilterInitialised = false;
uint8_t instrumentPendingCandidate = 255;
uint8_t instrumentPendingCount = 0;

uint8_t readInstrumentSelector(int raw) {
  const uint8_t HYST = 12; // ADC counts
  const uint8_t CONFIRM_READS = 2;

  if (raw < 0) raw = 0;
  if (raw > 1023) raw = 1023;

  if (!instrumentFilterInitialised) {
    instrumentFilteredRaw = raw;
    instrumentFilterInitialised = true;
  } else {
    int32_t filterDelta = (int32_t)raw - instrumentFilteredRaw;
    int32_t filterStep = filterDelta >> 3;
    if (filterDelta != 0 && filterStep == 0) {
      filterStep = filterDelta > 0 ? 1 : -1;
    }
    instrumentFilteredRaw += filterStep;
  }

  raw = instrumentFilteredRaw;

  uint8_t candidate = ((uint32_t)raw * NUM_INSTRUMENTS) >> 10;
  if (candidate >= NUM_INSTRUMENTS) candidate = NUM_INSTRUMENTS - 1;

  if (candidate == selectedInstrument) {
    instrumentPendingCandidate = 255;
    instrumentPendingCount = 0;
    return selectedInstrument;
  }

  bool crossedFarEnough = false;
  if (candidate > selectedInstrument) {
    uint16_t boundary =
        ((uint32_t)(selectedInstrument + 1) * 1024UL) / NUM_INSTRUMENTS;
    crossedFarEnough = raw >= (int)boundary + HYST;
  } else {
    uint16_t boundary =
        ((uint32_t)selectedInstrument * 1024UL) / NUM_INSTRUMENTS;
    crossedFarEnough = raw <= (int)boundary - HYST;
  }

  if (!crossedFarEnough) {
    instrumentPendingCandidate = 255;
    instrumentPendingCount = 0;
    return selectedInstrument;
  }

  if (candidate != instrumentPendingCandidate) {
    instrumentPendingCandidate = candidate;
    instrumentPendingCount = 1;
    return selectedInstrument;
  }

  if (instrumentPendingCount < 255) instrumentPendingCount++;
  if (instrumentPendingCount >= CONFIRM_READS) {
    instrumentPendingCandidate = 255;
    instrumentPendingCount = 0;
    return candidate;
  }

  return selectedInstrument;
}

// Soft takeover:
// Each instrument keeps its own param[][] values.
// After changing instrument, a physical pot has no effect until it reaches
// or crosses that instrument's stored value.
bool pickup[3] = {false, false, false};
uint8_t previousPhysical[3] = {0, 0, 0};
uint8_t parameterFiltered[3] = {0, 0, 0};

// The pots are read through the Nano ADC.  A small digital deadband prevents
// one ADC count of noise from becoming audible parameter modulation while the
// filtered value still covers the complete 0..255 range.
const uint8_t PARAMETER_DEADBAND = 2;

void armSoftTakeover() {
  pickup[0] = false;
  pickup[1] = false;
  pickup[2] = false;

  previousPhysical[0] = adcToByte(mozziAnalogRead(PIN_POT_P1));
  previousPhysical[1] = adcToByte(mozziAnalogRead(PIN_POT_P2));
  previousPhysical[2] = adcToByte(mozziAnalogRead(PIN_POT_P3));
  parameterFiltered[0] = previousPhysical[0];
  parameterFiltered[1] = previousPhysical[1];
  parameterFiltered[2] = previousPhysical[2];
}

void handleSoftTakeover(uint8_t index, int raw) {
  const uint8_t PICKUP_TOLERANCE = 3;

  uint8_t physical = adcToByte(raw);
  int16_t filterDelta = (int16_t)physical - parameterFiltered[index];
  int16_t filterStep = filterDelta >> 2;
  if (filterDelta != 0 && filterStep == 0) {
    filterStep = filterDelta > 0 ? 1 : -1;
  }
  int16_t filtered = (int16_t)parameterFiltered[index] + filterStep;
  if (filtered < 0) filtered = 0;
  if (filtered > 255) filtered = 255;
  parameterFiltered[index] = (uint8_t)filtered;
  uint8_t stablePhysical = parameterFiltered[index];
  uint8_t stored = param[selectedInstrument][index];

  if (!pickup[index]) {
    int16_t d = (int16_t)stablePhysical - (int16_t)stored;

    // Either arrive very close to the stored value...
    bool closeEnough = (d >= -(int16_t)PICKUP_TOLERANCE &&
                        d <=  (int16_t)PICKUP_TOLERANCE);

    // ...or cross it between two control reads.
    uint8_t lo = previousPhysical[index] < stablePhysical
                   ? previousPhysical[index] : stablePhysical;
    uint8_t hi = previousPhysical[index] > stablePhysical
                   ? previousPhysical[index] : stablePhysical;
    bool crossed = (stored >= lo && stored <= hi);

    if (closeEnough || crossed) pickup[index] = true;
  }

  if (pickup[index]) {
    int16_t delta = (int16_t)stablePhysical - param[selectedInstrument][index];
    if (delta >= PARAMETER_DEADBAND || delta <= -PARAMETER_DEADBAND) {
      param[selectedInstrument][index] = stablePhysical;
    }
  }

  previousPhysical[index] = stablePhysical;
}

// A4 step selector:
// - 16 equal-width zones between calibrated endpoints
// - slower smoothing, hysteresis and consecutive-read confirmation
// - values outside the usable range are clamped to steps 1 and 16
//
// Calibration values can later be adjusted if needed after measuring A4.
// Values below STEP_ADC_MIN stay on step 1.
// Values above STEP_ADC_MAX stay on step 16.
uint16_t stepFilteredRaw = 0;
bool stepFilterInitialised = false;
uint8_t stepPendingCandidate = 255;
uint8_t stepPendingCount = 0;

uint8_t readStepSelector(int raw) {
  // These are deliberately conservative values for a 10 k linear pot. They
  // leave a small margin for end-stop tolerance; tune after measuring the
  // actual panel pot and record the measured values in HARDWARE.md.
  const int STEP_ADC_MIN = 24;
  const int STEP_ADC_MAX = 999;
  const int STEP_HYST = 8;
  const uint8_t CONFIRM_READS = 3;

  if (raw < 0) raw = 0;
  if (raw > 1023) raw = 1023;

  if (!stepFilterInitialised) {
    stepFilteredRaw = raw;
    stepFilterInitialised = true;
  } else {
    // Slower IIR smoothing removes ADC noise without adding a blocking wait.
    int32_t filterDelta = (int32_t)raw - (int32_t)stepFilteredRaw;
    int32_t filterStep = filterDelta >> 3;
    if (filterDelta != 0 && filterStep == 0) {
      filterStep = filterDelta > 0 ? 1 : -1;
    }
    stepFilteredRaw += filterStep;
  }

  int filtered = stepFilteredRaw;

  // Saturate outside calibrated range.
  if (filtered <= STEP_ADC_MIN) {
    stepPendingCandidate = 255;
    stepPendingCount = 0;
    return 0;
  }
  if (filtered >= STEP_ADC_MAX) {
    stepPendingCandidate = 255;
    stepPendingCount = 0;
    return 15;
  }

  const int span = STEP_ADC_MAX - STEP_ADC_MIN + 1;

  // 16 equal-width zones.
  uint8_t candidate = (uint8_t)(
      ((uint32_t)(filtered - STEP_ADC_MIN) * 16UL) / span
  );
  if (candidate > 15) candidate = 15;

  if (candidate == selectedStep) {
    stepPendingCandidate = 255;
    stepPendingCount = 0;
    return selectedStep;
  }

  // Exact equal-width boundaries of the CURRENT step.
  int lowerBoundary =
      STEP_ADC_MIN + ((uint32_t)selectedStep * span) / 16UL;
  int upperBoundary =
      STEP_ADC_MIN + ((uint32_t)(selectedStep + 1) * span) / 16UL;

  bool crossedFarEnough = false;

  if (candidate > selectedStep) {
    crossedFarEnough = filtered >= upperBoundary + STEP_HYST;
  } else {
    crossedFarEnough = filtered <= lowerBoundary - STEP_HYST;
  }

  if (!crossedFarEnough) {
    stepPendingCandidate = 255;
    stepPendingCount = 0;
    return selectedStep;
  }

  // Require a few consecutive reads on the same candidate.
  if (candidate != stepPendingCandidate) {
    stepPendingCandidate = candidate;
    stepPendingCount = 1;
    return selectedStep;
  }

  if (stepPendingCount < 255) stepPendingCount++;

  if (stepPendingCount >= CONFIRM_READS) {
    selectedStep = candidate;
    stepPendingCandidate = 255;
    stepPendingCount = 0;
  }

  return selectedStep;
}

// A5 pattern-length selector: 1..16 with equal zones, smoothing and hysteresis.
uint16_t lengthFilteredRaw = 0;
bool lengthFilterInitialised = false;

uint8_t readPatternLength(int raw) {
  const int ADC_MIN = 24;
  const int ADC_MAX = 999;
  const int HYST = 8;

  if (raw < 0) raw = 0;
  if (raw > 1023) raw = 1023;

  if (!lengthFilterInitialised) {
    lengthFilteredRaw = raw;
    lengthFilterInitialised = true;
  } else {
    int32_t filterDelta = (int32_t)raw - (int32_t)lengthFilteredRaw;
    int32_t filterStep = filterDelta >> 3;
    if (filterDelta != 0 && filterStep == 0) {
      filterStep = filterDelta > 0 ? 1 : -1;
    }
    lengthFilteredRaw += filterStep;
  }

  int filtered = lengthFilteredRaw;
  if (filtered <= ADC_MIN) return 1;
  if (filtered >= ADC_MAX) return 16;

  const int span = ADC_MAX - ADC_MIN + 1;
  uint8_t candidate = 1 + (uint8_t)(((uint32_t)(filtered - ADC_MIN) * 16UL) / span);
  if (candidate > 16) candidate = 16;
  if (candidate == patternLength) return patternLength;

  int lower = ADC_MIN + ((uint32_t)(patternLength - 1) * span) / 16UL;
  int upper = ADC_MIN + ((uint32_t)patternLength * span) / 16UL;

  if (candidate > patternLength) {
    if (filtered >= upper + HYST) return candidate;
  } else {
    if (filtered <= lower - HYST) return candidate;
  }

  return patternLength;
}

void updateControlsUI() {
  uint32_t nowUs = micros();

  uint8_t newInstrument =
      readInstrumentSelector(mozziAnalogRead(PIN_POT_INSTR));

  if (newInstrument != selectedInstrument) {
    selectedInstrument = newInstrument;
    // Keep each instrument's own stored params; no confirmation sound.
    armSoftTakeover();
  }

  selectedStep = readStepSelector(mozziAnalogRead(PIN_POT_STEP));

  uint8_t newLength = readPatternLength(mozziAnalogRead(PIN_POT_LENGTH));
  if (newLength != patternLength) {
    patternLength = newLength;
    if (currentStep >= patternLength) {
      currentStep = 0;
      clockPhaseOriginUs = nowUs;
      nextStepDueUs = nowUs + stepDurationUs(currentStep);
    }
  }

  handleSoftTakeover(0, mozziAnalogRead(PIN_POT_P1));
  handleSoftTakeover(1, mozziAnalogRead(PIN_POT_P2));
  handleSoftTakeover(2, mozziAnalogRead(PIN_POT_P3));

  // STEP ON/OFF button (D7)
  bool stepRaw = digitalRead(PIN_STEP_BUTTON);
  if (stepRaw != oldStepButton &&
      (uint32_t)(nowUs - lastStepButtonEdgeUs) >= BUTTON_DEBOUNCE_US) {
    oldStepButton = stepRaw;
    lastStepButtonEdgeUs = nowUs;
    if (stepRaw == LOW) {
      pattern[selectedInstrument] ^= (1U << selectedStep);
    }
  }

  // START/STOP transport button (D12).
  // A stop freezes the sequencer position; voices already sounding are allowed
  // to decay naturally. Starting resumes from the current position.
  bool startStopRaw = digitalRead(PIN_START_STOP_BUTTON);
  if (startStopRaw != oldStartStopButton &&
      (uint32_t)(nowUs - lastStartStopButtonEdgeUs) >= BUTTON_DEBOUNCE_US) {
    oldStartStopButton = startStopRaw;
    lastStartStopButtonEdgeUs = nowUs;
    if (startStopRaw == LOW) {
      sequencerRunning = !sequencerRunning;
      if (sequencerRunning) {
        nextStepDueUs = nowUs + stepDurationUs(currentStep);
        clockPhaseOriginUs = nowUs;
      }
    }
  }

  // FIRE button (D8): immediate audition on press.
  // Long press toggles sequencer mute for the selected instrument.
  bool fireRaw = digitalRead(PIN_FIRE_BUTTON);
  if (fireRaw != oldFireButton &&
      (uint32_t)(nowUs - lastFireButtonEdgeUs) >= BUTTON_DEBOUNCE_US) {
    oldFireButton = fireRaw;
    lastFireButtonEdgeUs = nowUs;

    if (fireRaw == LOW) {
      firePressStartedUs = nowUs;
      fireLongPressHandled = false;
      triggerInstrument(selectedInstrument, false);
    }
  }

  if (fireRaw == LOW && !fireLongPressHandled &&
      (uint32_t)(nowUs - firePressStartedUs) >= FIRE_LONG_PRESS_US) {
    instrumentMuted[selectedInstrument] = !instrumentMuted[selectedInstrument];
    fireLongPressHandled = true;
  }

  // REC / VARIATION button (D11): toggle a different timbre/accent for the
  // selected instrument at the selected step. Pattern ON/OFF remains D7.
  bool recRaw = digitalRead(PIN_REC_BUTTON);
  if (recRaw != oldRecButton &&
      (uint32_t)(nowUs - lastRecButtonEdgeUs) >= BUTTON_DEBOUNCE_US) {
    oldRecButton = recRaw;
    lastRecButtonEdgeUs = nowUs;
    if (recRaw == LOW) {
      variationPattern[selectedInstrument] ^= (1U << selectedStep);
    }
  }
}

// ---------------- Mozzi ----------------
void updateControl() {
  if (resetPending) {
    noInterrupts();
    resetPending = false;
    interrupts();
    resetSequencer(true);
  }

  bool clockPulse = false;
  uint32_t clockPeriod = 0;
  noInterrupts();
  if (externalClockPulsePending) {
    externalClockPulsePending = false;
    clockPeriod = externalClockPeriodUs;
    clockPulse = true;
  }
  interrupts();

  if (clockPulse) {
    setTimingFromExternalStepPeriod(clockPeriod);
    if (!externalClockActive) {
      externalClockActive = true;
      resetSequencer(true);
    } else if (sequencerRunning) {
      advanceSequencer();
    }
  }

  uint32_t now = micros();
  if (sequencerRunning && !externalClockActive) {
    while ((int32_t)(now - nextStepDueUs) >= 0) {
      advanceSequencer();
      nextStepDueUs += stepDurationUs(currentStep);
    }
  }

  updateControlsUI();

  static uint8_t displayDivider = 0;
  if (++displayDivider >= 4) { // ~32 Hz display refresh
    displayDivider = 0;
    drawDisplay();
  }
}

AudioOutput updateAudio() {
  int32_t mix = 0;

  mix += synthKick();
  mix += synthSnare();
  mix += metallicHat(v[CHH], CHH, false);
  mix += metallicHat(v[OHH], OHH, true);
  mix += synthClap();
  mix += synthTom();

  // More headroom than V3.
  mix >>= 2;
  mix = softClip(mix);

  // Gentle digital low-pass.
  // This reduces high-frequency digital/hash content without heavily
  // muffling the kick/snare. It cannot completely remove PWM carrier:
  // the 270R + 100nF hardware filter remains important.
  static int32_t filteredMix = 0;
  filteredMix += (mix - filteredMix) >> 1;

  return MonoOutput::from16Bit((int16_t)(filteredMix << 2));
}

void setup() {
  pinMode(PIN_RESET, INPUT_PULLUP);
  pinMode(PIN_STEP_BUTTON, INPUT_PULLUP);
  pinMode(PIN_FIRE_BUTTON, INPUT_PULLUP);
  pinMode(PIN_REC_BUTTON, INPUT_PULLUP);
  pinMode(PIN_START_STOP_BUTTON, INPUT_PULLUP);
  pinMode(PIN_CLOCK_FUTURE, INPUT_PULLUP);

  maxInit();

  attachInterrupt(digitalPinToInterrupt(PIN_RESET), resetISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_CLOCK_FUTURE), externalClockISR, RISING);

  startMozzi(CONTROL_RATE);
  setTimingFromBPM(INTERNAL_BPM);

  // Read length once before starting so an already-connected A5 is honoured.
  patternLength = readPatternLength(mozziAnalogRead(PIN_POT_LENGTH));
  resetSequencer(true);

  // Initial instrument (Kick): keep stored settings until each physical pot
  // reaches/crosses the stored value.
  armSoftTakeover();
}

void loop() {
  audioHook();
}

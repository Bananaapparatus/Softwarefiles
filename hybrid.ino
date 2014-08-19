//BEST SO FAR!

// Auduino, the Lo-Fi granular synthesiser
// by Peter Knight, Tinker.it http://tinker.it
// Help:      http://code.google.com/p/tinkerit/wiki/Auduino
// More help: http://groups.google.com/group/auduino
// Pin Setup
// Analog in 0: Grain 1 pitch
// Analog in 1: Grain 2 decay
// Analog in 2: Grain 1 decay
// Analog in 3: Grain 2 pitch
// Analog in 4: Grain repetition frequency
// Analog in 5: Arpeggiator Speed
// Digital 3: Audio out (Digital 11 on ATmega8)

#include <avr/io.h>
#include <avr/interrupt.h>

uint16_t pitch;
uint16_t pitchc;
uint16_t grain1freq;
uint16_t grain1freqc;
uint16_t grain1amp;
uint8_t grain1dec;
uint16_t grain2freq;
uint16_t grain2freqc;
uint16_t grain2amp;
uint8_t grain2dec;

int pinState1 = 0;
int pinState2 = 0;
int pinState3 = 0;
int lastvalue = 0;
int switchpos = 0;


// Map Analogue channels
#define GRAIN1F  (0)
#define GRAIN1D  (1)
#define GRAIN2F  (2)
#define GRAIN2D  (3)
#define PITCH    (4)
#define SPEED    (5)

//Input Buttons
#define pin1        4
#define pin2        5
#define pin3        6
#define PWM_PIN     3

#define PWM_VALUE OCR2B
#define LED_PORT  PORTB
#define LED_BIT   5
#define PWM_INTERRUPT TIMER2_OVF_vect

// Smooth logarithmic mapping
uint16_t antilogTable[] = {
  64830,64132,63441,62757,62081,61413,60751,60097,59449,58809,58176,57549,56929,56316,55709,55109,
  54515,53928,53347,52773,52204,51642,51085,50535,49991,49452,48920,48393,47871,47356,46846,46341,
  45842,45348,44859,44376,43898,43425,42958,42495,42037,41584,41136,40693,40255,39821,39392,38968,
  38548,38133,37722,37316,36914,36516,36123,35734,35349,34968,34591,34219,33850,33486,33125,32768
};

//Function Definition for mapPhaseInc()
uint16_t continuousnote(uint16_t input) {
  return (antilogTable[input & 0x3f]) >> (input >> 6);
}

// Stepped chromatic mapping
//
uint16_t midiTable[] = {
  17,18,19,20,22,23,24,26,27,29,31,32,34,36,38,41,43,46,48,51,54,58,61,65,69,73,
  77,82,86,92,97,103,109,115,122,129,137,145,154,163,173,183,194,206,218,231,
  244,259,274,291,308,326,346,366,388,411,435,461,489,518,549,581,616,652,691,
  732,776,822,871,923,978,1036,1097,1163,1232,1305,1383,1465,1552,1644,1742,
  1845,1955,2071,2195,2325,2463,2610,2765,2930,3104,3288,3484,3691,3910,4143,
  4389,4650,4927,5220,5530,5859,6207,6577,6968,7382,7821,8286,8779,9301,9854,
  10440,11060,11718,12415,13153,13935,14764,15642,16572,17557,18601,19708,20879,
  22121,23436,24830,26306
};

// Function Definition for mapMidi()
uint16_t note1(uint16_t input)
{
  return (midiTable[(1023-input) >> 3]);
}
uint16_t note2(uint16_t input)
{
  return (midiTable[((1023-input) >> 3)+1]);
}
uint16_t note3(uint16_t input)
{
  return (midiTable[((1023-input) >> 3)+2]);
}
uint16_t note4(uint16_t input)
{
  return (midiTable[((1023-input) >> 3)+3]);
}
uint16_t note5(uint16_t input)
{
  return (midiTable[((1023-input) >> 3)+4]);
}
uint16_t note6(uint16_t input)
{
  return (midiTable[((1023-input) >> 3)+5]);
}
uint16_t note7(uint16_t input)
{
  return (midiTable[((1023-input) >> 3)+6]);
}
uint16_t note8(uint16_t input)
{
  return (midiTable[((1023-input) >> 3)+7]);
}
uint16_t note9(uint16_t input)
{
  return (midiTable[((1023-input) >> 3)+8]);
}
uint16_t note10(uint16_t input)
{
  return (midiTable[((1023-input) >> 3)+9]);
}
uint16_t note11(uint16_t input)
{
  return (midiTable[((1023-input) >> 3)+10]);
}
uint16_t note12(uint16_t input)
{
  return (midiTable[((1023-input) >> 3)+11]);
}
uint16_t note13(uint16_t input)
{
  return (midiTable[((1023-input) >> 3)+12]);
}

// Stepped Pentatonic mapping
uint16_t pentatonicTable[54] = {
  0,19,22,26,29,32,38,43,51,58,65,77,86,103,115,129,154,173,206,231,259,308,346,
  411,461,518,616,691,822,923,1036,1232,1383,1644,1845,2071,2463,2765,3288,
  3691,4143,4927,5530,6577,7382,8286,9854,11060,13153,14764,16572,19708,22121,26306
};

// Function Definition for mapPentatonic()
uint16_t pentatonicnote(uint16_t input)
{
  uint8_t value = (1023-input) / (1024/53);
  return (pentatonicTable[value]);
}

void drone1()
{
  pitchc = note1(analogRead(PITCH));
}

void drone2()
{
  pitchc = continuousnote(analogRead(PITCH));
}

void drone3()
{
  pitchc = pentatonicnote(analogRead(PITCH));
}


void arpeggio1()
{
  pitchc = note1(analogRead(PITCH));
  delay(analogRead(SPEED)/4);
  pitchc = note5(analogRead(PITCH));
  delay(analogRead(SPEED)/4);
  pitchc = note8(analogRead(PITCH));
  delay(analogRead(SPEED)/4);
  pitchc = note5(analogRead(PITCH));
  delay(analogRead(SPEED)/4);
}

void arpeggio2()
{
  pitchc = note1(analogRead(PITCH));
  delay(analogRead(SPEED)/4);
  pitchc = note4(analogRead(PITCH));
  delay(analogRead(SPEED)/4);
  pitchc = note8(analogRead(PITCH));
  delay(analogRead(SPEED)/4);
  pitchc = note4(analogRead(PITCH));
  delay(analogRead(SPEED)/4);
}

void audioOn() {
  // Set up PWM to 31.25kHz, phase accurate
  TCCR2A = _BV(COM2B1) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  TIMSK2 = _BV(TOIE2);
}


void setup() {
  //Serial.begin(31250);
  Serial.begin(9600);
  audioOn();

  pinMode(pin1, INPUT);
  pinMode(pin2, INPUT);
  pinMode(pin3, INPUT);
  pinMode(PWM_PIN,OUTPUT);
}



void loop() {

  pinState1 = digitalRead(pin1);
  pinState2 = digitalRead(pin2);
  pinState3 = digitalRead(pin3);
  grain1freqc = analogRead(GRAIN1F) / 2;
  grain1dec = analogRead(GRAIN1D) / 8;
  grain2freqc = analogRead(GRAIN2F) / 2;
  grain2dec    = analogRead(GRAIN2D)/8;

  if(pinState1 == HIGH){

    if(pinState2 == HIGH){
      switchpos = 2;
    }
    else {
      switchpos = 1;
    }
  }

  else if(pinState2 == HIGH){

    if(pinState1 == HIGH){
      switchpos = 2;
    }
    else if(pinState3 == HIGH){
      switchpos = 4;
    }
    else {
      switchpos = 3;
    }
  }

  else if(pinState3 ==HIGH){

    if(pinState2 == HIGH){
      switchpos = 4;
    }
    else {
      switchpos = 5;
    }
  }
  lastvalue = switchpos;

  if (switchpos == 1){
    drone1();
  }
  else if (switchpos == 2){
    drone2();
  }
  else if (switchpos == 3){
    drone3();
  }
  else if (switchpos == 4){
    arpeggio1();
  }
  else if (switchpos == 5){
    arpeggio2();

  }

  if(switchpos != lastvalue){
    Serial.print("Switch Position:");
    Serial.println(switchpos);
    Serial.println(""); //make some room
  }
}

SIGNAL(PWM_INTERRUPT)
{
  uint8_t value;
  uint16_t output;

  pitch = pitch + pitchc;
  if (pitch < pitchc)
  {
    //New Grain
    grain1freq = 0;
    grain1amp = 0x7fff;
    grain2freq = 0;
    grain2amp = 0x7fff;
    LED_PORT ^= 1 << LED_BIT; // Faster than using digitalWrite
  }

  // Increment the phase of the grain oscillators
  grain1freq =   grain1freq + grain1freqc;
  grain2freq =   grain2freq + grain2freqc;

  // Convert phase into a triangle wave
  value = (grain1freq  >> 7) & 0xff;

  if (grain1freq & 0x8000) value = ~value;
  // Multiply by current grain amplitude to get sample
  output = value * (grain1amp >> 8);

  // Repeat for second grain
  value = (grain2freq >> 7) & 0xff;
  if (grain2freq & 0x8000) value = ~value;
  output += value * (grain2amp >> 8);

  // Make the grain amplitudes decay by a factor every sample (exponential decay)
  grain1amp -= (grain1amp >> 8) * grain1dec;
  grain2amp -= (grain2amp >> 8) * grain2dec;

  // Scale output to the available range, clipping if necessary
  output >>= 9;
  if (output > 235) output = 235;

  // Output to PWM (this is faster than using analogWrite) 
  PWM_VALUE = output;
}


#include <Arduino.h>
#include <FastLED.h>

#include <TMCStepper.h>
#include <AccelStepper.h>

#define TENTACLE_NUMBER 0

#define EN_PIN 5    // Enable
#define DIR_PIN 3   // Direction
#define STEP_PIN 4  // Step
#define R_SENSE 0.2f
#define SERIAL_PORT Serial1 
#define DRIVER_ADDRESS 0b00

TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);
AccelStepper stepper = AccelStepper(stepper.DRIVER, STEP_PIN, DIR_PIN);

#if TENTACLE_NUMBER == 0
#define TOP_LEDS_HALF 28
#define BOTTOM_LEDS_HALF 81
#elif TENTACLE_NUMBER == 1
#define TOP_LEDS_HALF 28
#define BOTTOM_LEDS_HALF 81
#elif TENTACLE_NUMBER == 2
#define TOP_LEDS_HALF 28
#define BOTTOM_LEDS_HALF 81
#elif TENTACLE_NUMBER == 3
#define TOP_LEDS_HALF 28
#define BOTTOM_LEDS_HALF 81
#elif TENTACLE_NUMBER == 4
#define TOP_LEDS_HALF 28
#define BOTTOM_LEDS_HALF 81
#elif TENTACLE_NUMBER == 5
#define TOP_LEDS_HALF 28
#define BOTTOM_LEDS_HALF 81
#else
#define TOP_LEDS_HALF 45
#define BOTTOM_LEDS_HALF 81
#endif
#define VIRTUAL_LEDS BOTTOM_LEDS_HALF + TOP_LEDS_HALF

CRGB virtual_leds[VIRTUAL_LEDS];
CRGB top_leds[TOP_LEDS_HALF * 2];
CRGB bottom_leds[BOTTOM_LEDS_HALF * 2];

#define TOP_DATA_PIN 14
#define BOTTOM_DATA_PIN 15

CLEDController *top_fastled = NULL;
CLEDController *bottom_fastled = NULL;

#define FREAKOUT_PIN 16

void copy_virtual_leds_out() {
    for (uint8_t i = 0; i < BOTTOM_LEDS_HALF; i++) {
        bottom_leds[i] = virtual_leds[BOTTOM_LEDS_HALF - 1 - i];
        bottom_leds[BOTTOM_LEDS_HALF * 2 - 1 - i] = virtual_leds[BOTTOM_LEDS_HALF - 1 - i];
    }
    for (uint8_t i = 0; i < TOP_LEDS_HALF; i++) {
        top_leds[i] = virtual_leds[BOTTOM_LEDS_HALF + i];
        top_leds[TOP_LEDS_HALF * 2 - 1 - i] = virtual_leds[BOTTOM_LEDS_HALF + i];
    }
}


void setup1() {
    pinMode(FREAKOUT_PIN, INPUT_PULLUP);
    FastLED.addLeds<WS2812, TOP_DATA_PIN, GRB>(top_leds, TOP_LEDS_HALF * 2);
    FastLED.addLeds<WS2812, BOTTOM_DATA_PIN, GRB>(bottom_leds, BOTTOM_LEDS_HALF * 2);
}


CRGBPalette16 currentPalette = RainbowColors_p;
TBlendType    currentBlending = LINEARBLEND;


void FillLEDsFromPaletteColors( uint8_t colorIndex)
{   
    for( int i = 0; i < VIRTUAL_LEDS; ++i) {
        virtual_leds[i] = ColorFromPalette(currentPalette, colorIndex, 255, currentBlending);
        colorIndex += 3;
    }
}

long last_light_step_millis = 0;
bool freakout = false;

void loop1() {
    static uint8_t startIndex = 0;
    freakout = !digitalRead(FREAKOUT_PIN);
    if (freakout) {
        currentPalette = RainbowStripeColors_p;
        currentBlending = LINEARBLEND;
        if (millis() - last_light_step_millis > 7) {
            startIndex = startIndex + 1;
            last_light_step_millis = millis();
        }
    } else {
        currentBlending = LINEARBLEND;
        currentPalette = RainbowColors_p;
        if (millis() - last_light_step_millis > 100) {
            startIndex = startIndex + 1;
            last_light_step_millis = millis();
        }
    }
    FillLEDsFromPaletteColors(startIndex);
    copy_virtual_leds_out();
    FastLED.show();
}

constexpr uint32_t steps_per_mm = 80;

void setup() {
    // initialise motor pins
    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(EN_PIN, OUTPUT);
    digitalWrite(EN_PIN, LOW);  // Enable driver in hardware

    SERIAL_PORT.begin(115200);

    driver.begin();
    driver.toff(5);
    driver.rms_current(2000, 0.1); 
    driver.blank_time(24);
    driver.iholddelay(3); 
    driver.microsteps(8);
    driver.pwm_autoscale(true); 

    stepper.setMaxSpeed(60*steps_per_mm); // 100mm/s @ 80 steps/mm
    stepper.setAcceleration(1000*steps_per_mm); // 2000mm/s^2
    stepper.setEnablePin(EN_PIN);
    stepper.setPinsInverted(false, false, true);
    stepper.enableOutputs();
}

// Loop for alternating between the timers
void loop() {
    static float s = 0;
    if (freakout) {
        s = sin(2 * PI * millis() / 1000 * 3 / 4) * 70 * steps_per_mm;
    } else {
        s = sin(2 * PI * millis() / 1000 / 11 - 0.581) * 5*steps_per_mm
            + sin(2 * PI * millis() / 1000 / 7 + 3.5739) * 4*steps_per_mm
            + sin(2 * PI * millis() / 1000 / 3 - 2.1111) * 2*steps_per_mm
            + sin(2 * PI * millis() / 1000 / 5 + 95.72) * 5*steps_per_mm
            + sin(2 * PI * millis() / 1000 / 13 + 4.7) * 5*steps_per_mm
        ;
    }
    stepper.setSpeed(s);
    stepper.run();
}

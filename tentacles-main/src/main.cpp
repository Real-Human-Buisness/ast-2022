#include <Arduino.h>
#include <FastLED.h>

#include <TMCStepper.h>
#include <AccelStepper.h>

#define EN_PIN 6    // Enable
#define DIR_PIN 9   // Direction
#define STEP_PIN 8  // Step
#define R_SENSE 0.2f
#define SERIAL_PORT Serial1 
#define DRIVER_ADDRESS 0b00

TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);
AccelStepper stepper = AccelStepper(stepper.DRIVER, STEP_PIN, DIR_PIN);

#define TOP_LEDS_HALF 10
#define BOTTOM_LEDS_HALF 20
#define VIRTUAL_LEDS BOTTOM_LEDS_HALF + TOP_LEDS_HALF

CRGB virtual_leds[VIRTUAL_LEDS];
CRGB top_leds[TOP_LEDS_HALF * 2];
CRGB bottom_leds[BOTTOM_LEDS_HALF * 2];

#define TOP_DATA_PIN 14
#define BOTTOM_DATA_PIN 15

CLEDController *top_fastled = NULL;
CLEDController *bottom_fastled = NULL;

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

void setup() {
    FastLED.addLeds<WS2812, TOP_DATA_PIN, GRB>(top_leds, TOP_LEDS_HALF * 2);
    FastLED.addLeds<WS2812, BOTTOM_DATA_PIN, GRB>(bottom_leds, BOTTOM_LEDS_HALF * 2);
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{   
    for( int i = 0; i < VIRTUAL_LEDS; ++i) {
        virtual_leds[i] = ColorFromPalette(RainbowColors_p, colorIndex, 255, LINEARBLEND);
        colorIndex += 3;
    }
}

void loop() {
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */
    FillLEDsFromPaletteColors(startIndex);
    copy_virtual_leds_out();
    FastLED.show();
    FastLED.delay(100);
}

constexpr uint32_t steps_per_mm = 80;

void setup1() {
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

    stepper.setMaxSpeed(50*steps_per_mm); // 100mm/s @ 80 steps/mm
    stepper.setAcceleration(1000*steps_per_mm); // 2000mm/s^2
    stepper.setEnablePin(EN_PIN);
    stepper.setPinsInverted(false, false, true);
    stepper.enableOutputs();
}

// Loop for alternating between the timers
void loop1() {
    if (stepper.distanceToGo() == 0) {
        stepper.disableOutputs();
        delay(100);
        stepper.move(100*steps_per_mm); // Move 100mm
        stepper.enableOutputs();
    }
    stepper.run();
}

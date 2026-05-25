#include <Wire.h>
#include <U8g2lib.h>
#include <ezButton.h>
#include <AccelStepper.h>
#include <RotaryEncoder.h>

// Attiny 1614
#define SCL PIN_PB0
#define SDA PIN_PB1

// NEMA 17 standard stepper with a Pololu A4988 driver or drop-in equivalent
// Motor interface type must be set to 1 when using a driver https://www.makerguides.com/a4988-stepper-motor-driver-arduino-tutorial
#define dirPin 2
#define stepPin 3
#define motorInterfaceType 1

AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

// Seed OLED display, driven through I2C
U8G2_SH1107_SEEED_128X128_1_SW_I2C u8g2(U8G2_R0, /* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE);

// RotaryEncoder
#define PIN_CLK PIN_PA4
#define PIN_DT PIN_PA5

RotaryEncoder encoder(PIN_CLK, PIN_DT, RotaryEncoder::LatchMode::FOUR0);  // FOUR0 — latch on every edge (higher resolution) -> 80 steps per rev

#define stepsPerRevolutionStepper 200
#define stepsPerRevolutionEncoder 80
#define stepperEncoderRatio stepsPerRevolutionStepper / stepsPerRevolutionEncoder

// Program variables
int encoderPosition = 0;
int stepperPosition = 0;

void setup() {

  stepper.setMaxSpeed(1000);

  u8g2.begin();

  attachInterrupt(
    digitalPinToInterrupt(PIN_CLK), [] {
      encoder.tick();
    },
    CHANGE);
  attachInterrupt(
    digitalPinToInterrupt(PIN_DT), [] {
      encoder.tick();
    },
    CHANGE);

  u8g2.firstPage();

  do {
    u8g2.setFont(u8g2_font_luBIS08_tf);
    u8g2.drawStr(0, 24, "Hello Stepper!");
  } while (u8g2.nextPage());

  delay(5000);
}

void loop() {

  u8g2.firstPage();

  encoderPosition = encoder.getPosition();
  stepperPosition = encoderPosition * stepperEncoderRatio;

  do {
    u8g2.setFont(u8g2_font_luBIS08_tf);
    u8g2.drawStr(0, 24, encoderPosition);
    u8g2.drawStr(0, 48, stepperPosition);
  } while (u8g2.nextPage());

  // Move back to zero:
  stepper.moveTo(stepperPosition);
  stepper.runToPosition();
}

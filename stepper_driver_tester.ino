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
U8G2_SH1107_SEEED_128X128_1_HW_I2C u8g2(U8G2_R0, /* clock=*/SCL, /* data=*/SDA, /* reset=*/U8X8_PIN_NONE);

// RotaryEncoder
#define PIN_CLK PIN_PA4
#define PIN_DT PIN_PA5

RotaryEncoder encoder(PIN_CLK, PIN_DT, RotaryEncoder::LatchMode::FOUR0);  // FOUR0 — latch on every edge (higher resolution) -> 80 steps per rev

#define stepsPerRevolutionStepper 200
#define stepsPerRevolutionEncoder 80
#define stepperEncoderRatio stepsPerRevolutionStepper / stepsPerRevolutionEncoder

// Status LED
#define STATUS_LED PIN_PA2

// Program variables
int encoderPosition = 0;
int lastEncoderPosition = 0;
int encoderAdvance = 0;
int stepperPosition = 0;

void setup() {

  stepper.setMaxSpeed(1000);

  u8g2.begin();
  Wire.setClock(400000);

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

  pinMode(STATUS_LED, OUTPUT);
  delay(1000);
}

void loop() {
  lastEncoderPosition = encoderPosition;
  encoderPosition = encoder.getPosition();
  encoderAdvance = encoderPosition - lastEncoderPosition;

  // stepperPosition += 1;
  stepperPosition = stepperPosition + encoderAdvance * stepperEncoderRatio;
  stepper.moveTo(stepperPosition);
  stepper.setSpeed(1000);
  stepper.runSpeedToPosition();


  if (stepperPosition % 100 == 0) {
    digitalWriteFast(STATUS_LED, !digitalRead(STATUS_LED));
  }


  // updateDisplay();
}

void updateDisplay() {
  if (stepperPosition % 100 == 0) {  // This is slow, so don't do it too often
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_luBIS08_tf);
      u8g2.drawStr(0, 24, "E");

      u8g2.setCursor(24, 24);
      u8g2.print(u8x8_u16toa(encoderPosition, 4));

      u8g2.drawStr(0, 48, "S");
      u8g2.setCursor(24, 48);
      u8g2.print(u8x8_u16toa(stepperPosition, 4));
      stepper.run();
    } while (u8g2.nextPage());
  }
}

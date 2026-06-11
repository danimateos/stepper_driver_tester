#include <Wire.h>
#include <U8x8lib.h>
#include <ezButton.h>
#include <AccelStepper.h>
#include <RotaryEncoder.h>

// Attiny 1614
#define SCL PIN_PB0
#define SDA PIN_PB1

// NEMA 17 standard stepper with a Pololu A4988 driver or drop-in equivalent
// Motor interface type must be set to 1 when using a driver https://www.makerguides.com/a4988-stepper-motor-driver-arduino-tutorial
#define dirPin PIN_PA1
#define stepPin PIN_PA7
#define motorInterfaceType 1

AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

// OLED display, driven through I2C
U8X8_SSD1306_128X32_UNIVISION_HW_I2C u8x8(U8X8_PIN_NONE);

// RotaryEncoder
#define PIN_CLK PIN_PA4
#define PIN_DT PIN_PA5
#define PIN_SW PIN_PA6

RotaryEncoder encoder(PIN_CLK, PIN_DT, RotaryEncoder::LatchMode::FOUR0);  // FOUR0 — latch on every edge (higher resolution) -> 80 steps per rev

#define stepsPerRevolutionStepper 200
#define stepsPerRevolutionEncoder 80
#define stepperEncoderRatio stepsPerRevolutionStepper / stepsPerRevolutionEncoder

// Status LED
#define STATUS_LED PIN_PA2

// Program variables
long encoderPosition = 0;
long lastEncoderPosition = 0;
long encoderAdvance = 0;
long stepperPosition = 0;
long loops = 0;

void setup() {

  stepper.setMaxSpeed(1000);

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

  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setCursor(0, 24);
  u8x8.print("Hello Stepper!");

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


  if (loops % 10 == 0) {
    digitalWriteFast(STATUS_LED, !digitalRead(STATUS_LED));
  }


  updateDisplay();

  loops += 1;
}

void updateDisplay() {
  if (loops % 10 == 0) {
    u8x8.setCursor(0, 0);
    u8x8.print(loops);
    u8x8.setCursor(10, 0);
    u8x8.print(stepper.currentPosition());
    u8x8.setCursor(0, 1);
    u8x8.print("E:");
    u8x8.print(encoderPosition);
    u8x8.setCursor(8, 1);
    u8x8.print("S:");
    u8x8.print(stepperPosition);
  }
}
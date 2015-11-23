#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
#include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
/*=========================================================================*/

Adafruit_BluefruitLE_UART ble(Serial1, BLUEFRUIT_UART_MODE_PIN);

//initialize variables for where things are plugged in
int sensor_pin = A10;
int heater_pin = 12; 
int button1 = 3;
int button2 = 9;
int button3 = 6;

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}


void setup(void)
{
  //while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  Serial.println(F("Then Enter characters to send to Bluefruit"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
    delay(500);
  }

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("******************************"));
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
    Serial.println(F("******************************"));
  }
  pinMode(heater_pin, OUTPUT);
  pinMode(sensor_pin, INPUT);
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  digitalWrite(heater_pin, HIGH);
}


void loop(void)
{
  // put your main code here, to run repeatedly

  //double sensor_reading=analogRead(sensor_pin);
  double sensor_reading = 0;
  double temperature = 0;
  double new_reading, new_temperature;
  for (int i = 0; i < 500; i++) {
    new_reading = analogRead(sensor_pin);
    new_reading = (new_reading * 3.3) / 1023;
    sensor_reading = sensor_reading + new_reading;
    new_temperature = ((new_reading * 1000) - 500) / 10;
    new_temperature = (new_temperature * (9 / 5)) + 32;
    temperature = temperature + new_temperature;
  }

  sensor_reading = sensor_reading / 500;
  temperature = temperature / 500;

  if (temperature < 34.0) {
    digitalWrite(heater_pin, LOW);
  }
  else if (temperature > 38.0) {
    digitalWrite(heater_pin, HIGH);
  }

  //
  if (digitalRead(button1) == HIGH) {
    ble.print("AT+BLEUARTTX=");
    ble.println('a');
    delay(100);

    Serial.println("button1");
  }
  else if (digitalRead(button2) == HIGH) {
    ble.print("AT+BLEUARTTX=");
    ble.println('c');
    Serial.println("button2");
    delay(100);
  }
  else if (digitalRead(button3) == HIGH) {
    ble.print("AT+BLEUARTTX=");
    ble.println('b');
    delay(100);
    Serial.println("button3");
  }  
  else {
    ble.print("AT+BLEUARTTX=");
    ble.println(' ');
    delay(100);
    Serial.println("no button");
  }

  if (! ble.waitForOK() ) {
    Serial.println(F("Failed to send?"));
  }
}



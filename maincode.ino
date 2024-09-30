#include <WiFi.h>
#include <Wire.h>
#include "OneWire.h"
#include "DallasTemperature.h"

// Pin for temp sensor
#define ONE_WIRE_BUS 4 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// WiFi setup
#define ssid "Cg 1.67"         
#define password "ek theke aat"

// Turbidity setup
int turbiditysensorPin = 34;

// pH Setup 
int PhSensorPin = 32;
float calibration_value = 18.34 + 0.1;
int buffer_arr[10], temp;
float ph_act;
unsigned long last_ph_time = 0;
const int ph_delay = 1000;

// Flow sensor setup
volatile int NumPulses = 0;
int PinSensor = 2;
float factor_conversion = 7.5;
float volume = 0;
long dt = 0;
long t0 = 0;

// Functions for flow sensor
void IRAM_ATTR PulseCount() {
  NumPulses++;  
}

int GetFrequency() {
  NumPulses = 0;
  interrupts();
  delay(1);  // Reduce blocking delay
  noInterrupts();
  return NumPulses;
}

void setup() {
  Wire.begin();
  Serial.begin(115200); // Increase baud rate for better performance

  analogReadResolution(12); // Set ADC resolution

  Serial.println("Connecting to WiFi...");
  int retries = 0;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && retries < 10) {
    delay(1000);
    Serial.println("Connecting...");
    retries++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to WiFi");
    // Handle WiFi failure case
  } else {
    Serial.println("Connected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }

  // Flow sensor setup
  pinMode(PinSensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(PinSensor), PulseCount, RISING);
  t0 = millis();
}

void loop() {

  // Turbidity sensor code
  int sensorValue = analogRead(turbiditysensorPin);

  // Map sensor value to range -100 to 100
  int turbidity = map(sensorValue, 0, 4095, -100, 100);

  // Constrain the value to stay within -100 to 100
  turbidity = constrain(turbidity, -10, 10);

  // pH sensor code
  if (millis() - last_ph_time > ph_delay) {
    for (int i = 0; i < 10; i++) {
      buffer_arr[i] = analogRead(PhSensorPin);
      delay(1);  // Shorten delay to avoid blocking
    }

    // Sort buffer array (Simple bubble sort)
    for (int i = 0; i < 9; i++) {
      for (int j = i + 1; j < 10; j++) {
        if (buffer_arr[i] > buffer_arr[j]) {
          temp = buffer_arr[i];
          buffer_arr[i] = buffer_arr[j];
          buffer_arr[j] = temp;
        }
      }
    }

    unsigned long int avgval = 0;
    for (int i = 2; i < 8; i++) {
      avgval += buffer_arr[i];
    }

    float volt = (float)avgval * 5 / 4096 / 6;
    ph_act = 5.70 * volt - calibration_value;

    last_ph_time = millis();  // Reset timing for next pH measurement
  }

  // Flow sensor code
  if (Serial.available()) {
    if (Serial.read() == 'r') volume = 0;
  }

  float frequency = GetFrequency();
  float flow_L_m = frequency / factor_conversion;
  dt = millis() - t0;
  t0 = millis();
  volume += (flow_L_m / 60) * (dt / 1000);

  // Temperature sensor loop
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  float tempF = sensors.getTempFByIndex(0);

  // Print all sensor values formatted
  Serial.print("Turbidity: ");
  Serial.print(turbidity);
  Serial.print(" NTU\tpH: ");
  Serial.print(ph_act, 2);
  Serial.print("\tFlow: ");
  Serial.print(flow_L_m, 3);
  Serial.print(" L/min\tVolume: ");
  Serial.print(volume, 3);
  Serial.println(" L");

  Serial.print("Temperature: ");
  Serial.print(tempC);
  Serial.print(" \xC2\xB0"); 
  Serial.print("C  |  ");

  Serial.print(tempF);
  Serial.print(" \xC2\xB0");
  Serial.println("F");

  delay(1000);  // Shorten delay
}

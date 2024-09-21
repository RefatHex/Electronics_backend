#include <WiFi.h>
#include <Wire.h>
//Wifi setup
#define ssid "Cg 1.67"         
#define password "ek theke aat"
//Turbidity setup
int turbiditysensorPin = 34;  
//Ph Setup 
int PhSensorPin=32;
float calibration_value = 18.34 + 0.1;
int phval = 0;
unsigned long int avgval;
int buffer_arr[10], temp;
float ph_act;

//Float sensor
volatile int NumPulses = 0;  // Variable for the number of pulses received 
int PinSensor = 2;  // Sensor connected to pin 2 
float factor_conversion = 7.5;  // To convert from frequency to flow rate 
float volume = 0; 
long dt = 0; // Time variation for each loop 
long t0 = 0; // Millis() from the previous loop 

//Functions for flow sensor
void IRAM_ATTR PulseCount() {  
  NumPulses++;   
} 

//---Function to obtain pulse frequency--------
int GetFrequency() { 
  int frequency; 
  NumPulses = 0;    
  interrupts();      
  delay(1000);       
  noInterrupts();   
  frequency = NumPulses; 
  return frequency; 
}


void setup() { 
  Wire.begin();
  Serial.begin(9600);

  analogReadResolution(12); 
  
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  //flow sensor setup
  pinMode(PinSensor, INPUT);  
  attachInterrupt(digitalPinToInterrupt(PinSensor), PulseCount, RISING);  
  Serial.println("Send 'r' to reset the volume to 0 Liters");  
  t0 = millis(); 

}

void loop() {
  //TURBIDITY code
  int sensorValue = analogRead(turbiditysensorPin);  
  int adjustedValue = sensorValue - 2047;
  int turbidity = map(adjustedValue, -2047, 2047, -100, 100); 
  Serial.println(turbidity);  
  delay(100);  

  //PH code
    for (int i = 0; i < 10; i++) {
    buffer_arr[i] = analogRead(PhSensorPin);  // Use GPIO 32
    delay(30);
  }

  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buffer_arr[i] > buffer_arr[j]) {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }

  avgval = 0;
  for (int i = 2; i < 8; i++) {
    avgval += buffer_arr[i];
  }

  float volt = (float)avgval * 3.3 / 4096 / 6; 
  
  ph_act = 5.70 * volt - calibration_value;

  Serial.println(ph_act);
  delay(1000);


  //flow sensor code
  if (Serial.available()) { 
    if (Serial.read() == 'r') volume = 0;
  } 
  float frequency = GetFrequency();  // Get the frequency of the pulses in Hz 
  float flow_L_m = frequency / factor_conversion;  // Calculate the flow in L/m 
  dt = millis() - t0; 
  t0 = millis(); 
  volume = volume + (flow_L_m / 60) * (dt / 1000);  // Volume (L) = flow (L/s) * time (s)

  Serial.print("Flow: ");  
  Serial.print(flow_L_m, 3);  
  Serial.print(" L/min\tVolume: ");  
  Serial.print(volume, 3);  
  Serial.println(" L"); 

}


def send_playload(){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin("refathex.pythonanywhere.com/add_to_excel");
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"key\":\"value\"}";

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.println("Error on sending POST: " + String(httpResponseCode));
    }

    http.end();
  }
}

}
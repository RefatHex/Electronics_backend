#include <SoftwareSerial.h>

/* Motor control pins (L293D) */
const int IN1 = 4;
const int IN2 = 5;
const int IN3 = 6;
const int IN4 = 7;
const int motorLeftSpeedPin = 9;
const int motorRightSpeedPin = 10;

/* Bluetooth setup */
SoftwareSerial BTSerial(2, 3); // RX, TX pins for Bluetooth

/* Mode: 0 for learning, 1 for replay, -1 for idle */
int mode = -1; // Initialize mode to -1 (idle) to stop motors at startup

/* Define maximum number of commands to store */
const int MAX_COMMANDS = 100;

/* Structure to store a command and its duration */
struct Command {
  char action;               // 'F', 'B', 'L', 'R'
  unsigned long duration;     // Duration in milliseconds
};

/* Array to store commands */
Command commandList[MAX_COMMANDS];
int commandCount = 0;

/* Variables for REPLAY mode */
int replayIndex = 0;
unsigned long commandStartTime = 0;
bool isReplaying = false;

void setup() {
  Serial.begin(9600);   // Serial monitor for debugging
  BTSerial.begin(9600); // Start Bluetooth communication
  
  /* Set motor pins as outputs */
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(motorLeftSpeedPin, OUTPUT);
  pinMode(motorRightSpeedPin, OUTPUT);

  /* Initialize motors to a stopped state */
  stopMotors();
}

void loop() {
  /* Check for incoming Bluetooth commands */
  if (BTSerial.available()) {
    String command = BTSerial.readStringUntil('\n'); // Read command from Bluetooth
    command.trim(); // Remove any trailing newline or carriage return
    Serial.print("Received command: ");
    Serial.println(command);
    
    /* Mode setup commands */
    if (command.equalsIgnoreCase("LEARN")) {
      mode = 0; // Set to learning mode
      commandCount = 0; // Reset command list
      Serial.println("Mode set to LEARN");
    } 
    else if (command.equalsIgnoreCase("REPLAY")) {
      if (commandCount == 0) {
        Serial.println("No commands to replay.");
      } else {
        mode = 1; // Set to replay mode
        replayIndex = 0;
        isReplaying = true;
        Serial.println("Mode set to REPLAY");
      }
    } 
    else if (command.equalsIgnoreCase("STOP")) {  // New command to stop the vehicle
      stopMotors();  // Call stop function to halt the motors
      mode = -1;     // Set mode to idle after stopping
      Serial.println("STOP command received. Vehicle stopped.");
    }
    else if (mode == 0) { // LEARN mode: store commands
      if (command.length() >= 2) {
        char action = toupper(command.charAt(0));
        /* Assume the command format is "F:duration", e.g., "F:1000" */
        int separatorIndex = command.indexOf(':');
        if (separatorIndex != -1) {
          String durationStr = command.substring(separatorIndex + 1);
          unsigned long duration = durationStr.toInt();
          if (commandCount < MAX_COMMANDS) {
            commandList[commandCount].action = action;
            commandList[commandCount].duration = duration;
            commandCount++;
            Serial.print("Stored command ");
            Serial.print(action);
            Serial.print(" with duration ");
            Serial.println(duration);
            executeCommand(action, duration); // Execute immediately during learning
          } else {
            Serial.println("Command list is full. Cannot store more commands.");
          }
        } else {
          Serial.println("Invalid command format. Use <Action>:<Duration>, e.g., F:1000");
        }
      } else {
        Serial.println("Invalid command. Use <Action>:<Duration>, e.g., F:1000");
      }
    }
  }

  /* Only run the motors if mode is not idle (-1) */
  if (mode == 0) { // Learning mode
    // Commands are handled above when received
  } 
  else if (mode == 1) { // Replay mode
    if (isReplaying && replayIndex < commandCount) {
      Command currentCommand = commandList[replayIndex];
      unsigned long currentTime = millis();
      
      if (commandStartTime == 0) {
        /* Start the command */
        executeCommand(currentCommand.action, currentCommand.duration);
        commandStartTime = currentTime;
        Serial.print("Replaying command ");
        Serial.println(currentCommand.action);
      }
      else {
        /* Check if the duration has passed */
        if (currentTime - commandStartTime >= currentCommand.duration) {
          /* Stop motors after the duration */
          stopMotors();
          replayIndex++;
          commandStartTime = 0;
        }
      }
    } 
    else if (isReplaying && replayIndex >= commandCount) {
      /* Finished replaying */
      mode = -1; // Set to idle
      isReplaying = false;
      Serial.println("Replay finished.");
    }
  }
}

/* Function to execute a command based on action and duration */
void executeCommand(char action, unsigned long duration) {
  switch(action) {
    case 'F': // Forward
      setLeftMotor(150, HIGH);
      setRightMotor(150, HIGH);
      break;
    case 'B': // Backward
      setLeftMotor(150, LOW);
      setRightMotor(150, LOW);
      break;
    case 'L': // Left
      setLeftMotor(0, LOW); // Stop left motor
      setRightMotor(150, HIGH); // Forward right motor
      break;
    case 'R': // Right
      setLeftMotor(150, HIGH); // Forward left motor
      setRightMotor(0, LOW); // Stop right motor
      break;
    default:
      Serial.print("Unknown action: ");
      Serial.println(action);
      break;
  }
}

/* Function to control left motor direction and speed */
void setLeftMotor(int speed, int direction) {
  if (direction == HIGH) {
    // Forward direction: IN1 = HIGH, IN2 = LOW
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } else {
    // Reverse direction: IN1 = LOW, IN2 = HIGH
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }
  analogWrite(motorLeftSpeedPin, speed); // Control speed via PWM

  /* Debugging */
  Serial.print("Left Motor - IN1: ");
  Serial.println(digitalRead(IN1));
  Serial.print("Left Motor - IN2: ");
  Serial.println(digitalRead(IN2));
  Serial.print("Left Motor Speed: ");
  Serial.println(speed);
}

/* Function to control right motor direction and speed */
void setRightMotor(int speed, int direction) {
  if (direction == HIGH) {
    // Forward direction: IN3 = HIGH, IN4 = LOW
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else {
    // Reverse direction: IN3 = LOW, IN4 = HIGH
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }
  analogWrite(motorRightSpeedPin, speed); // Control speed via PWM

  /* Debugging */
  Serial.print("Right Motor - IN3: ");
  Serial.println(digitalRead(IN3));
  Serial.print("Right Motor - IN4: ");
  Serial.println(digitalRead(IN4));
  Serial.print("Right Motor Speed: ");
  Serial.println(speed);
}

/* Function to stop the motors */
void stopMotors() {
  analogWrite(motorLeftSpeedPin, 0);  // Stop the left motor
  analogWrite(motorRightSpeedPin, 0); // Stop the right motor
  Serial.println("Motors stopped.");
}


#include <SerialCommands.h>
#include <Servo.h>

#define BUZZER 11

#define ENABLE 9
#define DIRA 8
#define DIRB 7

#define ENABLE2 6
#define DIRA2 4
#define DIRB2 5

char ReceiveBuffer[32];
SerialCommands serial_commands(&Serial, ReceiveBuffer, sizeof(ReceiveBuffer), "?", ":");

Servo vServo;
Servo hServo;
int vAngle = 90;
int hAngle = 90;
int vSpeed = 0;
int hSpeed = 0;
int updateTime = 0;

void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
}

// Change direction of one motor (L293D)
void setDir(int dir, int motor) {
    if (dir == 1) {
        if (motor == 1) {
          digitalWrite(DIRA, HIGH);
          digitalWrite(DIRB, LOW);
        } else if (motor == 2) {
          digitalWrite(DIRA2, HIGH);
          digitalWrite(DIRB2, LOW);
        }
    } else {
        if (motor == 1) {
          digitalWrite(DIRA, LOW);
          digitalWrite(DIRB, HIGH);
        } else if (motor == 2) {
          digitalWrite(DIRA2, LOW);
          digitalWrite(DIRB2, HIGH);
        }
    }
}

// Left joystick Callback
void set_speed(SerialCommands* sender)  { 
  int motor1Spd = atoi(sender->Next()); // 0-255
  int motor1Dir = atoi(sender->Next()); // 0: backwards, 1: forward
  int motor2Spd = atoi(sender->Next()); // 0-255
  int motor2Dir = atoi(sender->Next()); // 0: backwards, 1: forward

  analogWrite(ENABLE, motor1Spd);
  setDir(motor1Dir, 1);
  analogWrite(ENABLE2, motor2Spd);
  setDir(motor2Dir, 2);
}

// Right joystick Callback
void move_servo(SerialCommands* sender)  { 
  int spd = atoi(sender->Next()); // 0-10
  int dir = atoi(sender->Next()); // 0: stop, 1:up, 2:right, 3:down, 4:left

  // Servo speed adjustment based on the command.
  if (dir == 1) {
    vSpeed = spd;
    hSpeed = 0;
  } else if (dir == 2) {
    hSpeed = spd;
    vSpeed = 0;
  } else if (dir == 3) {
    vSpeed = -spd;
    hSpeed = 0;
  } else if (dir == 4) {
    hSpeed = -spd;
    vSpeed = 0;
  } else {
    vSpeed = 0;
    hSpeed = 0;
  }
}

// Right buttons Callback
void right_buttons(SerialCommands* sender) {
  int buttonIndex = atoi(sender->Next()); // 1: A, 2: B, 3: X, 4: Y

  // Your code goes here.
}

// Grab button Callback
void grab_object(SerialCommands* sender) {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

// Buzzer button Callback
void sound_buzzer(SerialCommands* sender) {
  digitalWrite(BUZZER, HIGH);
  delay(1000);
  digitalWrite(BUZZER, LOW);
}

SerialCommand set_speed_command("speed", set_speed);
SerialCommand grab_object_command("grab", grab_object);
SerialCommand sound_buzzer_command("play", sound_buzzer);
SerialCommand move_servo_command("servo", move_servo);
SerialCommand right_buttons_command("button", right_buttons);

void setup() {

  vServo.attach(A2);
  hServo.attach(A3);

  vServo.write(vAngle);
  hServo.write(hAngle);
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(ENABLE,OUTPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(DIRB,OUTPUT);
  
  pinMode(ENABLE2,OUTPUT);
  pinMode(DIRA2,OUTPUT);
  pinMode(DIRB2,OUTPUT);
  
  Serial.begin(115200);
  serial_commands.SetDefaultHandler(cmd_unrecognized);
  serial_commands.AddCommand(&set_speed_command);
  serial_commands.AddCommand(&grab_object_command);
  serial_commands.AddCommand(&sound_buzzer_command);
  serial_commands.AddCommand(&move_servo_command);
  serial_commands.AddCommand(&right_buttons_command);

  digitalWrite(ENABLE, LOW);
  digitalWrite(ENABLE2, LOW);

  digitalWrite(DIRA, HIGH);
  digitalWrite(DIRB, LOW);
  digitalWrite(DIRA2, HIGH);
  digitalWrite(DIRB2, LOW);
}

void loop() {
  // Read serial commands sent by the ESP32-CAM
  serial_commands.ReadSerial();

  // Update servo positions every 50 milliseconds.
  // Using millis to avoid blocking.
  if (updateTime <= millis()) {
    // Vertical movement
    if ((vSpeed > 0 && vAngle <= 180-vSpeed) || (vSpeed < 0 && vAngle >= abs(vSpeed))) {
      vAngle += vSpeed;
    }
    // Horizontal movement
    if ((hSpeed > 0 && hAngle <= 180-hSpeed) || (hSpeed < 0 && hAngle >= abs(hSpeed))) {
      hAngle += hSpeed;
    }
    vServo.write(vAngle);
    hServo.write(hAngle);
    updateTime = millis() + 50; 
  }
}

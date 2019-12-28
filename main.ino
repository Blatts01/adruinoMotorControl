//include differten libs
//DualTB9051FT lib 
#include "DualTB9051FTGMotorShield.h"

//SerialCommands
#include <Arduino.h>
#include "SerialCommands.h"

//Setup handler for input commands 
//max 32 chars
char serial_command_buffer_[32];

//Attenention the Line Feed ("LF") character (0x0A, \n) must be set to End of Line ("EOL") character (0x0D0A, \r\n) if windows is used for control 
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\n", " ");

//Setup motor shield pins and instance 
DualTB9051FTGMotorShield ms;

///////////////////////////////////////MOTOR SECTION///////////////////////////////////////

//Check motor current and H-Bride status 
//For savety reasons, programm ends in while loop if motor fault is detected
void stopIfFault()
{
  if (ms.getM1Fault())
  {
    Serial.println("M1 fault");
    while (1);
  }
}

//helper function pass motor set cmd and pwm value
bool set_motor1(int pwm)
{
  Serial.println("set_motor1: ");
  Serial.println(pwm);
  if (pwm <= 0)
  {
    Serial.println("Set PWM 0");
    ms.setM1Speed(0);
    stopIfFault();
    return 0;
  }
  else if (pwm <= 400)
  {
    Serial.println("Set PWM");
    ms.setM1Speed(pwm);
    stopIfFault();
    Serial.print("Mc: ");
    Serial.println(ms.getM1CurrentMilliamps());
    return 0;
  }  
  Serial.println("set_motor1: return 1 ");
}

///////////////////////////////////////MOTOR SECTION END ///////////////////////////////////////


///////////////////////////////////////SERIAL COMMAND SECTION///////////////////////////////////////
//Handel unknow or incomplete commands  
void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
}

//First parameter M1 value is required
//Optional parameters: pwm
//e.g. M1 200
void cmd_motor_set(SerialCommands* sender)
{
  char* pwm_str = sender->Next();
  if (pwm_str == NULL)
  {
    sender->GetSerial()->println("ERROR NO_PWM");
    return;
  }
  int pwm = atoi(pwm_str);
  sender->GetSerial()->println("Set pwm value: ");
  sender->GetSerial()->println(pwm);

  if(set_motor1(pwm));
  {
    sender->GetSerial()->println("ERROR MOTOR CONTROL");
    return;
  }
}

//Register Serial command function list (string compare)
SerialCommand cmd_motor_set_("M1", cmd_motor_set);

///////////////////////////////////////SERIAL COMMAND SECTION END///////////////////////////////////////

void setup() {
  //init serial device 115200 Baud
  Serial.begin(57600);

  //Init the motor shield 
  ms.init();
  
  //Enable the H-Bride for motorcontrol
  ms.enableDrivers();
  delay(1);       //ToDo remove after test 

  stopIfFault();

  //Enable Serial commands for motor control 
  serial_commands_.SetDefaultHandler(cmd_unrecognized);
  serial_commands_.AddCommand(&cmd_motor_set_);

  Serial.println("Motor Control Ready!");

}

void loop() {
  // Run serial command listener
  serial_commands_.ReadSerial();
}

//include differten libs
//DualTB9051FT lib
#include "DualTB9051FTGMotorShield.h"

//SerialCommands
#include <Arduino.h>
#include "SerialCommands.h"

//global varibales

//Setup handler for input commands
//max 32 chars
char serial_command_buffer_[32];

//Attenention the Line Feed ("LF") character (0x0A, \n) must be set to End of Line ("EOL") character (0x0D0A, \r\n) if windows is used for control
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\n", " ");

//Setup motor shield pins and instance
DualTB9051FTGMotorShield ms;

//Motor pwm interpolation variables
int g_interpol_step_count = 0;
float g_interpol_step_count_size = 0.000;

float g_y0 = 0.0;       //Y-Start point current pwm
float g_y1 = 0.0;             //Y-End point pwm
const float g_x0 = 0.0;             //X-Start point is always 0
float g_x1 = 0.0;             //X-End duration

//Global update rate for pwm values 1/s (Hz)
const int global_update_rate_Hz = 2;
//calc ms from update rate
const int global_update_rate = (1000 / global_update_rate_Hz);

///////////////////////////////////////MOTOR SECTION///////////////////////////////////////

///////////////////////////////////////HELP FUNCTION SECTION///////////////////////////////////////
void update_global_int(int current_pwm)
{
//  Serial.print("update_global_int: ");
//  Serial.println(current_pwm);
  g_y0 = current_pwm;
}

int check_boundaries(int pwm_value)
{
  int ret_value = pwm_value;
  if (pwm_value < 0)
  {
    Serial.print("check_boundaries set pwm to 0");
    ret_value = 0;
  }
  if (pwm_value > 400)
  {
    Serial.print("check_boundaries set pwm to 400");
    ret_value = 400;
  }

  return ret_value;
}

///////////////////////////////////////HELP FUNCTION SECTION END///////////////////////////////////////

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
//  Serial.println("set_motor1: ");
//  Serial.println(pwm);
  if (pwm <= 0)
  {
//    Serial.println("Set PWM 0");
    ms.setM1Speed(0);
    stopIfFault();
    update_global_int(0);
    return 0;
  }
  else if (pwm <= 400)
  {
//    Serial.println("Set PWM");
    ms.setM1Speed(pwm);
    stopIfFault();
//    Serial.print("Mc: ");
//    Serial.println(ms.getM1CurrentMilliamps());
    update_global_int(pwm);
    return 0;
  }
  Serial.println("set_motor1: return 1 ");
}

///////////////////////////////////////MOTOR SECTION END///////////////////////////////////////


///////////////////////////////////////INTERPOLATION SECTION///////////////////////////////////////

void calc_pwm_points(int end_point, int ramp_duration)
{

//  Serial.print("calc_pwm_points: ");

  g_interpol_step_count = 0;

  g_y1 = float(end_point);     //Y-End point
  g_x1 = float(ramp_duration); //X-End point pwm

  g_interpol_step_count_size = ramp_duration / calc_steps_from_sec(ramp_duration);

  Serial.print("g_y1: ");
  Serial.println(g_y1);
  Serial.print("g_x1: ");
  Serial.println(g_x1);
  Serial.print("g_interpol_step_count_size: ");
  Serial.println(g_interpol_step_count_size);
}

int get_new_pwm_value(void)
{
  float x = g_interpol_step_count_size * g_interpol_step_count;

//  Serial.print("g_y1: ");
//  Serial.println(g_y1);
//  Serial.print("g_x1: ");
//  Serial.println(g_x1);
//  Serial.print("g_interpol_step_count_size: ");
//  Serial.println(g_interpol_step_count_size);


  int pwm_value = (((g_y0 * (g_x1 - x)) + (g_y1 * x)) / g_x1);
  g_interpol_step_count = g_interpol_step_count + 1;
  Serial.print("get_new_pwm_value: ");
  Serial.println(pwm_value);
  Serial.print("g_interpol_step_count: ");
  Serial.println(g_interpol_step_count);

  return check_boundaries(pwm_value);
}

float calc_steps_from_sec(int sec)
{
  return (sec * 1000) / global_update_rate;
}

///////////////////////////////////////INTERPOLATION SECTION END///////////////////////////////////////


///////////////////////////////////////USER FUNCTION SECTION///////////////////////////////////////
bool start_program(int program_number)
{
  bool ret  = false;
  switch (program_number) {

    case 0:
      sender->GetSerial()->println("Run Program 0");
      ret = program_0();
    break;
  
    case 1:
      sender->GetSerial()->println("Run Program 1");
      ret = program_1();
    break;
  
    case 2:
      sender->GetSerial()->println("Run Program 2");
      ret = program_2();
    break;

    case 3:
      sender->GetSerial()->println("Run Program 3");
      ret = program_3();
    break;
  
    default:
      sender->GetSerial()->println("NO PROGRAM FOUND!");
      ret = true;
    break;
  }
  return ret;
}

//demo programm 0
bool program_0(void)
{
  bool ret  = false;
  ret = bool set_motor1(20);
  delay(1000);
  ret = bool set_motor1(40);
  delay(1000);
  ret = bool set_motor1(60);
  delay(1000);
  ret = bool set_motor1(40);
  delay(1000);
  ret = bool set_motor1(20);
  delay(1000);
  ret = bool set_motor1(0);
  return ret;
}


//demo programm 1
bool program_1(void)
{
  bool ret  = false;
  for (int i = 0; i <= 400; i++)
  {
    ret = bool set_motor1(i);
    delay(10);
  }
  return ret;
}


//demo programm 2
bool program_2(void)
{
  bool ret  = false;
  for (int i = 0; i <= 400; i++)
  {
    ret = bool set_motor1(i);
    delay(20);
  }

  for (int i = 400; i >= -400; i--)
  {
    ret = bool set_motor1(i);
    delay(20);
  }
  return ret;
}

//demo programm 3
bool program_3(void)
{  
  //RAMP 
  // PWM 0 to 100 in 5 sec. 
  int target_pwm = 100; 
  int ramp_time = 5;
  int current_pwm_value = 0;
  int steps = 0;
  bool ret  = false;

  //Set pwm to 0
  ret = bool set_motor1(0);

  //clac interpolation 
  steps = calc_steps_from_sec(ramp_time);
  calc_pwm_points(target_pwm, ramp_time);

  for (int i = 0; i <= steps; i++) {
    current_pwm_value = get_new_pwm_value();
    ret = set_motor1(current_pwm_value);
    delay(global_update_rate);
  }

  //RAMP 
  // PWM 100 to 0 in 10 sec. 
  target_pwm = 0; 
  ramp_time = 10;

  //clac interpolation 
  steps = calc_steps_from_sec(ramp_time);
  calc_pwm_points(target_pwm, ramp_time);

  for (int i = 0; i <= steps; i++) {
    current_pwm_value = get_new_pwm_value();
    ret = set_motor1(current_pwm_value);
    delay(global_update_rate);
  }
  
  return ret;
}
///////////////////////////////////////USER FUNCTION SECTION END///////////////////////////////////////


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

  if (set_motor1(pwm) == 1);
  {
    //sender->GetSerial()->println("ERROR MOTOR CONTROL");
    return;
  }
}

//First parameter LIN is required
//Parameters: pwm end point, duration
//e.g. LIN 300 2
void cmd_lin_interpol_set(SerialCommands* sender)
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

  char* duration_sec = sender->Next();
  if (duration_sec == NULL)
  {
    sender->GetSerial()->println("ERROR NO TIME SET");
    return;
  }
  int seconds = atoi(duration_sec);

  sender->GetSerial()->println("Set duration: ");
  sender->GetSerial()->println(seconds);

  int steps_current_run = calc_steps_from_sec(seconds);
  calc_pwm_points(pwm, seconds);

  sender->GetSerial()->println("START MOTOR LOOP LIN");
  int current_pwm_value = 0;
  for (int i = 0; i <= steps_current_run; i++) {
    current_pwm_value = get_new_pwm_value();
    bool return_value = set_motor1(current_pwm_value);
//    sender->GetSerial()->print("set_motor1(current_pwm_value) return: ");
//    sender->GetSerial()->println(return_value);
    delay(global_update_rate);
  }
  update_global_int(pwm);
  sender->GetSerial()->println("FINISH MOTOR LOOP LIN");
}

//First parameter P is required
//Optional parameters: Program number
//e.g. P 2
void cmd_program_set(SerialCommands* sender)
{
  char* program_str = sender->Next();
  if (program_str == NULL)
  {
    sender->GetSerial()->println("ERROR NO PROGRAM NUMBER!");
    return;
  }
  int program_number = atoi(program_str);
  sender->GetSerial()->print("Set program number: ");
  sender->GetSerial()->println(program_number);

  if (start_program(program_number) == true);
  {
    //sender->GetSerial()->println("ERROR MOTOR CONTROL");
    return;
  }
}

//Register Serial command function list (string compare)
SerialCommand cmd_motor_set_("M1", cmd_motor_set);
SerialCommand cmd_motor_lin_set_("LIN", cmd_lin_interpol_set);
SerialCommand cmd_program_set_("P", cmd_program_set);

///////////////////////////////////////SERIAL COMMAND SECTION END///////////////////////////////////////


void setup() {
  //init serial device 115200 Baud
  Serial.begin(115200);

  //Init the motor shield
  ms.init();

  //Enable the H-Bride for motorcontrol
  ms.enableDrivers();
  delay(1);       //ToDo remove after test

  stopIfFault();

  //Enable Serial commands for motor control
  serial_commands_.SetDefaultHandler(cmd_unrecognized);
  serial_commands_.AddCommand(&cmd_motor_set_);
  serial_commands_.AddCommand(&cmd_motor_lin_set_);
  serial_commands_.AddCommand(&cmd_program_set_);

  Serial.println("Motor Control Ready!");

}

void loop() {
  // Run serial command listener
  serial_commands_.ReadSerial();
}

// Derived from
// MultiStepper.pde
// -*- mode: C++ -*-
//
// Shows how to multiple simultaneous steppers
// Runs one stepper forwards and backwards, accelerating and decelerating
// at the limits. Runs other steppers at the same time
//
// Copyright (C) 2009 Mike McCauley
// $Id: MultiStepper.pde,v 1.1 2011/01/05 01:51:01 mikem Exp mikem $
// And from code from Xinyi...

#include <AccelStepper.h>
#include <math.h>
#include "LaserSetup.h"

// LASER CONTROL SETUP
const int laser_pins[16] = {-1,      // CONFIG 
                            -1,      // JOB
                            14,      // UP_ARROW
                            -1,      // DOWN_ARROW
                            -1,      // SET_HOME
                            -1,      // X_Y_OFF
                            15,      // GO
                            -1,      // STOP
                            -1,      // SPEED
                            -1,      // POWER
                            -1,      // SAVE
                            16,      // DEL
                            -1,      // FOCUS
                            -1,      // POINTER
                            -1,      // MAIN
                            17};     // RESET
int buz = A4;

// Paint control
const int zap_pin = 19;
bool zap_on = false;
const float fixed_distance = 200.0;


int rider_samples[100];

// Define some steppers and the pins the will use
const int LiftPulse = 6;
const int LiftDirection = 7;
const int SpindlePulse = 4;
const int SpindleDirection = 5;
const int RiderPulse = 2;
const int RiderDirection = 3;
const int LoaderEnable = 20;

AccelStepper Spindle(AccelStepper::DRIVER, SpindlePulse, SpindleDirection);
const float SpindleStepsPerTurn = 1400.0;
const float SpindleTurnPreWound = 4.0;

AccelStepper Rider(AccelStepper::DRIVER, RiderPulse, RiderDirection);
//const float RiderRadius = 13.52817016
//const float RiderRadius = 12.39695680382856; //grey fabric 
//const float RiderRadius = 12.62; //grey fabric 
const float RiderRadius = 12.2; // Blue Fabric

//const float RiderRadius = 11.375;
//const float RiderRadius = 13.65;
//const float RiderRadius = 11.375;
const float RiderPerimeter = 2*M_PI*RiderRadius;
//const float RiderRadius = 16.0;
const float RiderStepsPerTurn = 1600.0;
//const int StepsBetweenRiderSpeedUpdate = SpindleStepsPerTurn/8;
const int StepsBetweenRiderSpeedUpdate = 100;
const float RiderCalibrationDistance = 200.0;

AccelStepper Lift(AccelStepper::DRIVER, LiftPulse, LiftDirection);
const float LiftStepsPerMm = 1013;
//const float LiftStepsPerMm = 1022;
//const float LiftStepsPerMm = 1025.641026;

const float BaseSpeed = 300;

//const float MaterialThickness = 1.6; // Grey Felt in millimiter
//const float MaterialThickness = 1.5; // Grey Felt in millimiter
//const float MaterialThickness = .7; // Denim in millimiter
const float MaterialThickness = .55; // Blue Fabric in millimiter
//const float MaterialThickness = 1.5; // in millimiter
//float MaterialThickness = 1.455; // in millimiter
//float MaterialThickness = 1.325; // in millimiter


const int LiftStopSwitch = 10;
const int SpindleAlignedSwitch = 11;

void setup()
{      
  Serial.begin(9600);
  pinMode(LiftPulse, OUTPUT);
  pinMode(LiftDirection, OUTPUT);
  pinMode(SpindlePulse, OUTPUT);
  pinMode(SpindleDirection, OUTPUT);
  pinMode(RiderPulse, OUTPUT);
  pinMode(RiderDirection, OUTPUT);
  pinMode(LiftStopSwitch, INPUT);
  pinMode(SpindleAlignedSwitch, INPUT);
  pinMode(LoaderEnable, OUTPUT);
  digitalWrite(LoaderEnable, LOW);

  for (int i = 0; i< 100; i++)
  {
    rider_samples[i] = -1;
  }
  
  Spindle.setAcceleration(500000);
  //Spindle.setPinsInverted(true, false, false); // invert the standard direction
  Spindle.setPinsInverted(false, false, false); 

  Lift.setAcceleration(500000);
  Lift.setPinsInverted(false, false, false);

  Rider.setAcceleration(500000);
  Rider.setPinsInverted(false, false, false); // invert the standard directionl



  homing_all_axes();
  spooling_system_initialization();
}

void homing_all_axes()
{
  digitalWrite(LoaderEnable, HIGH);
  homing_lift();
  homing_spindle();
  homing_rider();
  digitalWrite(LoaderEnable, LOW);
}

void homing_lift()
{
  Lift.setMaxSpeed(4000.0);
  Lift.moveTo(-500000);

  while (digitalRead(LiftStopSwitch) == LOW)
  {
    Lift.run();
  }
  Lift.setCurrentPosition(0);
  Lift.setMaxSpeed(200.0);
  Lift.runToNewPosition(400);
  Lift.setMaxSpeed(200.0);
  Lift.moveTo(-1000);
  while (digitalRead(LiftStopSwitch) == LOW)
  {
    Lift.run();
  }
  Serial.print("Lift home position: ");
  Serial.println(Lift.currentPosition());
  Lift.setCurrentPosition(0);

  Lift.setMaxSpeed(2000.0);
//  Lift.move(50000);
//  while(Lift.run());
//  Serial.print("Lift 50000 steps:");
//  Serial.println(Lift.currentPosition());
//  while(true);
}

void homing_spindle()
{
  Spindle.setMaxSpeed(1000.0);
  Spindle.moveTo(-500000);

  while (digitalRead(SpindleAlignedSwitch) == LOW)
  {
    Spindle.run();
  }
  Spindle.setCurrentPosition(0);
  Spindle.setMaxSpeed(200.0);
  Spindle.runToNewPosition(+20);
  Spindle.setMaxSpeed(200.0);
  Spindle.moveTo(-40);
  while (digitalRead(SpindleAlignedSwitch) == LOW)
  {
    Spindle.run();
  }
  Serial.print("Spindle position: ");
  Serial.println(Spindle.currentPosition());
  Spindle.setCurrentPosition(0);

  Spindle.setMaxSpeed(2000.0);
//  Spindle.move(+1400);
//  while(Spindle.run());
//  Serial.print("SPindle 1400 steps:");
//  Serial.println(Spindle.currentPosition());
}

void homing_rider()
{
  Rider.setCurrentPosition(0);
}



float compute_rider_speed(float spindle_speed, float current_spindle_radius)
{
  return(spindle_speed*(RiderStepsPerTurn/SpindleStepsPerTurn)*(current_spindle_radius/RiderRadius));
}

float compute_lift_speed(float spindle_speed)
{
  float movement_ratio = (LiftStepsPerMm*MaterialThickness/SpindleStepsPerTurn);
  return(spindle_speed*movement_ratio);
}

float compute_fabric_lenght(float thickness, float number_of_turns)
{
  float total_angle = number_of_turns*2*M_PI; // radiant
  float fabric_lenght = .5*(thickness/(2*M_PI))*(total_angle*sqrt(1+total_angle*total_angle)+log(total_angle+sqrt(1+total_angle*total_angle)));
  return(fabric_lenght);
}

float compute_rider_travel(float thickness, float number_of_turns)
{
  // Take into account the diameter of the spindle
  float actual_fabric_lenght = compute_fabric_lenght(thickness, number_of_turns+SpindleTurnPreWound) - compute_fabric_lenght(thickness, SpindleTurnPreWound);
  return(RiderStepsPerTurn*(actual_fabric_lenght/(RiderPerimeter)));
}


void toogle_glue_spray()
{
  if (zap_on) 
  {
    digitalWrite(zap_pin, LOW);
    zap_on = false;
    Serial.println("zap off");
  } else
  {
    digitalWrite(zap_pin, HIGH);
    zap_on = true;
    Serial.println("zap on");
  }
}

void run_fabric_lenght(float distance_to_spool)
{
  bool not_done = true;
  int last_speed_update_position = -1;

  Spindle.setSpeed(BaseSpeed);
  Spindle.setMaxSpeed(BaseSpeed);
  
  Lift.setSpeed(compute_lift_speed(BaseSpeed));
  Lift.setMaxSpeed(compute_lift_speed(BaseSpeed));

  float new_speed = compute_rider_speed(BaseSpeed, ((Spindle.currentPosition() + StepsBetweenRiderSpeedUpdate/2)/(SpindleStepsPerTurn) + SpindleTurnPreWound)*MaterialThickness);
  //last_speed_update_position = Rider.currentPosition();
  Rider.setMaxSpeed(new_speed);
  Rider.setSpeed(new_speed);
  Serial.print("Rider speed: ");
  Serial.println(new_speed);
 
  Rider.move(RiderStepsPerTurn*(distance_to_spool/RiderPerimeter));
  while (Rider.distanceToGo() > 0)
  {
    if ((Spindle.currentPosition()%StepsBetweenRiderSpeedUpdate == 0) && (Spindle.currentPosition() != last_speed_update_position))
    {
      //float new_speed = compute_rider_speed(BaseSpeed, Lift.currentPosition()/(LiftStepsPerMm) + SpindleTurnPreWound*MaterialThickness);
      float new_speed = compute_rider_speed(BaseSpeed, ((Spindle.currentPosition() + StepsBetweenRiderSpeedUpdate/2)/(SpindleStepsPerTurn) + SpindleTurnPreWound)*MaterialThickness);
      last_speed_update_position = Spindle.currentPosition();
      Rider.setMaxSpeed(new_speed);
      Rider.setSpeed(new_speed);
      //rider_samples[Spindle.currentPosition()/StepsBetweenRiderSpeedUpdate] = Rider.currentPosition();
      Serial.write('*');
    }
    Spindle.runSpeed();
    Lift.runSpeed();
    Rider.runSpeed();
  }
  Serial.println();
  Serial.print("Spindle speed: ");
  Serial.print(Spindle.speed());
  Serial.print(" Spindle position: ");
  Serial.print(Spindle.currentPosition());
  Serial.print(" Fabric lenght: ");
  Serial.println(compute_fabric_lenght(MaterialThickness, Spindle.currentPosition()/SpindleStepsPerTurn));

  
  Serial.print("Lift speed: ");
  Serial.print(Lift.speed());
  Serial.print(" Lift position: ");
  Serial.println(Lift.currentPosition());

  Serial.print("Rider speed: ");
  Serial.print(Rider.speed());
  Serial.print(" Rider position: ");
  Serial.println(Rider.currentPosition());
}

void perform_laser_command(int command_code)
{
  switch (command_code) {
    case LASER_GO:
        Serial.println("Press \"Go\", and press any key when the action is completed...");
        while(Serial.available() == 0);
//        {
//          double bz = analogRead(buz);
//          while (bz > 10)
//          {
//            bz = analogRead(buz);
//          }
//  
//          Serial.println("buzzed");
//          delay(1000);
//        }
      break;
    case LASER_RESET:
        Serial.println("Press \"Reset\", and press any key when the action is completed...");
        while(Serial.available() == 0);
//        delay(4000);
      break;
    case LASER_UP_ARROW:
        Serial.println("Press \"Up Arrow\", and press any key when the action is completed...");
        while(Serial.available() == 0);
//        delay(4000);
      break;
    case LASER_DEL:
        Serial.println("Press \"Delete\", and press any key when the action is completed...");
        while(Serial.available() == 0);
      break;
    default:
      Serial.println("Not Implemented yet: nothing done!");    
  }

  // switchPin(laser_pins[command_code])

}

void spooling_system_initialization()
{
  Serial.println("Starting to spool...");
  int number_of_turn = 100;
   
  Serial.print("Spindle speed: ");
  Serial.println(BaseSpeed);
  Spindle.setSpeed(BaseSpeed);
  Spindle.setMaxSpeed(BaseSpeed);
  
  float lift_speed = compute_lift_speed(BaseSpeed);
  Serial.print("Lift speed: ");
  Serial.println(lift_speed);
  Lift.setSpeed(lift_speed);
  Lift.setMaxSpeed(lift_speed);

  //float rider_speed = compute_rider_speed(BaseSpeed, Lift.currentPosition()/LiftStepsPerMm + SpindleTurnPreWound*MaterialThickness);
  float rider_speed = compute_rider_speed(BaseSpeed, (Spindle.currentPosition()/(SpindleStepsPerTurn) + SpindleTurnPreWound)*MaterialThickness);
  Serial.print("Rider speed: ");
  Serial.println(rider_speed);
  Rider.setSpeed(rider_speed);
  Rider.setMaxSpeed(rider_speed);


  Spindle.move(SpindleStepsPerTurn*number_of_turn);
  Lift.move(SpindleStepsPerTurn*number_of_turn*(LiftStepsPerMm*MaterialThickness/SpindleStepsPerTurn));
  //Rider.move(compute_rider_travel(MaterialThickness, number_of_turn));
  Serial.println("System Initialized!");
}

void process_one_page(int page_lenght)
{
  static bool first_page = true;
  perform_laser_command(LASER_GO); // GO
  perform_laser_command(LASER_RESET); // RESET
  perform_laser_command(LASER_UP_ARROW); // UP
  perform_laser_command(LASER_DEL); // DELETE
  
  Serial.println("ready for stepping!");
  run_fabric_lenght(page_lenght);
  Serial.println("fixed stepped");
}

void manual_mode()
{
//  // s{1,2}{f,b}{number of steps 1 rev = 200} 
//  // eg. s1f100 == stp1 forward 100
//  
//  char m_stp = readNext();
//  char m_dir = readNext();
//  int m_steps = 8 * readNextInt();
//  
//  Serial.println("manually stepping:");
//  Serial.println(" stepper: " + String(m_stp) + " direction: " + String(m_dir) + " steps: " + String(m_steps));
//  
//  boolean set_speed = false;
//  
//  switch (m_dir) {
//    case 'f':
//      m_steps = -1 * m_steps;
//      break;
//    case 'b':
//      break;
//    case 's':
//      set_speed = true;
//      break;
//  }
//  
//  switch (m_stp) {
//    case '1':
//    {
//      if (set_speed) {
//        m_stp1_speed = m_steps;
//        Serial.println(" set stp 1 speed to " + String(m_steps));
//      } else {
//        stp1.move(-1 * m_steps);
//        stp1.setSpeed(m_stp1_speed);
//      }
//      break;
//    }
//    case '2': 
//    {
//      if (set_speed) {
//        m_stp2_speed = m_steps;
//        Serial.println(" set stp 2 speed to " + String(m_steps));
//      } else {
//        stp2.move(m_steps);
//        stp2.setSpeed(m_stp2_speed);
//      }
//      break;
//    }
//  }
//  break;
//  }
 
}

void loading_mode()
{
  const int movement_length = 20;

  digitalWrite(LoaderEnable, HIGH);
  Lift.setMaxSpeed(4000.0);
  Lift.move(LiftStepsPerMm*movement_length);
  while(Lift.distanceToGo() > 0)
  {
    Lift.run();
  }
  homing_spindle();
  
  Serial.println("Attach the fabric and press any key when done.");
  while(Serial.available() == 0);
  Serial.read();
  homing_lift();
  Serial.println("Tigthen material and press any key when done.");
  while(Serial.available() == 0);
  Serial.read();
  digitalWrite(LoaderEnable, LOW);
  spooling_system_initialization();
}

void unload_spool()
{
  const int movement_length = 10;
  
  digitalWrite(LoaderEnable, HIGH);
  Lift.setSpeed(20000.0);
  Lift.move(LiftStepsPerMm*movement_length);
  while(Lift.distanceToGo() > 0)
  {
    Lift.runSpeed();
  }

  Spindle.setSpeed(20000.0);
  Spindle.moveTo(0);
  while(Spindle.distanceToGo() > 0)
  {
    Spindle.runSpeed();
  }
  digitalWrite(LoaderEnable, LOW);
}

void one_turn_at_a_time()
{
  int number_of_turn = 1;
  bool not_done = true;
  int last_speed_update_position = -1;
   
  Spindle.setSpeed(BaseSpeed);
  Spindle.setMaxSpeed(BaseSpeed);
  
  Serial.print("Lift speed: ");
  Serial.println(compute_lift_speed(BaseSpeed));
  Lift.setSpeed(compute_lift_speed(BaseSpeed));
  Lift.setMaxSpeed(compute_lift_speed(BaseSpeed));

  float rider_speed = compute_rider_speed(BaseSpeed, ((Spindle.currentPosition() + StepsBetweenRiderSpeedUpdate/2)/(SpindleStepsPerTurn) + SpindleTurnPreWound)*MaterialThickness);

  Serial.print("Rider speed: ");
  Serial.println(rider_speed);
  Rider.setSpeed(rider_speed);
  Rider.setMaxSpeed(rider_speed);

  Spindle.move(SpindleStepsPerTurn*number_of_turn);
  Lift.move(number_of_turn*(LiftStepsPerMm*MaterialThickness));
  Rider.move(compute_rider_travel(MaterialThickness, number_of_turn));
  Serial.println("Starting to spool...");
  
  while (Spindle.distanceToGo() > 0)
  {
    if ((Spindle.currentPosition()%StepsBetweenRiderSpeedUpdate == 0) && (Spindle.currentPosition() != last_speed_update_position))
    {
      //float new_speed = compute_rider_speed(BaseSpeed, Lift.currentPosition()/(LiftStepsPerMm) + SpindleTurnPreWound*MaterialThickness);
      float new_speed = compute_rider_speed(BaseSpeed, ((Spindle.currentPosition() + StepsBetweenRiderSpeedUpdate/2)/(SpindleStepsPerTurn) + SpindleTurnPreWound)*MaterialThickness);
      last_speed_update_position = Spindle.currentPosition();
      Rider.setMaxSpeed(new_speed);
      Rider.setSpeed(new_speed);
      //rider_samples[Spindle.currentPosition()/StepsBetweenRiderSpeedUpdate] = Rider.currentPosition();
      Serial.write('*');
    }
    Spindle.runSpeed();
    Lift.runSpeed();
    Rider.runSpeed();
  }
  Serial.println();
  Serial.print("Spindle position: ");
  Serial.println(Spindle.currentPosition()/SpindleStepsPerTurn);
  Serial.print("Lift position: ");
  Serial.println(Lift.currentPosition()/(LiftStepsPerMm*MaterialThickness));
  Serial.print("Rider position (target): ");
  Serial.print(Rider.currentPosition());
  Serial.print(" (");
  Serial.print(compute_rider_travel(MaterialThickness, Spindle.currentPosition()/SpindleStepsPerTurn));
  Serial.println(")");
  Serial.println("Rider sampling (target\tactual): ");
  Serial.println();
//  for (int i = 0; i < (Spindle.currentPosition()/StepsBetweenRiderSpeedUpdate + SpindleTurnPreWound); i++)
//  {
//    Serial.print(i*0.125);
//    Serial.print("\t");
//    Serial.print(compute_rider_travel(MaterialThickness, i*0.125));
//    Serial.print("\t");
//    Serial.println(rider_samples[i]);
//  }
  

}

void loop() {
  if (Serial.available() > 0) {
    char ch = Serial.read();
    
    switch (ch) {
      case 'z':
      {
        toogle_glue_spray();
        break;
      }
      case 'p':
      {
        Serial.println("zapping once");
        digitalWrite(zap_pin, HIGH);
        delay(2000);
        digitalWrite(zap_pin, LOW);
        break;
      }
//      case 't':
//      {
//        Serial.print("Current thickness is: ");
//        Serial.println(MaterialThickness);
//        Serial.println("Enter new thickness: ");
//        MaterialThickness = Serial.parseFloat();
//        Serial.print("New thickness is: ");
//        Serial.println(MaterialThickness);
//        homing_all_axes();
//        spooling_system_initialization();
//        break;
//      }
//      case 'T':
//      {
//        Serial.print("Current thickness is: ");
//        Serial.println(MaterialThickness);
//        Serial.println("Enter new thickness: ");
//        MaterialThickness = Serial.parseFloat();
//        Serial.print("New thickness is: ");
//        Serial.println(MaterialThickness);
//        //homing_all_axes();
//        spooling_system_initialization();
//        break;
//      }

      case 'c':
      {
        run_fabric_lenght(RiderCalibrationDistance);
        Serial.print("Ran ");
        Serial.print(RiderCalibrationDistance);
        Serial.print("mm @ thickness:");
        Serial.println(MaterialThickness);
        break;
      }
      case 'C':
      {
         Rider.move(RiderStepsPerTurn*(RiderCalibrationDistance/RiderPerimeter));
         Rider.setSpeed(BaseSpeed);
         while (Rider.distanceToGo() != 0)
         {
           Rider.runSpeed();
         }
        Serial.print("Ran ");
        Serial.println(RiderCalibrationDistance);
        break;
      }
 
      case 'r':
      {
        one_turn_at_a_time();
        break;
      }
      case 'k':
      {
        digitalWrite(LoaderEnable, HIGH);
        process_one_page(fixed_distance);
        // runDistance(fixed_distance);
        // Serial.println("fixed stepped");
        break;
      }
      case 's': 
      {
        // manual stepping mode
        manual_mode();
        break;
      }

      case 'd':
      {
        Serial.println();
        Serial.print("Spindle position: ");
        Serial.println(Spindle.currentPosition()/SpindleStepsPerTurn);
        Serial.print("Lift position: ");
        Serial.println(Lift.currentPosition()/(LiftStepsPerMm*MaterialThickness));
        Serial.println();
        break;
      }

      case 'l':
      {
        loading_mode();
        break;
      }
      case 'h':
        homing_all_axes();
        spooling_system_initialization();
        break;
       case 'u':
        unload_spool();
        break;
       case 'P': //polishing
        Rider.setSpeed(10000);
        Rider.move(1000000);  
        while (Rider.distanceToGo() > 0)
        {
          Rider.runSpeed();
        }
        break;

   }
  }
}


/**
 * Author Teemu Mäntykallio
 * Initializes the library and runs the stepper
 * motor in alternating directions.
 */

#include <TMCStepper.h>
#include <AccelStepper.h>
#include <ezButton.h>

#define MS1 2       // Pin 25 connected to MS1 pin
#define MS2 3       // Pin 26 connected to MS2 pin
#define EN_PIN           4 // Enable
#define DIR_PIN          6 // Direction
#define STEP_PIN         5 // Step
#define SW_RX            40 // TMC2208/TMC2224 SoftwareSerial receive pin
#define SW_TX            41 // TMC2208/TMC2224 SoftwareSerial transmit pin
#define SERIAL_PORT Serial // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS 0b00 // TMC2209 Driver address according to MS1 and MS2
#define INC_AMOUNT 100

#define UP_PIN 7
#define DOWN_PIN 8
#define DIR_BTN_PIN 9

#define R_SENSE 0.11f // Match to your driver
                      // SilentStepStick series use 0.11
                      // UltiMachine Einsy and Archim2 boards use 0.2
                      // Panucatt BSD2660 uses 0.1
                      // Watterott TMC5160 uses 0.075
TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);

constexpr uint32_t steps_per_revolution = 12800;

AccelStepper stepper = AccelStepper(stepper.DRIVER, STEP_PIN, DIR_PIN);

ezButton* upButton;
ezButton* downButton;
ezButton* dirButton;
bool upPress = false;
bool downPress = false;
bool dirPress = false;
bool upRelease = false;
bool downRelease = false;
bool dirRelease = false;

int speed = 500;

void setup() {
  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);      // Enable driver in hardware
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);

  upButton = new ezButton(UP_PIN);
  downButton = new ezButton(DOWN_PIN);
  dirButton = new ezButton(DIR_BTN_PIN);
  
  upButton->setDebounceTime(50);
  downButton->setDebounceTime(50);
  dirButton->setDebounceTime(50);
  
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, HIGH);

                                  // Enable one according to your setup
//SPI.begin();                    // SPI drivers
//SERIAL_PORT.begin(115200);      // HW UART drivers
//driver.beginSerial(115200);     // SW UART drivers
  Serial.begin(115200);
  driver.begin();                 //  SPI: Init CS pins and possible SW SPI pins
                                  // UART: Init SW UART (if selected) with default 115200 baudrate
  driver.toff(5);                 // Enables driver in software
  driver.rms_current(600);        // Set motor RMS current
  driver.microsteps(16);          // Set microsteps to 1/16th

  driver.pwm_autoscale(true);     // Needed for stealthChop

  stepper.setMaxSpeed(steps_per_revolution); // 100mm/s @ 80 steps/mm
  stepper.setAcceleration(80000); // 2000mm/s^2
  stepper.setEnablePin(EN_PIN);
  stepper.setPinsInverted(false, false, true);
  stepper.enableOutputs();
}

bool left = false;

void loop() {
  readDownButton();
  readUpButton();
  readDirButton();

  if(dirRelease){
    left = !left;
  }

  if(downRelease){
    speed -= INC_AMOUNT;
    if(speed < 0){
      speed = 0;
    }
    Serial.println(speed);
  }else if(upRelease){
    speed += INC_AMOUNT;
    if(speed > steps_per_revolution){
      speed = steps_per_revolution;
    }
    Serial.println(speed);
  }

  if(left){
    stepper.setSpeed(-1 * speed);
  }else{
    stepper.setSpeed(speed);
  }

  stepper.runSpeed();
}

void readDownButton(){
  downButton->loop();
  downPress = false;
  downRelease = false;
  
  if(downButton->isPressed()){
    downPress = true;
  }

  if(downButton->isReleased()){
    //Do quick press action 
    downRelease = true;
  }
}

void readUpButton(){
  upButton->loop();
  upPress = false;
  upRelease = false;
  
  if(upButton->isPressed()){
    upPress = true;
  }

  if(upButton->isReleased()){
    //Do quick press action 
    upRelease = true;
  }
}

void readDirButton(){
  dirButton->loop();
  dirPress = false;
  dirRelease = false;
  
  if(dirButton->isPressed()){
    dirPress = true;
  }

  if(dirButton->isReleased()){
    //Do quick press action 
    dirRelease = true;
  }
}

//To Explore: During Auto Operation Mode, can Arduino nano safe the memory of the Duration Operation set

#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define PUMPPin 9
#define ValvePin 10
#define ManualPin 11
#define AutoPin 12

#define Password_Length 2  //8
char Data[Password_Length];
char Master[Password_Length] = "1";  //"123*456"
byte data_count = 0;
byte master_count = 0;
bool Pass_is_good;
char key;

byte ManualState;
byte AutoState;
bool isOpsMessageDisplayed = false;  // Flag to track if the message Manual/Auto has been displayed
bool confirmationReceived = false;
unsigned long prevTime, currTime, prevT_LCD;
unsigned long OnDuration = 5000, OffDuration = 5000;
bool AutoOpsFlag = false;
bool AutoMainOption = false;

bool isSeconds = true;     // Flag to track the current unit (seconds or minutes)
String inputNumber = "";   // Buffer to store user input
bool durationSet = false;  // Flag to check if a valid duration is set

// Define the number of rows and columns in the keypad
const byte ROWS = 4;
const byte COLS = 3;

// Define the keymap for the keypad
char keys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};

// Define the pins connected to the rows and columns
byte rowPins[ROWS] = { 3, 8, 7, 5 };
byte colPins[COLS] = { 4, 2, 6 };

// Create a Keypad object
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2);

void (*resetFunc)(void) = 0;  // create a standard reset function

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.init();       //Initialize LCD
  lcd.backlight();  //Light the LCD to be visible

  pinMode(PUMPPin, OUTPUT);
  pinMode(ValvePin, OUTPUT);
  pinMode(ManualPin, INPUT_PULLUP);
  pinMode(AutoPin, INPUT_PULLUP);


  delay(1000);
  //Display on Row 2 of LCD

  lcd.setCursor(0, 0);
  lcd.print("Initializing");
  //Display on Row 2 of LCD
  lcd.setCursor(0, 1);
  for (int i = 0; i < 4; i++) {
    lcd.setCursor(i, 1);
    lcd.print(".");
    delay(1000);
  }
  //delay(5000);
  lcd.clear();

  while (true) {
    lcd.setCursor(0, 0);
    lcd.print("Enter Password:");
    key = keypad.getKey();  //Read the key presssed

    if (key) {
      Data[data_count] = key;
      lcd.setCursor(data_count, 1);
      lcd.print(Data[data_count]);
      Serial.print(Data[data_count]);

      data_count++;
    }

    if (data_count == Password_Length - 1) {
      lcd.clear();
      Serial.println();
      if (!strcmp(Data, Master)) {
        lcd.print("Correct");
        Serial.println("Correct");
        delay(1500);
        break;

      } else {
        lcd.print("Incorrect");
        Serial.println("Incorrect");

        delay(2000);
      }

      lcd.clear();
      clearData();
    }
  }  //while true that password is incorrect

  lcd.clear();
  clearData();
  prevTime = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  ManualState = digitalRead(ManualPin);
  AutoState = digitalRead(AutoPin);
  // Serial.print("ManualPinState: ");
  // Serial.print(ManualState);
  // Serial.print("  AutoPinState: ");
  // Serial.println(AutoState);

  if (ManualState == 1 && AutoState == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Select Operation");
    lcd.setCursor(0, 1);
    lcd.print("Turn the Knob");

  }  //while (ManualState == 0 || AutoState == 0) {

  else {
    lcd.clear();

    //MANUAL OPERATION
    while (ManualState == 0 && AutoState == 1) {  //Manual Operation Selected

      //Display once with the false flag
      if (!isOpsMessageDisplayed) {
        lcd.setCursor(0, 0);
        lcd.print("Manual Operation");
        lcd.setCursor(0, 1);
        lcd.print("Selected ");
        delay(2000);
        lcd.clear();  //Clear LCD after delay

        isOpsMessageDisplayed = true;
      }  //End of Display

      lcd.setCursor(0, 0);
      lcd.print("Press 1:ON 2:OFF");
      lcd.setCursor(0, 1);
      lcd.print("*:RESET");

      key = keypad.getKey();  //Read the key presssed

      if (key) {  //Check if a key is pressed

        Serial.print("You pressed: ");
        Serial.println(key);  //Print the key to the Serial Monitor

        //Action after key pressed
        if (key == '1') {
          Serial.println("Turning ON Pump");
          digitalWrite(PUMPPin, HIGH);
          digitalWrite(ValvePin, LOW);

        } else if (key == '2') {
          Serial.println("Turning OFF Pump");
          digitalWrite(PUMPPin, LOW);
          digitalWrite(ValvePin, HIGH);


        } else if (key == '*') {
          lcd.clear();
          ResetConfirmation();
          // Wait for confirmation
        }
      }
      ManualState = digitalRead(ManualPin);

      //Check if ManualState pin has been turned;
      if (ManualState != 0) {
        //Make all state and process to its default
        isOpsMessageDisplayed = false;
        digitalWrite(PUMPPin, LOW);   // Turn off the pump
        digitalWrite(ValvePin, LOW);  // Turn off the valve


        break;
      }
      Serial.println("MANUAL MODE");
    }

    //AUTO OPERATION
    while (ManualState == 1 && AutoState == 0) {  //Auto Operation Selected
      currTime = millis();

      //Display once with the false flag
      if (!isOpsMessageDisplayed) {
        lcd.setCursor(0, 0);
        lcd.print("Auto Operation");
        lcd.setCursor(0, 1);
        lcd.print("Selected ");
        delay(2000);
        lcd.clear();  //Clear LCD after delay

        isOpsMessageDisplayed = true;
      }  //End of Display

      if (!AutoMainOption) {
        lcd.setCursor(0, 0);
        lcd.print("1:Set Active Time");
        lcd.setCursor(0, 1);
        lcd.print("2:Set Inactive Time");
        if (currTime - prevT_LCD >= 2000) {
          lcd.clear();
          AutoMainOption = true;
          prevT_LCD = currTime;
        }
      }

      else {
        lcd.setCursor(0, 0);
        lcd.print("0:Pause");
        lcd.setCursor(0, 1);
        lcd.print("*:RESET");
        if (currTime - prevT_LCD >= 2000) {
          lcd.clear();
          AutoMainOption = false;
          prevT_LCD = currTime;
        }
      }

      /////Direct Start and Switch every 5000second
      if (!AutoOpsFlag) {  //Change
        digitalWrite(PUMPPin, HIGH);
        digitalWrite(ValvePin, LOW);
        if (currTime - prevTime >= OnDuration) {  //Rules to Off the Pump (inside if statement) by changing the flag
          AutoOpsFlag = true;
          prevTime = currTime;
        }
      }

      else {

        digitalWrite(PUMPPin, LOW);
        digitalWrite(ValvePin, HIGH);
        if ((currTime - prevTime) >= OffDuration) {  //Rules to ON the Pump (inside if statement) by changing the flag
          AutoOpsFlag = false;
          prevTime = currTime;
        }
      }

      key = keypad.getKey();  //Read the key presssed

      if (key) {           //Check if a key is pressed
        if (key == '0') {  //AUTO OPERATION on hold
          confirmationReceived = false;
          digitalWrite(PUMPPin, LOW);
          digitalWrite(PUMPPin, LOW);
          while (!confirmationReceived) {
            char confirmKey = keypad.getKey();  // Wait for a new key press
            lcd.setCursor(0, 0);
            lcd.print("PAUSE..");
            lcd.setCursor(0, 1);
            lcd.print("#:TO CONTINUE");

            if (confirmKey == '#') {        // If user confirms reset
              lcd.clear();                  // Clear LCD First
              confirmationReceived = true;  // Exit the confirmation loop
            }
          }
        }

        else if (key == '1') {
          durationSet = false;
          lcd.clear();
          //   lcd.setCursor(0, 0);
          //   lcd.print("Set ON Duration:");
          inputNumber = "";  // Clear previous input

          while (true) {  // Loop to capture user input
            lcd.setCursor(0, 0);
            lcd.print("Set ON Duration:");
            char numKey = keypad.getKey();
            if (numKey >= '0' && numKey <= '9') {  // If a number is pressed
              inputNumber += numKey;               // Append the number to the input buffer
              // Clear row 1 to prevent leftover characters
              lcd.setCursor(0, 1);
              lcd.print("                ");  // Clear the row completely
              lcd.setCursor(0, 1);
              lcd.print(inputNumber + (isSeconds ? " s" : " min"));

            } else if (numKey == '*') {  // Toggle between seconds and minutes
              isSeconds = !isSeconds;
              // Clear row 1 to prevent leftover characters
              lcd.setCursor(0, 1);
              lcd.print("                ");  // Clear the row completely
              lcd.setCursor(0, 1);
              lcd.print(inputNumber + (isSeconds ? " s" : " min"));
            } else if (numKey == '#') {                                              // Confirm the input and exit
              if (inputNumber != "") {                                               // Ensure there's a valid input
                unsigned long duration = inputNumber.toInt();                        // Convert input to integer
                unsigned long tempDuration = duration * (isSeconds ? 1000 : 60000);  // Convert to milliseconds

                // Confirmation step
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Confirm: ");
                lcd.print(inputNumber + (isSeconds ? " s" : " min"));
                lcd.setCursor(0, 1);
                lcd.print("#:Start *:Edit");

                while (true) {  // Wait for confirmation
                  char confirmKey = keypad.getKey();
                  if (confirmKey == '#') {  // Start with confirmed duration
                    OnDuration = tempDuration;
                    durationSet = true;  // Mark duration as set
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("Duration Set To:");
                    lcd.setCursor(0, 1);
                    lcd.print(inputNumber + (isSeconds ? " s" : " min"));
                    delay(2000);  // Display the confirmation for 2 seconds
                    lcd.clear();
                    break;                         // Exit the confirmation loop
                  } else if (confirmKey == '*') {  // Re-enter the duration
                    inputNumber = "";              // Clear previous input
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("Re-enter Duration");
                    delay(1000);
                    lcd.clear();
                    break;  // Exit to re-enter duration
                  }
                }  //End of while confirmation loop

                // Exit the outer loop if durationSet is true
                if (durationSet) {
                  break;
                }
              }  //End of if statement of valid's input assurance
            }    //End of else if statement of #:Enter Exit
          }
        }

        else if (key == '2') {
        }

        else if (key == '*') {
          lcd.clear();
          //Wait for Confirmation Reset
          ResetConfirmation();
        }
      }

      AutoState = digitalRead(AutoPin);
      //Check if AutoState pin has been turned;
      if (AutoState != 0) {
        isOpsMessageDisplayed = false;
        digitalWrite(PUMPPin, LOW);
        digitalWrite(PUMPPin, LOW);

        break;
      }
    }
  }
}

void clearData() {
  while (data_count != 0) {
    Data[data_count--] = 0;
  }
  return;
}


void ResetConfirmation() {  //Wait Confirmation Function for RESET
  // Wait for confirmation
  confirmationReceived = false;
  while (!confirmationReceived) {
    char confirmKey = keypad.getKey();  // Wait for a new key press
    lcd.setCursor(0, 0);
    lcd.print("#:CONFIRM RESET");
    lcd.setCursor(0, 1);
    lcd.print("*:CANCEL RESET");

    if (confirmKey == '#') {  // If user confirms reset
      Serial.println("System resetting...");
      lcd.clear();  // Clear LCD First
      lcd.setCursor(0, 0);
      lcd.print("System resetting");
      for (int i = 0; i < 3; i++) {
        lcd.setCursor(i, 1);
        lcd.print(".");
        delay(1000);
      }
      digitalWrite(PUMPPin, LOW);   // Turn off the pump
      digitalWrite(ValvePin, LOW);  // Turn off the valve
      resetFunc();                  // Reset the system
      lcd.clear();
      confirmationReceived = true;  // Exit the confirmation loop
    }

    else if (confirmKey == '*') {  // If user cancels reset
      Serial.println("Reset canceled");
      lcd.clear();  // Clear LCD First
      lcd.setCursor(0, 0);
      lcd.print("RESET CANCELED");
      delay(1000);                  // Display the cancellation message
      lcd.clear();                  // Clear the LCD again
      confirmationReceived = true;  // Exit the confirmation loop
    }
  }
}
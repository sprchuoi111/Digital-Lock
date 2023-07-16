#include <SPI.h>
#include <Keypad.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROM.h>
//define max length
#define MAX_LENGTH_PASS 5
#define MAX_LENGTH_RFID 4

//intialize rfid

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN); // Create instance of the MFRC522 class
//10 (SS), 11 (MOSI), 12 (MISO), 13 (SCK)
/*
   Pin layout should be as follows:
   Signal     Pin              Pin
              Arduino Uno      MFRC522 board
   -----------------------------------------
   Reset      9                RST
   SPI SS     10               SDA
   SPI MOSI   11               MOSI
   SPI MISO   12               MISO
   SPI SCK    13               SCK
*/

//intialize lcd using i2c
LiquidCrystal_I2C lcd(0x27, 16, 2); //SCL A19 SDA A18

//intialize keypad
const byte rows = 4; //số hàng
const byte columns = 3; //số cột
char keys[rows][columns] =
{
  {'9', '8', '7'},
  {'6', '5', '4'},
  {'3', '2', '1'},
  {'#', '0', '*'},
};
// from R0 - > R3
byte rowPins[rows] = { 2, 3, 4, 5};
// from C0 -> C3
byte columnPins[columns] = {6, 7, 8};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rows, columns);

//intialize variable
byte mode = 0;
byte isDoorOpen = 0;
byte rfidChoose = 0;
byte wrongPassCount = 0;
byte wrongRFIDCount = 0;

unsigned long startTime = 0; // variable to store the start time of the timer
unsigned long duration = 10000; // set time to auto close door = 10s
unsigned long wrongWaiting = 10000;//set wait time if wrong pass = 10s

//intialize password 
//char pass[MAX_LENGTH_PASS] = {'0', '1', '2', '3', '4'};
char pass[MAX_LENGTH_PASS];
char newpass[MAX_LENGTH_PASS] = {' ', ' ', ' ', ' ', ' '};
char oldpass[MAX_LENGTH_PASS] = {' ', ' ', ' ', ' ', ' '};

//intialize rfid pass
//byte masterCard[MAX_LENGTH_RFID] = {138,121,206,14};   // Stores master card's ID read from EEPROM
//byte masterCard2[MAX_LENGTH_RFID] = {58,58,125,23};
byte masterCard[MAX_LENGTH_RFID];
byte masterCard2[MAX_LENGTH_RFID];

//intialize pin
const int buzzerPin = A0;
const int relayPin = A1;

//intialize function
void doorScreen(void);
void useKeypad(void);
void useRFID(void);
void wrongPass(void);
void wrongRFID(void);
void openDoor(void);
void changePass1(void);
void changePass2(void);
void changePass3(void);
void changeSelect(void);
void changeRFID(void);
void mainScreen(void);
void openSelect(void);
void rfidSelect(void);
void readRFID(void);

void readEEPROM(void);
void buzzer(byte mode);
void closeDoor();


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  SPI.begin();           // MFRC522 Hardware uses SPI protocol
  rfid.PCD_Init();    // Initialize MFRC522 Hardware
  //analogReference(EXTERNAL);
  pinMode(buzzerPin, OUTPUT); // Set buzzer pin as output
  pinMode(relayPin, OUTPUT); // Set buzzer pin as output
  digitalWrite(relayPin, HIGH);
  readEEPROM();
}

void loop() {
  switch (mode)
  {
    case 0: {
        doorScreen();
        break;
      }
    case 1: {
        useKeypad();
        break;
      }
    case 2: {
        useRFID();
        break;
      }
    case 3: {
        wrongPass();
        break;
      }
    case 4: {
        openDoor();
        break;
      }
    case 5: {
        changePass1();
        break;
      }
    case 6: {
        changePass2();
        break;
      }
    case 7: {
        changePass3();
        break;
      }
      case 8: {
        wrongRFID();
        break;
      }
      case 9: {
        changeSelect();
        break;
      }
      case 10: {
        changeRFID();
        break;
      }
      case 11: {
        mainScreen();
        break;
      }
      case 12: {
       openSelect();
        break;
      }
      case 13: {
        rfidSelect();
        break;
      }
      case 14:{
        readRFID();
        break;
      }
  }
}
// buzzer
void buzzer(byte time)
{
  for (byte i = 0; i < time ; i++)
  {
    digitalWrite(buzzerPin, HIGH); // Set buzzer tone to half of the maximum volume
    delay(200); // Wait for 0.2 second
    digitalWrite(buzzerPin, LOW); // Turn off buzzer
    delay(200); // Wait for 0.2 second
  }
  return;
}
//read EEPROM
void readEEPROM(void)
{
  if(EEPROM.read(0)== 0)
  {
    EEPROM.write(0, 1);
    //pass
    EEPROM.write(1, 0);
    EEPROM.write(2, 1);
    EEPROM.write(3, 2);
    EEPROM.write(4, 3);
    EEPROM.write(5, 4);
    //rfid1
    EEPROM.write(6, 138);
    EEPROM.write(7, 121);
    EEPROM.write(8, 206);
    EEPROM.write(9, 14);
    //rfid2
    EEPROM.write(10, 58);
    EEPROM.write(11, 58);
    EEPROM.write(12, 125);
    EEPROM.write(13, 23);    
  }
  for(int i = 0; i < MAX_LENGTH_PASS; i++)
  {
    pass[i] = EEPROM.read(i+1)+'0';
  }
  for(int j = 0 ; j < MAX_LENGTH_RFID; j++)
  {
    masterCard[j] = EEPROM.read(j+6);
    masterCard2[j] =  EEPROM.read(j+10);
  }
}

//=====================door Screen, mode = 0===============================//
void doorScreen(void)
{
  char key;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CHON CACH MO CUA");
  lcd.setCursor(0, 1);
  lcd.print("NHAN MODE");
  while (mode == 0)
  {
    key = keypad.getKey();
    if (key == '#') mode = 12;
  }
}
//====================open select, mode = 12======================//
void openSelect(void){
  char key;
  lcd.clear();
  lcd.print("1-NHAP MAT KHAU");
  lcd.setCursor(0, 1);
  lcd.print("2-MO BANG THE");
   while (mode == 12)
  {
    key = keypad.getKey();
    if (key)
    {
      if (key == '1')     mode = 1;
      else if (key == '2')  mode = 2;
    }
  }
  return mode;
}
//====================use Keypad, mode = 1========================//
void useKeypad(void)
{
  startTime = millis(); // store the start time of the timer
  unsigned long counter = 0;
  char user[5] = {'*', '*', '*', '*', '*'};
  char key, index = 0, isPassRight = 1;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("NHAP MAT KHAU");
  lcd.setCursor(0, 1);
  lcd.print("*****");
  while (counter < duration && mode == 1)
  {
    counter = millis() - startTime;
    if (counter >= duration) //khoa cua tu dong
    {
      mode = 0;
    }
    lcd.setCursor(index, 1);
    lcd.cursor();
    delay(30);
    key = keypad.getKey();  //check keypad
    if (key)
    {
      if (key == '#') //enter
      {
        for (byte i = 0; i < MAX_LENGTH_PASS; i++)
        {
          if (pass[i] != user[i])
          {
            isPassRight = 0;
            break;
          }
        }
        if (isPassRight == 0 )  mode = 3;
        else  mode = 4;
      }
      else if (key == '*') 
      {
        index = (index != 0) ? index - 1 : 4; // return        
      }
      else
      {
        user[index] = key;
        lcd.print(user[index]);
        index = (index != 4) ? index + 1 : 0; //next
      }
      counter = millis() - startTime;
    }
  }
  lcd.noCursor();
}
//======================read RFID, mode = 2==========================//
void useRFID(void)
{
  startTime = millis(); // store the start time of the timer
  unsigned long counter = 0;
  char key;
  byte isPassRight = 1;
  byte card1 = 1;
  byte card2 = 1;
  lcd.clear();
  lcd.print("QUET THE");
  while(counter < duration && mode == 2)
  {
    
    counter = millis() - startTime;
    key = keypad.getKey();
    if (counter >= duration) //khoa cua tu dong
    {
      mode = 0;
    }
    else if (key == '*')
    {
      mode = 12;
    }
    else if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    // Get the UID of the card
      for (byte i = 0; i < rfid.uid.size; i++) {
        if(rfid.uid.uidByte[i] != masterCard[i])
        {
          card1 = 0;
        }
        if(rfid.uid.uidByte[i] != masterCard2[i])
        {
          card2 = 0;
        }
        if(card1+card2 == 0)
        {
          isPassRight = 0;
          break;
        }
      }
      
      if(isPassRight == 0 ) mode = 8;
      else mode = 4; 
      rfid.PICC_HaltA(); // Halt the PICC (tag)
      rfid.PCD_StopCrypto1(); // Stop encryption on PCD (reader)
      counter = millis() - startTime;
    }
  }
}
//======================Open Door, mode = 4=======================//
void openDoor(void)
{
  startTime = millis(); // store the start time of the timer
  unsigned long counter = 0;
  wrongPassCount = 0;
  wrongRFIDCount = 0;
  char key;
  buzzer(1);
  delay(100);
  digitalWrite(relayPin, LOW);
  lcd.clear();
  lcd.print("MUNG BAN DA VE");
  lcd.setCursor(0, 1);
  lcd.print("CHON MODE");
  lcd.setCursor(0, 0);
  while (counter < duration && mode == 4)
  {
    counter = millis() - startTime;
    if (counter >= duration) //khoa cua tu dong
    {
      closeDoor();
    }
    key = keypad.getKey();
    if (key == '#') 
    {
      mode = 11;
    }
  }
}
//=========================Close Door=================================//
void closeDoor()
{
  buzzer(1);
  digitalWrite(relayPin, HIGH);
  //analogWrite(relayPin, 255);
  isDoorOpen = 0;
  delay(400);
  mode = 0;
}
//====================Main Screen, mode = 11========================//
void mainScreen(void)
{
  char key;
  startTime = millis(); // store the start time of the timer
  unsigned long counter = 0;
  lcd.clear();
  lcd.print("1-THAY DOI");
  lcd.setCursor(0, 1);
  lcd.print("2-KHOA CUA");
  while (mode == 11)
  {
    counter = millis() - startTime;
    if (counter >= duration) //khoa cua tu dong
        closeDoor();  
    key = keypad.getKey();
    if(key)
    {
      if(key == '1') mode = 9;
      else if (key == '2') closeDoor();
      if(key == '*') mode = 4;
    }
  }
}
//====================Change Select, mode = 9==========================//
void changeSelect(void)
{
  char key;
  lcd.clear();
  lcd.print("1-DOI MAT KHAU");
  lcd.setCursor(0, 1);
  lcd.print("2-DOI THE TU");
  while (mode == 9)
  {
    key = keypad.getKey();
    if (key)
    {
      if (key == '1') mode = 5;
      else if (key == '2') mode = 13; 
      else if(key == '*') mode = 11;
      
    }
  } 
}
//=====================Wrong Keypass, mode = 3========================//
void wrongPass(void)
{
  char key;
  lcd.clear();
  if(wrongPassCount == 2)
  {
    startTime = millis(); // store the start time of the timer
    unsigned long counter = 0;
    lcd.print("SAI PASS 3 LAN");
    digitalWrite(buzzerPin, HIGH);
    while(counter < wrongWaiting)
    {
     counter = millis() - startTime;
     if(counter >= wrongWaiting)
     {
        digitalWrite(buzzerPin, LOW);
        mode = 12;
        wrongPassCount  = 0;
     }
    }
  }
  else
  {
    wrongPassCount+=1;
    lcd.print("SAI MAT KHAU");  
    lcd.setCursor(0, 1);
    lcd.print("BACK <|EXIT MODE");
    buzzer(2);
    while (mode == 3)
    {
      key = keypad.getKey();   //check keypad
      if (key == '*')   mode = 1;
      else if(key == '#') mode = 12;
    }
  }
}
//=====================Wrong RFID, mode = 8 ======================//
void wrongRFID(void)
{
  char key;
  lcd.clear();
  
  if(wrongRFIDCount == 2)
  {
    startTime = millis(); // store the start time of the timer
    unsigned long counter = 0;
    lcd.print("SAI THE 3 LAN");
    digitalWrite(buzzerPin, HIGH);
    while(counter < wrongWaiting)
    {
     counter = millis() - startTime;
     if(counter >= wrongWaiting)
     {
      digitalWrite(buzzerPin, LOW);
      mode = 12;
      wrongRFIDCount = 0;
     }
    }
  }
  else
  {
    wrongRFIDCount+=1;
    lcd.print("SAI THE");
    lcd.setCursor(0, 1);
    lcd.print("BACK <|EXIT MODE");
    buzzer(2);
    while (mode == 8)
    {
      key = keypad.getKey();   //check keypad
      if (key == '*')   mode = 2;
      else if(key == '#') mode = 12;
    }
  }
}

//=====================Change Password, mode 5 -> 7===========================//
void changePass1()
{
  char key;
  byte isPassRight = 1, index = 0;
  lcd.clear();
  lcd.print("NHAP MK CU");
  lcd.cursor();
  while (mode == 5 )
  {
    lcd.setCursor(index, 1);
    key = keypad.getKey();   //check keypad
    if (key)
    {
      if (key == '*') index = (index != 0) ? index - 1 : 4; // return
      else if (key == '#')
      {
        for (byte i = 0; i < 5; i++)
        {
          if (pass[i] != oldpass[i])
          {
            isPassRight = 0;
            break;
          }
        }
        if (isPassRight == 0 )
        {
          isPassRight = 1;
          lcd.clear();
          lcd.print("SAI MAT KHAU");
          lcd.setCursor(0, 1);
          lcd.print("BACK <|EXIT MODE");
          buzzer(2);
          while(1)
          {
            key = keypad.getKey();   //check keypad
            if (key == '*')   break;
            else if(key == '#') 
            {
              mode = 11;
              break;
            }
          }
          lcd.clear();
          lcd.print("NHAP MK CU");
          index = 0;
        }
        else
        {
          for (byte i = 0; i < MAX_LENGTH_PASS; i++)
          {
            oldpass[i] = newpass[i];
          }
          mode = 6;
        }
      }
      else
      {
        oldpass[index] = key;
        lcd.print(oldpass[index]);
        index = (index != 4) ? index + 1 : 0; //next
      }
    }
  }
  lcd.noCursor();
}

void changePass2()
{
  char key;
  byte isItFill = 1;
  byte index = 0;
  lcd.clear();
  lcd.print("NHAP MK MOI");
  lcd.cursor();
  while (mode == 6 )
  {
    lcd.setCursor(index, 1);
    key = keypad.getKey();    //check keypad
    if (key)
    {
      if (key == '*') index = (index != 0) ? index - 1 : 4; // return
      else if (key == '#')    
      {
        for(byte i = 0; i < MAX_LENGTH_PASS; i++)
        {
          if(oldpass[i] == ' ')
          {
            isItFill = 0;
            break;
          }
        }
        if(isItFill == 1) mode = 7;
        else
        {
          isItFill = 1;
          lcd.setCursor(0,0);
          lcd.print("BAN CHUA NHAP DU");
          buzzer(2);
        }
      }

      else
      {
        oldpass[index] = key;
        lcd.print(oldpass[index]);
        index = (index != 4) ? index + 1 : 0; //next
      }
    }
  }
  lcd.noCursor();
}

void changePass3()
{
  char key;
  byte isPassRight = 1, index = 0;
  lcd.clear();
  lcd.print("NHAP LAI MK MOI");
  lcd.cursor();
  while (mode == 7)
  {
    lcd.setCursor(index, 1);
    key = keypad.getKey();   //check keypad
    if (key)
    {
      if (key == '*') index = (index != 0) ? index - 1 : 4; // return
      else if (key == '#')
      {
        for (byte i = 0; i < MAX_LENGTH_PASS; i++)
        {
          if (newpass[i] != oldpass[i])
          {
            isPassRight = 0;
            break;
          }
        }
        if (isPassRight == 0 )
        {
          isPassRight = 1;
          lcd.clear();
          lcd.print("SAI MAT KHAU");
          buzzer(2);
          lcd.print("BACK <|EXIT MODE");
          buzzer(2);
          while(1)
          {
            key = keypad.getKey();   //check keypad
            if (key == '*')   break;
            else if(key == '#') 
            {
              mode = 11;
              break;
            }
          }
          //delay(200);
          lcd.clear();
          lcd.print("NHAP LAI MK");
          index = 0;
        }
        else
        {
          lcd.clear();
          lcd.print("XAC NHAN DOI");
          lcd.setCursor(0,1);
          lcd.print("1-CO     2-KHONG");
          while(1)
          {
            key = keypad.getKey();
            if(key == '2') return 10;
            else if(key == '1')
            {
              lcd.clear();
              lcd.print("DA THAY DOI");
              for (byte i = 0; i < MAX_LENGTH_PASS; i++)
              {
                pass[i] = newpass[i];
                EEPROM.write(i+1,newpass[i]-'0');
              }
              delay(400);
              mode = 11;
              break;
            } 
          }  
           delay(400);              
        }
      }
      else
      {
        newpass[index] = key;
        lcd.print(newpass[index]);
        index = (index != 4) ? index + 1 : 0;
      }
    }
  }
  for (byte i = 0; i < 5; i++)
  {
    newpass[i] = ' ';
    oldpass[i] = ' ';
  }
  delay(100);
  lcd.noCursor();
}
//===================Select RFID, mode = 13========================//
void rfidSelect(void)
{
  char key;
  lcd.clear();
  lcd.print("1-THE 1  2-THE 2");
  lcd.setCursor(0,1);
  lcd.print("3-QUET THE");
  while(mode == 13)
  {
    key = keypad.getKey();   //check keypad
    if (key == '1')
    {
      rfidChoose = 6;
      mode = 10;
    }
    else if (key == '2')
    {
      rfidChoose = 10;
      mode = 10;
    }
    else if(key == '3')
    {
      mode = 14;
    }
    else if(key == '*') mode = 9;
  }
}
//=======================use RFID to read, mode = 14=========================//
void readRFID(void)
{
  byte card1 = 1;
  byte card2 = 1;
  char key;
  lcd.clear();
  lcd.print("QUET THE CAN DOI");
  delay(10);
  while(mode == 14)
  {
    key = key = keypad.getKey();
    if(key == '*') mode = 13;
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    // Get the UID of the card
      for (byte i = 0; i < rfid.uid.size; i++) {
        if(rfid.uid.uidByte[i] != masterCard[i])
        {
          card1 = 0;
        }
        if(rfid.uid.uidByte[i] != masterCard2[i])
        {
          card2 = 0;
        }
        if(card1 == 0 && card2 == 0)
        {
          break;
        }
      }
      if(card1 == 0 && card2 == 0) 
      {
        lcd.clear();
        lcd.print("BAN QUET SAI THE");
        buzzer(2);
        delay(400);
        lcd.clear();
        lcd.print("QUET THE CAN DOI");
        card1 = 1;
        card2 = 1;
      }
      else if(card1 == 1)
      {
        rfidChoose = 6;
        mode = 10;  
        lcd.clear();
        lcd.print("DOI THE 1");
        delay(1000);
      }
      else if(card2 == 1)
      {
        rfidChoose = 10;
        mode = 10;  
        lcd.clear();
        lcd.print("DOI THE 2");
        delay(1000);
      }
      rfid.PICC_HaltA(); // Halt the PICC (tag)
      rfid.PCD_StopCrypto1(); // Stop encryption on PCD (reader)
    }
  }
}
//===================Change RFID, mode = 10========================//
void changeRFID(void)
{
  byte isPassChange = 1;
  byte card1 = 1;
  byte card2 = 1;
  char key;
  lcd.clear();
  lcd.print("QUET THE MOI");
  while(mode == 10)
  {
    key = key = keypad.getKey();
    if(key == '*') mode = 13;
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    // Get the UID of the card
      for (byte i = 0; i < rfid.uid.size; i++) {
        if(rfid.uid.uidByte[i] == masterCard[i])
        {
          card1 = 0;
        }
        if(rfid.uid.uidByte[i] == masterCard2[i])
        {
          card2 = 0;
        }
        if(card1 == 0 || card2 == 0)
        {
          isPassChange = 0;
          break;
        }
      }
      if(isPassChange == 0 ) 
      {
        lcd.clear();
        lcd.print("BAN QUET THE CU");
        buzzer(2);
        delay(400);
        lcd.clear();
        lcd.print("QUET THE MOI");
        isPassChange = 1;
        card1 = 1;
        card2 = 1;
      }
      else 
      {
        lcd.clear();
        lcd.print("XAC NHAN DOI");
        lcd.setCursor(0,1);
        lcd.print("1-CO     2-KHONG");
        while(1)
        {
          key = keypad.getKey();
          if(key == '2') return 11;
          else if(key == '1')
          {
            lcd.clear();
            lcd.print("DA THAY DOI");
            for (byte i = 0; i < rfid.uid.size; i++) {
              masterCard[i] = rfid.uid.uidByte[i]; 
              EEPROM.write(i+rfidChoose,rfid.uid.uidByte[i]);
            }
            mode = 11;
            delay(1000);
            break;
          }
        }
        delay(400);    
      }
      rfid.PICC_HaltA(); // Halt the PICC (tag)
      rfid.PCD_StopCrypto1(); // Stop encryption on PCD (reader)
    }
  }
}

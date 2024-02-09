int tx = 1; //Pin for transmitting serial data
int rx = 0; //Pin for receiving serial data
int i,j; //Variables for 'for' loops
unsigned long time_to_read=1000; //Time for pause and wait for data to fully arrive
unsigned long startTime,elapsed_time,curr_time;//variables for millis() function
const int MAX_ENTRIES = 20; //Maximum size for List struct aka 'max password size'
const int password[] = {109,97,114,99,111};//ASCII Decimal 'marco'
const int passwordSize = 5;
bool refreshLCD;
const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
const int rolePerMinute = 15;         // Adjustable range of 28BYJ-48 stepper is 0~17 rpm
#include <LiquidCrystal.h>
#include <Stepper.h>
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
Stepper myStepper(stepsPerRevolution, 48, 50, 49, 51);
struct List
{
  int entries[MAX_ENTRIES];
  int count;  // Number of entries currently in the structure

  void init()
  {
    count = 0;
  }

  bool push(int value)
  {
    if (count < MAX_ENTRIES)
    {
      entries[count] = value;
      count++;
      return true;
    } else
    {
      return false;
    }
  }

  int get(int index)
  {
    if (index >= 0 && index < count)
    {
      return entries[index];
    } else
    {
      return -1;  //Not in ASCII table and out of bounds
    }
  }

  int deleteEntry(int index)
  {
    if (index >= 0 && index < count)
    { 
      int temp = entries[i];
      for (int i = index; i < count - 1; i++)
      {
        entries[i] = entries[i + 1];
      }
      count--;
      return temp;
    } else
    {
      return -1;
    }
  }

  bool isEmpty()
  {
    return count==0;
  }

  int pop()
  {
    if(!isEmpty())
    { 
      int temp = entries[count-1];
      count --;
      return temp;
    }
    else
    {
      return -1;
    }
  }

  void clear(){count=0;}

  int size(){return count;}

};

List buffer;

void setup()
{
  //Initialization of Serial Interface
  Serial.begin(9600);
  pinMode(tx, OUTPUT);
  pinMode(rx, INPUT);
  //Initialization of List
  buffer.init();
  //Initialization of LCD
  lcd.begin(16, 2);
  lcd.print("Booting");
  refreshLCD=true;
  //Initialization of Stepper
  myStepper.setSpeed(rolePerMinute);


}

void loop()
{

  if(refreshLCD) //Constant Function Calling if no data to consume
  {
    lcd.clear();
    lcd.print("Enter Password");
    refreshLCD=false;
  }

  if(consume()) //Checks to see if there is data on the Serial comms
  {
    cleanBuffer(); //Removes garbage data from the buffer

    if(isPassword()) //User entered the correct Password
    {
      lcd.clear();
      lcd.print("Correct Password");
      Serial.println("Correct Password");
      unlock(); //Function to perform on correct password entered
      buffer.clear(); //Deletes the data structure, making it ready for the next attempt
      delay(2000);
    }
    else//Wrong password entered
    {
      lcd.clear();
      lcdPrintWrap("Incorrect Password",0,0);
      Serial.println("Inorrect Password");
      buffer.clear(); //Deletes the data structure, making it ready for the next attempt
      delay(2000);
    }
    refreshLCD=true;
  }

}

bool consume() //Returns false if nothing was consumed
{ 
  while(Serial.available() > 0)//When the first byte arrives
  {
    startTime = millis();
    curr_time = millis();
    elapsed_time = curr_time - startTime;

    while(elapsed_time < time_to_read)//Wait for all the data to arrive. BT comms is slower than CPU
    {
      curr_time = millis();
      elapsed_time = curr_time - startTime;
    }

    while(Serial.available()>0)//Some time passed after first byte was received
    {
      int temp = Serial.read();//Read byte
      if(temp != -1)//Only push valid ASCII characters
      {
        buffer.push(temp);
      }
    }

  }

  if(buffer.isEmpty())
  {
    return false;
  }

  return true;
}

bool cleanBuffer() //Function to delete non viewable ascii characters and everything after; returns true on success
{
  int temp = searchEnd();//Find the end of the usable characters. Returns -1 on fail
  if(temp == -1){return false;}
  while(buffer.count > temp)//delete garbage data after ascii carriage return
  {
    buffer.pop();
  }
  return true;
}

int searchEnd()//Searches for the end of transmission. Defined by ASCII 13, returns -1 if not found
{
  for(i=0;i<(buffer.size()-1);i++)
  {
    int temp = buffer.get(i);
    if(temp==(13))//ASCI Character 13 is the end of usable transmission data
    {
      return i;
    }
  }
    return -1;
}

bool isPassword()//Returns true on correct password
{ 
  if(buffer.count!=passwordSize){buffer.clear();return false;}//Returns false of password are not the same size

  for(i=0;i<passwordSize;i++)
  {
    if((buffer.get(i))!=(password[i]))
    {
      return false;
    }
  }
  return true;
}

void unlock()//Function to perform after correct password was validated
{
  Serial.println("unlock");
  lcd.clear();
  lcd.print("Unlocking device");
  myStepper.setSpeed(stepsPerRevolution);
  delay(1000);
  myStepper.setSpeed(-stepsPerRevolution);
  delay(1000);
}

void lcdPrintWrap(const String &text, int col, int row) {
  int maxLength = 16;
  int length = text.length();
  String s = text.substring(16);
  lcd.print(text);
  lcd.setCursor(0, 1);
  lcd.print(s);
}

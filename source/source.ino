int tx = 1; //Pin for transmitting serial data
int rx = 0; //Pin for receiving serial data
int i,j; //Variables for 'for' loops
unsigned long time_to_read=1000; //Time for pause and wait for data to fully arrive
unsigned long hora_cuando_empeze_a_medir,elapsed_time,curr_time;//variables for millis() function
const int MAX_ENTRIES = 20; //Maximum size for List struct aka 'max password size'
const int password[] = {109,97,114,99,111};//ASCII Decimal 'marco'
const int passwordSize = 5;

#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

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
  Serial.begin(9600);
  pinMode(tx, OUTPUT);
  pinMode(rx, INPUT);
  buffer.init();
  lcd.begin(16, 2);
  lcd.print("Booting");
}

void loop()
{
  lcd.clear();
  lcd.print("Enter Password");


  if(consume())
  {
    cleanBuffer(); //Removes garbage data

    lcd.clear();
    lcd.print("Password Entered");
    lcd.setCursor(0, 1);
    lcd.print(bufferToString());

    if(isPassword())
    {
      lcd.clear();
      lcd.print("Unlock");
      unlock();
      buffer.clear();
    }
    else
    {
      buffer.clear();
    }
  delay(2000);
  }
}

bool consume() //Returns false if nothing was consumed
{ 
  while(Serial.available() > 0)//When the first byte arrives
  {
    hora_cuando_empeze_a_medir = millis();
    curr_time = millis();
    elapsed_time = curr_time - hora_cuando_empeze_a_medir;
    while(elapsed_time < time_to_read)//Wait for all the data to arrive. BT comms is slower than CPU
    {
      curr_time = millis();
      elapsed_time = curr_time - hora_cuando_empeze_a_medir;
    }
    while(Serial.available()>0)//Gave BT time, now should be OK to read fast
    {
      int temp = Serial.read();//Read byte
      if(temp != -1)//if no data available, dont push to buffer
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
  int temp = searchEnd();//Find the end of the usable characters
  if(temp == -1){return false;}
  while(buffer.count>temp)//delete garbage data after ascii carriage return
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
    if(temp==(13))
    {
      return i;
    }
  }
    return -1;
}

bool isPassword()//Returns true on correct password
{ 
  if(buffer.count!=passwordSize){buffer.clear();return false;}
  for(i=0;i<passwordSize;i++)
  {
    if((buffer.get(i))!=(password[i]))
    {
      return false;
    }
  }
  return true;
}

void unlock()//TO-DO
{
  Serial.println("unlock");
}

String bufferToString()
{
  char content[buffer.size()+1];
  for(i=0;i<buffer.size();i++){
    content[i]=buffer.get(i);
  }
  content[buffer.size()]=0;
  String s = content;
}



//Please notice the range for user is from 0xE0000 to 0xE1000. Size is 4096 bytes.

#include <Arduino.h>
#include "Flash.h"

Flash flash_handle;

#define ARRAY_NUM 7

void setup()
{
  Serial.begin(115200);
  while (!Serial){}

  Serial.println("================================");
  Serial.println("Flash Test");
  Serial.println("================================");
}

uint8_t data[ARRAY_NUM] = {0x11,0x22,0x33,0x44,0x55,0x66,0x77};
uint8_t data2[ARRAY_NUM] = {0};
void print_log()
{
  int i =0;
  for(i;i<ARRAY_NUM;i++)
  {
      Serial.print(data2[i],HEX);
      Serial.print(" ");
  }
}

void loop()
{
   memset(data2,0,ARRAY_NUM);
   Serial.println("Before");
   print_log();   
   flash_handle.write(data,ARRAY_NUM);
   delay(1000);
   flash_handle.read(data2,ARRAY_NUM);
   Serial.println("After");
   print_log();
   delay(1000);
}
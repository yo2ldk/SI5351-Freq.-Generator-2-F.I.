/*
 Module Nano Pro Digital Pin Allocation
 
 D0
 D1  
 D2  LCD RS 
 D3  LCD enable 
 D4  LCD DB4
 D5  LCD DB5
 D6  LCD DB6 
 D7  LCD DB7
 D8  
 D9  
 D10 Rotary encoder pin A 
 D11 Rotary encoder pin B 
 D12 Frequency resolution button 
 D13 Band Select button
 A0/D14 
 A1/D15 Decrease frequency button 
 A2/D16 Increase frequency button 
 A3/D17 Offset enable                
 A4/D18 Si5351 SDA
 A5/D19 Si5351 SCL 
 
 ----------------------------------------------------------------
*/
const unsigned long Freq_array [] [3] = {
  { 100000,28800000,10000000  },  
  { 1810000,28800000,10000000  },            
  { 3500000,28800000,10000000  },
  { 7000000,28800000,10000000  },
  { 10100000,28800000,10000000 },
  { 14000000,28800000,10000000 },
  { 18090000,28800000,10000000 },
  { 21000000,28800000,10000000 },
  { 24900000,28800000,10000000 },
  { 28000000,28800000,10000000 },
  { 50000000,28800000,10000000 },
  { 70000000,28800000,10000000 },
  { 80000000,28800000,10000000 },
  { 90000000,28800000,10000000 },
  { 100000000,28800000,10000000},
  { 110000000,28800000,10000000},
  { 120000000,28800000,10000000},
  { 130000000,28800000,10000000},
  { 140000000,28800000,10000000},
  { 150000000,28800000,10000000},
  (0,0,0)
  };
int fOffset = 000;
int CalFactor = 0;
#include <LiquidCrystal.h>
#include <string.h>
#include <ctype.h>
#include <avr/interrupt.h>  
#include <avr/io.h>
#include <Wire.h>
#define encoderPinA              10   //3
#define encoderPinB              11   //4
#define Resolution               12   //6
#define RS                       2    //7
#define E                        3    //8
#define DB4                      4    //9
#define DB5                      5    //10
#define DB6                      6    //11
#define DB7                      7    //12
#define FreqDown                 A1   //A0
#define FreqUp                   A2   //A1
#define Offset                   A3   //A2
#define BandSelect               9
#define Si5351A_addr             0x60 
#define CLK_ENABLE_CONTROL       3
#define CLK0_CONTROL             16 
#define CLK1_CONTROL             17
#define CLK2_CONTROL             18
#define SYNTH_PLL_A              26
#define SYNTH_PLL_B              34
#define SYNTH_MS_0               42
#define SYNTH_MS_1               50
#define SYNTH_MS_2               58
#define PLL_RESET                177
#define XTAL_LOAD_CAP            183




LiquidCrystal lcd(RS, E, DB4, DB5, DB6, DB7);


byte fStepcount,offsetFlag=0,band;
String resolution = "  1    Hz  ";
unsigned int tcount=2,encoderA,encoderB,encoderC=1;
unsigned long time,fStep=1,XtalFreq=25000000;
unsigned long mult=0,Freq_1,Freq_2,debounce,DebounceDelay=500000;
ISR(TIMER1_OVF_vect) 
{
  mult++;                                          
  TIFR1 = (1<<TOV1);                              
}



void setup(){

lcd.begin(24,2);

for (int i = 0, k = 0, n = 3; i <= 100; i++){   // k-cursor, n= % / nr. char display; 
    lcd.setCursor(9,0);
    if (i<100) lcd.print(" ");  //print a space if the percentage is < 100 
    if (i<10) lcd.print(" ");  //print a space if the percentage is < 10
    lcd.print(i);
    lcd.print(" %");
    //
    n--; 
    if (n == 0){
     lcd.setCursor(k,1);
     lcd.print("*|-->");
     k++;
     n = 4; 
    }
    //
    delay(50);  //change the delay to change how fast the boot up screen changes 
  }




  
lcd.begin(24,2);
lcd.clear();
lcd.setCursor(2,0);
lcd.print("Variable RF Generator");
lcd.setCursor(7,1);
lcd.print("- YO2LDK -");
  delay(3000);
  lcd.clear();




 
  
  Wire.begin(1);                 
  si5351aStart();
  TCCR1B = 0;                                    
  TCCR1A = 0;                                   
  TCNT1  = 0;                                  
  TIFR1  = 1;                                  
  TIMSK1 = 1;                                   
  {
    XtalFreq += CalFactor;
    detachInterrupt(0); 
  } 
  XtalFreq *= 4;
    lcd.begin(24,2); 
  pinMode(encoderPinA, INPUT);
  digitalWrite(encoderPinA, HIGH);         
  pinMode(encoderPinB, INPUT);
  digitalWrite(encoderPinB, HIGH);         
  pinMode(Resolution, INPUT);
  digitalWrite(Resolution, HIGH);        
  pinMode(BandSelect, INPUT);
  digitalWrite(BandSelect, HIGH);         
  pinMode(FreqDown, INPUT);
  digitalWrite(FreqDown, HIGH);          
  pinMode(Offset, INPUT);
  digitalWrite(Offset, HIGH);            
  pinMode(FreqUp, INPUT);
  digitalWrite(FreqUp, HIGH);            
  pinMode(FreqDown, INPUT);
  digitalWrite(FreqDown, HIGH);  

           
  lcd.display();                          
  lcd.setCursor(0,1);
  lcd.print("Step =");                   
  lcd.setCursor(15,1);                   
  lcd.print(resolution);                
 
  TCCR1B = 0;                             
  Freq_1 = Freq_array [0] [0];            
  Freq_2 = Freq_array [0] [1];        
  si5351aSetFreq(SYNTH_MS_0,10000000); 
  if(Freq_2 == 0)
  {
    Si5351_write(CLK_ENABLE_CONTROL,0b00000100);
  }
  else
  {
    Si5351_write(CLK_ENABLE_CONTROL,0b00000000); 
    Freq_2 = Freq_array [band] [1];             
    si5351aSetFreq(SYNTH_MS_2, Freq_2);          
  }
  setfreq();                                   
}
void loop()
{   
  {
    if(tcount==1)                                  
    { 
      si5351aSetFreq(SYNTH_MS_1, Freq_1);
      if(Freq_2 > 0) si5351aSetFreq(SYNTH_MS_2, Freq_2);
      tcount=2;
    }
    byte encoderA = digitalRead(encoderPinA); 
    byte encoderB = digitalRead(encoderPinB);
    if ((encoderA == HIGH) && (encoderC == LOW))
    {
      if (encoderB == LOW)
      {       
        Freq_1 -= fStep; 
      }
      else
      {        
        Freq_1 += fStep;
      }
      setfreq();         
      resDisplay();     
    }
    encoderC = encoderA;
    if(digitalRead(Resolution) == LOW)
    {
      for(debounce=0; debounce < DebounceDelay; debounce++) {
      };
      fStepcount++;
      if(fStepcount>9)fStepcount=0;
      setResolution();    
    }
    if(digitalRead(BandSelect) == LOW)
    {
      for(debounce=0; debounce < DebounceDelay; debounce++) {
      };
      band=band+1;                        
      if(Freq_array [band] [0]==0)band=0; 
      if(Freq_array [band] [1] == 0)     
      {
        Si5351_write(CLK_ENABLE_CONTROL,0b00000100); 
      }
      else
      {
        Si5351_write(CLK_ENABLE_CONTROL,0b00000000); 
        Freq_2 = Freq_array [band] [1];              
        si5351aSetFreq(SYNTH_MS_2, Freq_2);         
      }
      Freq_1 = Freq_array [band] [0];                
      setfreq();                                    
    } 
    if(digitalRead(Offset) == LOW && offsetFlag == 0) 
    { 
      offsetFlag = 1;                                
      Freq_1 += fOffset;                              
      lcd.setCursor(15,0);                            
      lcd.print("*");
      setfreq();                                    
    }
    if(digitalRead(Offset) == HIGH && offsetFlag == 1) 
    {
      offsetFlag = 0;                                  
      Freq_1 -= fOffset;                               
      lcd.setCursor(15,0);                             
      lcd.print("  ");
      setfreq();                                       
    }
    if(digitalRead(FreqUp) == LOW)  
    {
      for(debounce=0; debounce < DebounceDelay; debounce++) {
      };
     
      Freq_1 += fStep;                                  
      setfreq();                                      
      resDisplay();                                   
    }
    if(digitalRead(FreqDown) == LOW) 
    {
      for(debounce=0; debounce < DebounceDelay; debounce++) {
      };
      Freq_1 -= fStep;                               
      setfreq();                                     
      resDisplay();                                 
    }     
  }
}

void setfreq()
{
 unsigned long  Freq_temp = Freq_1; 
 switch(Freq_array [band] [2])     
 {
 case 1:  
   Freq_temp = Freq_1 + Freq_2;
  break;
 case 2:  
  Freq_temp = Freq_2 - Freq_1;
  break;
 }
  char buf[10];
  lcd.setCursor(0,0);
  lcd.print("Freq =");
  ltoa(Freq_temp,buf,10);

  if (Freq_temp < 1000000) // 1m
  {
    lcd.print("     "); //8
    lcd.print(buf[0]);
    lcd.print(buf[1]);
    lcd.print(buf[2]);
    lcd.print('.');
    lcd.print(buf[3]);
    lcd.print(buf[4]);
    lcd.print(buf[5]);
    lcd.print("   KHz ");         
  }

    
  if (Freq_temp >= 1000000 && Freq_temp < 10000000)
  {
    lcd.print("   "); //8
    lcd.print(buf[0]);
    lcd.print(',');
    lcd.print(buf[1]);
    lcd.print(buf[2]);
    lcd.print(buf[3]);
    lcd.print('.');
    lcd.print(buf[4]);
    lcd.print(buf[5]);
    lcd.print(buf[6]);
    lcd.print("   MHz   ");
  }
  if (Freq_temp >= 10000000 && Freq_temp < 100000000)
  {
    lcd.print("  "); //8
    lcd.print(buf[0]);
    lcd.print(buf[1]);
    lcd.print(',');
    lcd.print(buf[2]);
    lcd.print(buf[3]);
    lcd.print(buf[4]);
    lcd.print('.');
    lcd.print(buf[5]);
    lcd.print(buf[6]);
    lcd.print(buf[7]);
    lcd.print("   MHz  ");
  }

  
  if (Freq_temp >= 100000000)
  {
    lcd.print(" "); //8
    lcd.print(buf[0]);
    lcd.print(buf[1]);
    lcd.print(buf[2]);
    lcd.print(',');
    lcd.print(buf[3]);
    lcd.print(buf[4]);
    lcd.print(buf[5]);
    lcd.print('.');
    lcd.print(buf[6]);
    lcd.print(buf[7]);
    lcd.print(buf[8]);
    lcd.print("   MHz ");
    
  }  
  
  si5351aSetFreq(SYNTH_MS_1, Freq_1);
}
void setResolution()
{
  switch(fStepcount)
  {
  case 0:
    fStep=1;
    resolution = "      1    Hz     ";
    break;
  case 1:
    fStep=10;
    resolution = "     10    Hz    ";
    break;
  case 2:
    fStep=50;
    resolution = "     50    Hz    ";
    break;
  case 3:
    fStep=100;
    resolution = "    100    Hz   ";
    break;
  case 4:
    fStep=1000;
    resolution = "      1   kHz    ";
    break;
  case 5:
    fStep=5000;
    resolution = "      5   kHz    ";
    break;
  case 6:
    fStep=10000;
    resolution = "     10   kHz   ";
    break;
  case 7:
    fStep=12500;
    resolution = "     12.5 kHz ";
    break;
  case 8:
    fStep=100000;
    resolution = "    100   kHz  ";
    break;
  case 9:
    fStep=1000000;
    resolution = "  1       MHz    ";
    break;
  }
  resDisplay();
}
void resDisplay()
{
  lcd.setCursor(11,1);
  lcd.print(resolution);
  time = millis()+10000;
}
void si5351aSetFreq(int synth, unsigned long  freq)
{
  unsigned long long CalcTemp;
  unsigned long  a, b, c, p1, p2, p3;

  c = 0xFFFFF;  

  a = ((XtalFreq * 36) /4) / freq; 
  CalcTemp = round((XtalFreq * 36) /4) % freq;
  CalcTemp *= c;
  CalcTemp /= freq ; 
  b = CalcTemp;  
  p3  = c;
  p2  = (128 * b) % c;
  p1  = 128 * a;
  p1 += (128 * b / c);
  p1 -= 512;
  Si5351_write(synth, 0xFF);  
  Si5351_write(synth + 1, 0xFF);
  Si5351_write(synth + 2, (p1 & 0x00030000) >> 16);
  Si5351_write(synth + 3, (p1 & 0x0000FF00) >> 8);
  Si5351_write(synth + 4, (p1 & 0x000000FF));
  Si5351_write(synth + 5, 0xF0 | ((p2 & 0x000F0000) >> 16));
  Si5351_write(synth + 6, (p2 & 0x0000FF00) >> 8);
  Si5351_write(synth + 7, (p2 & 0x000000FF));

}
void si5351aStart()
{
 
  Si5351_write(XTAL_LOAD_CAP,0b11000000);      
  Si5351_write(CLK_ENABLE_CONTROL,0b00000000); 
  Si5351_write(CLK0_CONTROL,0b00001111);       
  Si5351_write(CLK1_CONTROL,0b00001111);       
  Si5351_write(CLK2_CONTROL,0b00101111);       
  Si5351_write(PLL_RESET,0b10100000);          

  unsigned long  a, b, c, p1, p2, p3;

  a = 36;           
  b = 0;           
  c = 0xFFFFF;     
  p3  = c;
  p2  = (128 * b) % c;
  p1  = 128 * a;
  p1 += (128 * b / c);
  p1 -= 512;

  Si5351_write(SYNTH_PLL_A, 0xFF);
  Si5351_write(SYNTH_PLL_A + 1, 0xFF);
  Si5351_write(SYNTH_PLL_A + 2, (p1 & 0x00030000) >> 16);
  Si5351_write(SYNTH_PLL_A + 3, (p1 & 0x0000FF00) >> 8);
  Si5351_write(SYNTH_PLL_A + 4, (p1 & 0x000000FF));
  Si5351_write(SYNTH_PLL_A + 5, 0xF0 | ((p2 & 0x000F0000) >> 16));
  Si5351_write(SYNTH_PLL_A + 6, (p2 & 0x0000FF00) >> 8);
  Si5351_write(SYNTH_PLL_A + 7, (p2 & 0x000000FF));

  Si5351_write(SYNTH_PLL_B, 0xFF);
  Si5351_write(SYNTH_PLL_B + 1, 0xFF);
  Si5351_write(SYNTH_PLL_B + 2, (p1 & 0x00030000) >> 16);
  Si5351_write(SYNTH_PLL_B + 3, (p1 & 0x0000FF00) >> 8);
  Si5351_write(SYNTH_PLL_B + 4, (p1 & 0x000000FF));
  Si5351_write(SYNTH_PLL_B + 5, 0xF0 | ((p2 & 0x000F0000) >> 16));
  Si5351_write(SYNTH_PLL_B + 6, (p2 & 0x0000FF00) >> 8);
  Si5351_write(SYNTH_PLL_B + 7, (p2 & 0x000000FF));

}

uint8_t Si5351_write(uint8_t addr, uint8_t data)
{
  Wire.beginTransmission(Si5351A_addr);
  Wire.write(addr);
  Wire.write(data);
  Wire.endTransmission();
}
//------------------------------------------------------------------------------------------------------




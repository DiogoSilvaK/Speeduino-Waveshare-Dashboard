/*


 By Diogo Silva


*/



#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Waveshare4InchTftShield.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/gasolina16pt7b.h>
#include <EEPROM.h>



namespace{Waveshare4InchTftShield Waveshield; Adafruit_GFX &tft = Waveshield;}

uint8_t Orientation = 1;

#define BLACK 0x0000
#define NAVY 0x000F
#define DARKGREEN 0x03E0
#define DARKCYAN 0x03EF
#define MAROON 0x7800
#define PURPLE 0x780F
#define OLIVE 0x7BE0
#define LIGHTGREY 0xC618
#define DARKGREY 0x7BEF
#define DARKBLUE 0x001F
#define BLUE 0x045D
#define GREEN 0x07E0
#define CYAN 0x07FF
#define RED 0xF800
#define R21RED 0xB145
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define R21 0xD4A9
#define DARKORANGE 0xFD20
#define ORANGE  0xFA60
#define GREENYELLOW 0xAFE5
#define PINK 0xF81F

uint8_t speeduinoResponse[100];
uint8_t byteNumber[2];
#define BYTES_TO_READ 74
#define SERIAL_TIMEOUT 300
float rps;
boolean sent = false;
boolean received = false;
uint32_t sendTimestamp;

#define debounce 20
#define holdTime 500

int tps;
int iat;
int clt;
int engStatus;
int adv;
int bat;
unsigned int rpm;
float afr;
float afrConv;
float mapData;
float bar;

int PinBot = 21;
int ValBot = 0;
int ValBotUlt = 0;
long BotDwnTime;
long BotUpTime;
bool ignoreUp = false;

int turnSignalLeft = 0;
int turnSignalRight = 0;
int fuelLight = 0;

int rpmMax;
int rpmRedline;

bool togglebat = false;
bool toggleign = false;
bool toggletemp = false;

uint8_t cmdAdata[50];

int CaixaObjetoMainScreen[2][2] = {{70,285},{225,285}};
int Pointer = 0;

int brightness;

double valorMap[] = {-1.00,-0.50, -0.00, 0.50, 1.00};

void setup() 
{
  Serial.begin(115200);
  while(!Serial);

  SPI.begin();
  Waveshield.begin();
  
  tft.setRotation(Orientation);

  pinMode(21,INPUT_PULLUP);
  //pinMode(A5,INPUT);
  //pinMode(A4,INPUT);
  //pinMode(A2,INPUT);

  tft.fillScreen(BLACK);
  
  tft.setTextColor(R21, BLACK);
  tft.setCursor(75,305);
  tft.setTextSize(3);
  tft.setFont(&FreeSans12pt7b);
  tft.print("RENAULT");

  for(int i = 0; i<50; i++)
  {
  tft.fillRoundRect(100 + 120 - (i*1.3),100 + (i*2.3)-90,11,11,6,R21);
  tft.fillRoundRect(100 +56 + (i*1.3),213 + (i*2.3)-90,11,11,6,R21);
  tft.fillRoundRect(100 + 120 + (i*1.3),326 - (i*2.3)-90,11,11,6,R21);
  tft.fillRoundRect(100 +85 + (i*1.3),214 - (i*2.3)-90,11,11,6,R21);
  tft.fillRoundRect(100 +149 + (i*1.3),100 + (i*2.3)-90,11,11,6,R21);
  tft.fillRoundRect(100 +213 - (i*1.3),213 + (i*2.3)-90,11,11,6,R21);
  }
   
  for(int i = 0; i<30; i++)
  {
  tft.fillRoundRect(100 +184 - (i*1.3),213 - (i*2.3)-90,11,11,6,R21);
  tft.fillRoundRect(100 +123 - (i*1.3),281 - (i*2.3)-90,11,11,6,R21);
  }
 
  delay(1500);

  pageChange();

  drawDataInicial();

  tft.setFont();
}

void loop()
{

  RequestData();
  //if(received)
  //{
    drawData();
    received = false;
  //}

}

void drawDataInicial()
{
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(R21,BLACK);
  tft.setTextSize(1);
  
  tft.setCursor(325,25);
  tft.print("IAT-");

  tft.setCursor(165,25);
  tft.print("IGN-");
  
  tft.setCursor(5,25);
  tft.print("BAT-");

  tft.setCursor(35,135);
  tft.print("MAP");

  tft.setCursor(85,218);
  tft.print("CLT");

  tft.setCursor(295,218);
  tft.print("AFR");

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(133,130);
  tft.print("RPM");

  tft.setCursor(390+60 , 15);tft.print("oC");
  tft.setCursor(180,245);tft.print("oC");

  int count = 0;
  tft.setTextColor(WHITE,BLACK);
  for(int i = 0; i <= 400; i++)
  {
    if(i % 100 == 0)
    {
      tft.fillRect(10+i, 82, 3, 9,WHITE);
      tft.setCursor(2 + i-(i/20), 106);
      tft.print(valorMap[count]);
      count++;
    }
  }


  tft.drawRect(0,35, 420, 75, R21);//MAP BOX
  tft.drawRect(0,110, 130,85,R21);//MAP TEXT
  tft.drawRect(130,110,290,85,R21);//RPM TEXT
  tft.drawRect(0,195,210,85,R21);//CLT TEXT
  tft.drawRect(210,195,210,85,R21);//AFR TEXT
  tft.drawRect(9,44, 402, 37, WHITE);//MAP BAR
  tft.drawRect(429 , 245 , 42, -200,WHITE);//TPS
  tft.drawRect(0, 0, 480, 35, R21);//TOP
  tft.drawRect(0,35, 420, 245, R21);//MAIN LFT.
  tft.drawRect(0, 35, 480,245,R21);//MAIN Comp.
  tft.drawRect(0,280,480, 40,R21);//BOTTOM
}

void drawData()
{
  tft.setTextSize(3);



  if(rpm >= rpmRedline)
  {
    for(int i = 0; i <6; i++)
    {
      tft.fillScreen(R21RED);
      tft.setTextColor(BLACK);
      tft.setTextSize(12);
      tft.setCursor(30,115);
      tft.print("SHIFT!");
      delay(15);
    }
    tft.fillScreen(BLACK);
    drawDataInicial();
  }

  tft.setTextColor(R21, BLACK);
  tft.setCursor(422,260);
  tft.setTextSize(2);
  tft.print(tps);tft.print(" ");  
  tft.setCursor(465,260);
  tft.print("%");
  tft.fillRect(430 , 245 - (tps*2), 40 , -(200 - (tps*2)), BLACK);
  tft.fillRect(430 , 245 , 40, -(tps * 2),R21);

/*
  ValBot = digitalRead(PinBot);

  if (ValBot == LOW && ValBotUlt == HIGH && (millis() - BotUpTime) > long(debounce))
  {
    BotDwnTime = millis();
  }
  //Clique
  if (ValBot == HIGH && ValBotUlt == LOW && (millis() - BotDwnTime) > long(debounce))
  {
    if (ignoreUp == false) CliqueAction();
    else ignoreUp = false;
    BotUpTime = millis();
  }

  //Pressionar
  if (ValBot == LOW && (millis() - BotDwnTime) > long(holdTime))
  {
    tft.print("Test2");
    ignoreUp = true;
    BotDwnTime = millis();
  }

  ValBotUlt = ValBot;
*/

  //turnSignalLeft = digitalRead(A5);
  //turnSignalRight = digitalRead(A4);
  //fuelLight = digitalRead(A2);


  if(turnSignalLeft == HIGH)
  {
    tft.setTextColor(GREEN, BLACK);
    tft.setCursor(20,286);
    tft.print((char)27);    
  }
  else
  {
    tft.setTextColor(BLACK, BLACK);
    tft.setCursor(20,286);
    tft.print((char)27);    
  }

  if(turnSignalRight == HIGH)
  {
    tft.setTextColor(GREEN, BLACK);
    tft.setCursor(440,286);
    tft.print((char)26);
  }
  else
  {
    tft.setTextColor(BLACK, BLACK);
    tft.setCursor(440,286);
    tft.print((char)26);    
  }

  if(fuelLight == HIGH)
  {
    tft.setTextSize(1);
    tft.setFont(&gasolina16pt7b);
    tft.setCursor(382, 314);
    tft.setTextColor(RED,BLACK);
    tft.print("F");
    tft.drawRect(374,282,44,36,RED);
  }
  else
  {
    tft.setTextSize(1);
    tft.setFont(&gasolina16pt7b);
    tft.setCursor(382,314);
    tft.setTextColor(BLACK,BLACK);
    tft.print("F"); 
    tft.drawRect(374,282,44,36,BLACK);
  }
  
  tft.setFont();

  
  tft.fillRect(10, 45, mapData * 2, 35, ORANGE);
  tft.fillRect(410,45, -400+(mapData*2),35, BLACK);
  tft.setCursor(17,165);
  tft.setTextSize(3);
  tft.setTextColor(ORANGE,BLACK);
  tft.print((double)((mapData/100)-1));

  tft.setTextSize(8);
  tft.setCursor(178,127);
  tft.print(rpm);tft.print(" ");

  tft.setTextSize(4);
  tft.setCursor(78,240);
  tft.print(clt);tft.print(" ");

  tft.setCursor(255,240);
  tft.print(afr);tft.print(" ");
   
  tft.setTextColor(R21,BLACK);
  if( bat < 10 && togglebat == true){togglebat = false; tft.fillRect(65,7,65,25,BLACK);}
  if( bat >= 10 && togglebat == false){togglebat = true;}
  tft.setTextSize(3);
  tft.setCursor(65,7);
  tft.print(bat/10);

  if( adv < 10 && toggleign == true){toggleign = false; tft.fillRect(225,7,65,25,BLACK);}
  if( adv >= 10 && toggleign == false){toggleign = true;}
  tft.setCursor(225,7);
  tft.print(adv);
  
  if( iat < 10 && toggletemp == true){toggletemp = false; tft.fillRect(385,7,65,25,BLACK);}
  if( iat > 10 && toggletemp == false){toggletemp = true;}
  tft.setCursor(385,7);
  tft.print(iat);tft.print(" ");



}

void CliqueAction()
{
 if(Pointer <= 1)
 {
   Pointer++;
   DesenharCaixaObjeto();
 }  
 else if(Pointer > 1)
 {
   Pointer = 0;
   DesenharCaixaObjeto();
 }
}

void DesenharCaixaObjeto()
{
 tft.drawRect(CaixaObjetoMainScreen[Pointer][0],CaixaObjetoMainScreen[Pointer][1], 150, 30,WHITE);
 tft.setCursor(70,285);
 tft.print(CaixaObjetoMainScreen[Pointer][0]);
 tft.print(CaixaObjetoMainScreen[Pointer][1]);
}

void pageChange()
{
  

  EEPROM.get(1,brightness);
  //EEPROM.get(4, corSel);
  EEPROM.get(6, rpmMax);
  EEPROM.get(8, rpmRedline);

  tft.fillScreen(BLACK);
  rpmRedline = 6500;
  rpmMax = rpmMax * 100;
  brightness = brightness * 10;
  
  Waveshield.setScreenBrightness(10+brightness*(brightness/10));
  
  
  drawDataInicial();
}

void RequestData()
{

  if(sent && Serial.available())
  {
    if(Serial.read() == 'A')
    {
      uint8_t bytesRead = Serial.readBytes(speeduinoResponse, BYTES_TO_READ);
      if(bytesRead != BYTES_TO_READ)
      {
        processData();
        for(uint8_t i = 0; i < bytesRead; i++)
        {
          received = true;
          clearRX();
        }
      }
      else
      {
        processData();
        received = true;
        rps = 1000.0/(millis() - sendTimestamp);
      }
      sent = true;
    }
    else Serial.read();
  }
  else if(!sent)
  {
    Serial.write('A');
    sent = true;
    sendTimestamp = millis();
  }
  else if(sent && millis() - sendTimestamp > SERIAL_TIMEOUT)
  {
    sent = false;
  }
}

void processData()
{
  adv = speeduinoResponse[23];
  afr = speeduinoResponse[10];
  afrConv = afr/10;
  bat = speeduinoResponse[9];
  clt = (speeduinoResponse[7]-40);
  engStatus= speeduinoResponse[31];
  iat = (speeduinoResponse[6]-40);
  mapData = ((speeduinoResponse [5] << 8) | (speeduinoResponse [4]));
  rpm = ((speeduinoResponse [15] << 8) | (speeduinoResponse [14]));
  tps = speeduinoResponse[24];
}

void clearRX()
{
  while(Serial.available())
  Serial.read();
}

#include <Wire.h>
#include "RTClib.h"
#include "SIM900.h"
#include <GSM.h>
#include <SoftwareSerial.h>
#include "sms.h"
#include <dht.h>
#define DHT11_PIN 7
dht DHT;
SMSGSM sms;
RTC_DS1307 RTC;

boolean started=true;
char smsbuffer[160];
char n[20];
String nim="3133507555";
String plusig="+";
char posicion;
char posicion1;
int ledAma=8;
int ledVer=13;
char hum1[160];
char temp1[160];
String g="Grados: ";
String h="Porcentaje de humedad: ";
String fecha="Fecha: ";

byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;
float calibrationFactor = 4.5;
volatile byte pulseCount;  
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long oldTime;
boolean pasandoAgua = false;

int relay1 = 3;
int relay2 = 4;
int motobomba = 5;

/* TFT */
#include <UTFT.h>
#include <UTouch.h>
UTFT    myGLCD(SSD1289,38,39,40,41);
UTouch  myTouch( 42, 44, 46, 48, 50);
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
int x, y, segpin, modulos, day, month, year, hour, minute, maxlen;
long update_time;
char movil[20];
float humidity, temperature;
char stCurrent[20]="";
int stCurrentLen=0;
char stLast[20]="";
extern unsigned int Sena[0x28A0]; //100x104

void msjerror(char *error){
  myGLCD.setColor(255, 0, 0);
  myGLCD.print(error, CENTER, 192);
  delay(500);
  myGLCD.print("                    ", CENTER, 192);
  delay(500);
  myGLCD.print(error, CENTER, 192);
  delay(500);
  myGLCD.print("                    ", CENTER, 192);
  myGLCD.setColor(0, 255, 0);
}

void updateStr(int val){
  if (stCurrentLen<maxlen){
    stCurrent[stCurrentLen]=val;
    stCurrent[stCurrentLen+1]='\0';
    stCurrentLen++;
    myGLCD.setColor(0, 255, 0);
    myGLCD.print(stCurrent, LEFT, 224);
  }
  else{
    msjerror("LIMITE AGOTADO!");
  }
}

void waitForIt(int x1, int y1, int x2, int y2){
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
  while (myTouch.dataAvailable())
    myTouch.read();
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
}

void bootloader(){
  myGLCD.clrScr();
  myGLCD.setColor(35, 130, 118);
  myGLCD.fillRect(0, 0, 319, 55);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(35, 130, 118);
  myGLCD.print("BIENVENIDO", CENTER, 4);
  myGLCD.print("A", CENTER, 21);
  myGLCD.print("SARA", CENTER, 37);
  myGLCD.setFont(SmallFont);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("SISTEMA AUTOMATIZADO", CENTER, 60);
  myGLCD.print("DE RIEGO POR", CENTER, 73);
  myGLCD.print("ARDUINO", CENTER, 89);
  myGLCD.print("VER: 1.0", CENTER, 105);
  myGLCD.drawBitmap (108, 116, 100, 104, Sena, 1);
  myGLCD.setBackColor(35, 130, 118);
  myGLCD.setColor(35, 130, 118);
  myGLCD.fillRect(0, 224, 319, 239);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("(C) SENA 2016", CENTER, 226);
  delay(4000);
  myGLCD.clrScr();
  myGLCD.setFont(BigFont); 
  pregchangmovil();
}

void pulseCounter()
{
  // Incrementa el contador de pulsos
  pulseCount++;
}

void update(){
  /* GSM */
  DHT.read11(DHT11_PIN);
  DateTime now = RTC.now();
  posicion = sms.IsSMSPresent(SMS_UNREAD);

  float temp=DHT.temperature;
  float hum=DHT.humidity;
  
  humidity=DHT.humidity;
  temperature=DHT.temperature;
  
  Serial.println(temp);
  Serial.println(hum);

//  fecha+=now.day();
//  fecha+='/';
//  fecha+=now.month();
//  fecha+='/';
//  fecha+=now.year();
//  fecha+='\n';
//  fecha+="Hora: ";
//  fecha+=now.hour();
//  fecha+=':';
//  fecha+=now.minute();
//  fecha+=':';
//  fecha+=now.second();

  day=now.day();
  month=now.month();
  year=now.year();
  hour=now.hour();
  minute=now.minute();

  String nimconcat =  String(plusig + nim);
  char nimconcat2[11];
  nimconcat.toCharArray(nimconcat2, 11);
  char motobombaErr[]="ERROR: Compruebe motobomba.";

  myGLCD.setFont(BigFont);
  myGLCD.setColor(255,255,255);
  myGLCD.setBackColor(0,0,0);
  myGLCD.print("  ", 12, 52);
  myGLCD.printNumI(day, 12, 52);
  myGLCD.printNumI(month, 63, 52);
  myGLCD.printNumI(year, 114, 52);
  myGLCD.printNumI(hour, 200, 52);
  myGLCD.printNumI(minute, 245, 52);
  myGLCD.printNumF(temperature,1, 200, 97);
  myGLCD.printNumF(humidity,1, 200, 142);
  // myGLCD.print("Regando", 10, 142);
  // delay(1000);
  // myGLCD.print("       ", 10, 142);

  if (hum<40.0){
    if (modulos==1){
      digitalWrite(relay1, LOW);
      digitalWrite(relay2, HIGH);
    }
    if (modulos==2){
      digitalWrite(relay2, LOW);
      digitalWrite(relay1, HIGH);
    }
    if(modulos==3){
      digitalWrite(relay1, LOW);
      digitalWrite(relay2, LOW);
    }
    digitalWrite(motobomba, LOW);
    Serial.println("ARRANCO - Flujo de Agua");
    delay(5000);
    detachInterrupt(sensorInterrupt);
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    oldTime = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;
    unsigned int frac;
    Serial.print("Flow rate: ");
    if (flowMilliLitres>1){
      Serial.print("ESTA PASANDO AGUA");
      pasandoAgua=true;
    }
    else {
      pasandoAgua=false;
    }
    frac = (flowRate - int(flowRate)) * 10;
    pulseCount = 0;
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

    if(pasandoAgua){
      digitalWrite(motobomba, LOW);  
      myGLCD.print("Regando", 10, 142);
    }
    else {
      Serial.println("ERROR: Compruebe motobomba.");
      digitalWrite(motobomba, HIGH);
//      sms.SendSMS(nimconcat2,motobombaErr);
//      Serial.println("\nSMS Enviado OK");
      myGLCD.print("       ", 10, 142);
    }
  }
  if (hum>50.0) {
    if (modulos==1){
      digitalWrite(relay1, HIGH);
      digitalWrite(relay2, HIGH);
    }
    if (modulos==2){
      digitalWrite(relay2, HIGH);
      digitalWrite(relay1, HIGH);
    }
    if(modulos==3){
      digitalWrite(relay1, HIGH);
      digitalWrite(relay2, HIGH);
    }
    digitalWrite(motobomba, HIGH);
  }

  if (hour==22 && minute==00){
    
     if (modulos==1){
      digitalWrite(relay1, LOW);
      digitalWrite(relay2, HIGH);
      }
    if (modulos==2){
      digitalWrite(relay2, LOW);
      digitalWrite(relay1, HIGH);
    }
    if(modulos==3){
      digitalWrite(relay1, LOW);
      digitalWrite(relay2, LOW);
    }
    delay (50000);
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    }
  
  if(posicion){    
    sms.GetSMS(posicion, n, smsbuffer, 100);
    String numeroreal(n);
    if(numeroreal==nim){
      if(smsbuffer[0]=='1'){
        h+=hum;
        Serial.println(smsbuffer[0]);
        digitalWrite(ledVer,HIGH);
        digitalWrite(ledAma,LOW);
        int longitud;
        longitud=h.length();
        for (int i=0;i<longitud;i++){
          hum1[i]=h.charAt(i);
        }
        sms.SendSMS(nimconcat2,hum1);
        Serial.println("\nSMS Enviado OK");
        delay(5000);
        h="Porcentaje de humedad: ";
      }
      else if (smsbuffer[0]=='2'){
        g+=temp;
        Serial.println(smsbuffer[0]);
        digitalWrite(ledVer,HIGH);
        digitalWrite(ledAma,LOW);
        int longitud;
        longitud=g.length();
        for (int i=0;i<longitud;i++){
          temp1[i]=g.charAt(i);
        }
        sms.SendSMS(nimconcat2,temp1);
        Serial.println("\nSMS sent OK");
        delay(5000);
        g="Grados: ";
      }
    }
  } 
  while (posicion1 = sms.IsSMSPresent(SMS_READ)){
    Serial.println(posicion1);
    sms.DeleteSMS(posicion1);
    Serial.println("Mensaje borrado");
  }
  
  flowMilliLitres=0;
  oldTime=0;
}

void pregchangmovil(){
  int pulso_correcto=0;
  myGLCD.clrScr();
  myGLCD.setFont(BigFont);
  myGLCD.setColor(35, 130, 118);
  myGLCD.fillRect(0, 0, 319, 20);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(35, 130, 118);
  myGLCD.print("SARA", CENTER, 4);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("CONF. NUM. MOVIL", CENTER, 49);
  myGLCD.print("Desea modificar el", CENTER, 67);
  myGLCD.print("num. celular que", CENTER, 85);
  myGLCD.print("trabajara con SARA?", CENTER, 103);
  myGLCD.print("Debe ingresar PIN", CENTER, 121); 
  myGLCD.print("de seguridad!.", CENTER, 139);  
  myGLCD.setColor(35, 130, 118);
  myGLCD.setBackColor(35, 130, 118);
  myGLCD.fillRoundRect (80, 164, 130, 204);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (80, 164, 130, 204);
  myGLCD.print("SI", 90, 176);
  myGLCD.setColor(35, 130, 118);
  myGLCD.fillRoundRect (200, 164, 250, 204);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (200, 164, 250, 204);
  myGLCD.print("NO", 211, 176);
  
  while(pulso_correcto==0){
    if (myTouch.dataAvailable()){
      myTouch.read();
      x=myTouch.getX();
      y=myTouch.getY();
      if((y>=110) && (y<=150)){
        if ((x>=80) && (x<=130))
        {
          waitForIt(80, 164, 130, 204);
          pulso_correcto=1;
          teclado(6, 1);
          
        }
        if ((x>=200) && (x<=250))
        {
          waitForIt(200, 164, 250, 204);
          pulso_correcto=1;
          pantalla1();
          
        }
      }
    }
  }
}

void changemovil(){
  myGLCD.clrScr();
  myGLCD.setFont(BigFont);
  myGLCD.setColor(35, 130, 118);
  myGLCD.fillRect(0, 0, 319, 20);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(35, 130, 118);
  myGLCD.print("SARA", CENTER, 4);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("CONF. NUM. MOVIL", CENTER, 49);
  myGLCD.print("A continuacion", CENTER, 67);
  myGLCD.print("introduzca", CENTER, 85);
  myGLCD.print("el numero", CENTER, 103);
  myGLCD.print("celular a trabajar", CENTER, 121);  
  myGLCD.print("con SARA.", CENTER, 139); 
  delay(5000);
  teclado(10, 2);
}
void teclado(int maxlong, int opc){
  int pulso_correcto=0;
  maxlen=maxlong;
  myGLCD.clrScr();
  myGLCD.setBackColor(35, 130, 118);
  for (x=0; x<5; x++)
  {
    myGLCD.setColor(35, 130, 118);
    myGLCD.fillRoundRect (10+(x*60), 10, 60+(x*60), 60);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (10+(x*60), 10, 60+(x*60), 60);
    myGLCD.printNumI(x+1, 27+(x*60), 27);
  }

  for (x=0; x<5; x++)
  {
    myGLCD.setColor(35, 130, 118);
    myGLCD.fillRoundRect (10+(x*60), 70, 60+(x*60), 120);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (10+(x*60), 70, 60+(x*60), 120);
    if (x<4)
    myGLCD.printNumI(x+6, 27+(x*60), 87);
  }
  myGLCD.print("0", 267, 87);
  myGLCD.setColor(35, 130, 118);
  myGLCD.fillRoundRect (10, 130, 150, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (10, 130, 150, 180);
  myGLCD.print("Borrar", 40, 147);
  myGLCD.setColor(35, 130, 118);
  myGLCD.fillRoundRect (160, 130, 300, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (160, 130, 300, 180);
  myGLCD.print("Enter", 190, 147);
  myGLCD.setBackColor (0, 0, 0);
  
  while(pulso_correcto==0){
    if (myTouch.dataAvailable()){
      myTouch.read();
      x=myTouch.getX();
      y=myTouch.getY();
      if ((y>=10) && (y<=60)){
        if ((x>=10) && (x<=60))
        {
          waitForIt(10, 10, 60, 60);
          updateStr('1');
        }
        if ((x>=70) && (x<=120))
        {
          waitForIt(70, 10, 120, 60);
          updateStr('2');
        }
        if ((x>=130) && (x<=180))
        {
          waitForIt(130, 10, 180, 60);
          updateStr('3');
        }
        if ((x>=190) && (x<=240))
        {
          waitForIt(190, 10, 240, 60);
          updateStr('4');
        }
        if ((x>=250) && (x<=300))
        {
          waitForIt(250, 10, 300, 60);
          updateStr('5');
        }
      }

      if ((y>=70) && (y<=120)){
        if ((x>=10) && (x<=60))
        {
          waitForIt(10, 70, 60, 120);
          updateStr('6');
        }
        if ((x>=70) && (x<=120))
        {
          waitForIt(70, 70, 120, 120);
          updateStr('7');
        }
        if ((x>=130) && (x<=180))
        {
          waitForIt(130, 70, 180, 120);
          updateStr('8');
        }
        if ((x>=190) && (x<=240))
        {
          waitForIt(190, 70, 240, 120);
          updateStr('9');
        }
        if ((x>=250) && (x<=300))
        {
          waitForIt(250, 70, 300, 120);
          updateStr('0');
        }
      }

      if ((y>=130) && (y<=180)){
        // Clear All
        // if ((x>=10) && (x<=150))
        // {
          // waitForIt(10, 130, 150, 180);
          // stCurrent[0]='\0';
          // stCurrentLen=0;
          // myGLCD.setColor(0, 0, 0);
          // myGLCD.fillRect(0, 224, 319, 239);
        // }
        // Clear one
        if ((x>=10) && (x<=150))
        {
          waitForIt(10, 130, 150, 180);
            for (x=0; x<stCurrentLen; x++)
            {
              stCurrent[x]=stCurrent[x];
            }
          if(stCurrentLen>0){
            stCurrentLen=stCurrentLen-1;
          }
          myGLCD.setColor(0, 0, 0);
          myGLCD.fillRect(stCurrentLen*16, 224, 16+(stCurrentLen*16)+16, 239);      
        }
        if ((x>=160) && (x<=300))
        {
          waitForIt(160, 130, 300, 180);
          if (stCurrentLen==maxlong)
          {
            for (x=0; x<stCurrentLen+1; x++)
            {
              stLast[x]=stCurrent[x];
            }
            stCurrent[0]='\0';
            stCurrentLen=0;
            myGLCD.setColor(0, 0, 0);
            myGLCD.fillRect(0, 208, 319, 239);
            myGLCD.setColor(0, 255, 0);
            myGLCD.print(stLast, LEFT, 208);
            int answer= atoi(stLast);
            if(opc==1){
              if(answer==segpin){
                changemovil();
              }
              else
              {
                msjerror(" PIN  INVALIDO ");
              }
            }
            if(opc==2){
              nim =  String(stLast);
              pantalla1();
            }
          }
          else
          {
            msjerror(" DATO INVALIDO ");
          }
        }
      }
    }
  } 
}

void pantalla1(){
  int pulso_correcto=0;
  myGLCD.clrScr();
  myGLCD.setFont(BigFont);
  myGLCD.setColor(35, 130, 118);
  myGLCD.fillRect(0, 0, 319, 20);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(35, 130, 118);
  myGLCD.print("SARA", CENTER, 4);
  myGLCD.setBackColor(0, 0, 0);
  myGLCD.print("CONF. DE MODULO", CENTER, 49);
  myGLCD.print("Seleccione modulo(s)", CENTER, 67);
  myGLCD.print("a utilizar.", CENTER, 85);
  myGLCD.setBackColor(35, 130, 118);
  
  for (x=0; x<2; x++){
    myGLCD.setColor(35, 130, 118);
    myGLCD.fillRoundRect (80+(x*120), 110, 130+(x*120), 150);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (80+(x*120), 110, 130+(x*120), 150);
    myGLCD.printNumI(x+1, 97+(x*120), 122);
  }
  
  myGLCD.setColor(35, 130, 118);
  myGLCD.fillRoundRect (80, 170, 250, 200);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (80, 170, 250, 200);
  myGLCD.print("TODOS", 126, 177);
  
  while(pulso_correcto==0){
    if (myTouch.dataAvailable()){
      myTouch.read();
      x=myTouch.getX();
      y=myTouch.getY();
      if((y>=110) && (y<=150)){
        if ((x>=80) && (x<=130))
        {
          waitForIt(80, 110, 130, 150);
          modulos=1;
          pulso_correcto=1;
        }
        if ((x>=200) && (x<=250))
        {
          waitForIt(200, 110, 250, 150);
          modulos=2;
          pulso_correcto=1;
        }
      }
      if((y>=170) && (y<=200)){
        if ((x>=80) && (x<=250))
        {
          waitForIt(80, 170, 250, 200);
          modulos=3;
          pulso_correcto=1;
        }
      }
    }
  }
  pantalla2();  
}

void pantalla2(){
  int pulso_correcto=0;
  myGLCD.clrScr();
  myGLCD.setFont(BigFont);
  myGLCD.setColor(35, 130, 118);
  myGLCD.fillRect(0, 0, 319, 20);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(35, 130, 118);
  myGLCD.print("SARA", CENTER, 4);
  myGLCD.setBackColor(0, 0, 0);
  
  myGLCD.setColor(255,255,255);
  myGLCD.print("Fecha:", 10, 32);
  //myGLCD.printNumI(day, 12, 52);
  myGLCD.print("/", 46, 52);
  //myGLCD.printNumI(month, 63, 52);
  myGLCD.print("/", 97, 52);
  //myGLCD.printNumI(year, 114, 52);
  myGLCD.print("Hora:", 200, 32);
  //myGLCD.printNumI(hour, 200, 52);
  myGLCD.print(":", 230, 52);
  //myGLCD.printNumI(minute, 245, 52);
  myGLCD.print("Tempt:", 200, 77);
  //myGLCD.printNumF(temperature,1, 200, 97);
  myGLCD.print("C", 270, 97);
  myGLCD.print("Humed:", 200, 122);
  //myGLCD.printNumF(humidity,1, 200, 142);
  myGLCD.print("%", 270, 142);
  myGLCD.print("Modulo(s):", 10, 77);
  if(modulos==1){
    myGLCD.printNumI(modulos, 10, 97);
  }
  if(modulos==2){
    myGLCD.printNumI(modulos, 10, 97);
  }
  if(modulos==3){
    myGLCD.print("1 y 2", 10, 97);
  }
  myGLCD.print("Estado:", 10, 122);
  
  myGLCD.setFont(SmallFont);
  myGLCD.setBackColor(35, 130, 118);
  myGLCD.setColor(35, 130, 118);
  myGLCD.fillRoundRect (200, 170, 296, 210);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (200, 170, 296, 210);
  myGLCD.print("CAMBIAR", 220, 175);
  myGLCD.print("MODULO", 224, 190);
  
  while(pulso_correcto==0){
    update_time=millis()/1000;
    if (update_time%2!=0)
    {
      update();
    }
    if (myTouch.dataAvailable()){
      myTouch.read();
      x=myTouch.getX();
      y=myTouch.getY();
      if((y>=160) && (y<=210)){
        if ((x>=200) && (x<=296))
        {
          waitForIt(200, 170, 296, 210);
          pulso_correcto=1;
          pantalla1();
        }
      }
    }
  }
}

void setup()
{
  digitalWrite(relay1,HIGH);
  digitalWrite(relay2,HIGH);
  digitalWrite(motobomba,HIGH);
  pinMode(relay1,OUTPUT);
  pinMode(relay2,OUTPUT);
  pinMode(motobomba,OUTPUT);
  Serial.begin(9600);
  myGLCD.InitLCD(LANDSCAPE);
  myGLCD.clrScr();
  myTouch.InitTouch(LANDSCAPE);
  myTouch.setPrecision(PREC_LOW);
  myGLCD.setFont(BigFont);
  segpin=813276;
  
  // flow rate
  
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);
  
  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;
  
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  
  /* GSM */
  Wire.begin(); // Inicia el puerto I2C
  RTC.begin(); // Inicia la comunicación con el RTC
  RTC.adjust(DateTime(__DATE__, __TIME__));
  
  pinMode(ledAma,OUTPUT);
  pinMode(ledVer,OUTPUT);
  //Serial connection.
  Serial.begin(9600);
  Serial.println("GSM Shield testing.");
  
  if (gsm.begin(2400)){
    Serial.println("\nstatus=READY");
    started=true;  
  }
  else Serial.println("\nstatus=IDLE");
  
} 

void loop()
{
  DateTime now = RTC.now();
  bootloader();
}

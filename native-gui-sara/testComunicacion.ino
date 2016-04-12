int led = 6;       // LED 
int comunicador = 8;   // Pin comunicador recibe un HIGH/LOW

boolean hayVolt = false; // Boolean si hay lectura en comunicador es true.

void setup()
{
  Serial.begin(9600);
  pinMode(led, OUTPUT);
  pinMode(comunicador, INPUT);
  digitalWrite(led, LOW); 
}

void loop()
{
  if (digitalRead(comunicador) == HIGH)
  {
    Serial.println("input 8 is high");
    hayVolt = true;
  }
  else {
      hayVolt=false; 
      Serial.println("input 8 is low");
    }

  if (hayVolt){
      digitalWrite(led, HIGH); 
    }
    else{
        digitalWrite(led, LOW); 
      }
}

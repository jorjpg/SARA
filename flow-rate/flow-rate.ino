byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;

/*  
El sensor de flujo de efecto Hall emite aproximadamente 4,5 pulsos por segundo para flujo
de litros/minuto.
*/
 
float calibrationFactor = 4.5;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;

void setup()
{
  
  Serial.begin(38400);
   
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

/*
El sensor de efecto Hall está conectada al pin 2, 
que utiliza interrupción 0. configurado para disparar en estado FALLING
(transición de estado HIGH al estado LOW)
*/
   
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}

void loop()
{
   
   if((millis() - oldTime) > 1000)    // Sólo ejecuta contadores de proceso una vez por segundo
  { 
    // Desactivar Interrupt mientras se hace cálculo del caudal y el envío del valor al host
    detachInterrupt(sensorInterrupt);
        
/*
Debido a que este bucle puede no terminar exactamente en intervalos de 1 segundo 
se calcula el número de milisegundos que han transcurrido desde la última ejecución y 
el uso de escala de salida. También aplicamos el factor de calibración para escalar la 
salida basándose en el número de pulsos por segundo por unidad de medida
(litros/minuto en este caso) procedentes del sensor.
*/	
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
/*
Note el tiempo que paso en la ejecucion del proceso. Tenga en cuenta que la función millis() 
realmente no va a incrementar en este punto, pero todavía devolverá el valor 
que fue fijado justo antes de las interrupciones.
*/
    oldTime = millis();
/*
Divide la tasa de flujo en litros/minuto por 60 para determinar
la cantidad de litros que han pasado pOr el sensor en el interval
de 1 seg, luego los multiplica por 1000 para convert a mililitros.
*/
    flowMilliLitres = (flowRate / 60) * 1000;
	// Añadir los mililitros que pasaron en este segundo para el total acumulado
    totalMilliLitres += flowMilliLitres;
	unsigned int frac;
	// Imprime la tasa de flujo para este segundo en litros/minutos
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Imprime la parte entera de la variable
    Serial.print(".");             // Imprime el punto decimal
    // Determinar la parte fraccionaria y multiplicado por 10 nos da 1 decimal.
    frac = (flowRate - int(flowRate)) * 10;
    Serial.print(frac, DEC) ;      // Imprimir la parte fraccionaria de la variable
    Serial.print("L/min");
    // Imprimir el número de litros que fluyó en este segundo
    Serial.print("  Current Liquid Flowing: ");             // separador de salida
    Serial.print(flowMilliLitres);
    Serial.print("mL/Sec");
	// Imprimir el total acumulado de litros que fluyó desde el inicio
    Serial.print("  Output Liquid Quantity: ");             // separador de salida
    Serial.print(totalMilliLitres);
    Serial.println("mL"); 
	// Restablecer el contador de pulsos para que podamos empezar a incrementar de nuevo
    pulseCount = 0;
	// Activa interrupt de nuevo, hemos terminado de enviar la salida.
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
}

/*
Rutina de servicio del Insterrupt
 */
void pulseCounter()
{
  // Incrementa el contador de pulsos
  pulseCount++;
}

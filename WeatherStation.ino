#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <dht.h>


#define DHT22_PIN 2
#define ALT 500

//Sensore pressione
// Connect VCC of the BMP085 sensor to 3.3V (NOT 5.0V!)
// Connect GND to Ground
// Connect SCL to i2c clock - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 5
// Connect SDA to i2c data - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 4
// EOC is not used, it signifies an end of conversion
// XCLR is a reset pin, also not used here


dht DHT;
Adafruit_BMP085 bmp;


void setup()
{
  Serial.begin(9600);

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {
    }
  }
}

void loop()
{
  double qnh;
  double umidita;
  double temp;

  // READ DATA
  int chk = DHT.read22(DHT22_PIN);
  switch (chk)
  {
  case DHTLIB_OK:  

    qnh = bmp.readSealevelPressure(ALT)/100;
    umidita = DHT.humidity;
    temp = DHT.temperature;


    // DISPLAY DATA
    Serial.print("Umidita' (%): ");
    Serial.println(umidita, 1);
    Serial.print("Temperatura (C): ");
    Serial.println(temp, 1);
    Serial.print("Rugiada (C): ");
    Serial.println(puntoDiRugiada(temp, umidita), 1);
    Serial.print("QNH: ");
    Serial.println(qnh, 1);
    Serial.println();

    break;
  case DHTLIB_ERROR_CHECKSUM: 
    Serial.print("Checksum error,\t"); 
    break;
  case DHTLIB_ERROR_TIMEOUT: 
    Serial.print("Time out error,\t"); 
    break;
  default: 
    Serial.print("Unknown error,\t"); 
    break;
  }


  delay(5000);
}

//calcolo il punto di rugiada
double puntoDiRugiada(double temperatura, double umidita)
{
  double a = 17.271;
  double b = 237.7;
  double temp = (a * temperatura) / (b + temperatura) + log(umidita/100);
  double Td = (b * temp) / (a - temp);
  return Td;
}

#include <SPI.h>
#include <Ethernet.h>


#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <dht.h>


#define UPDATE_INTERVAL 900000L //intervallo di aggiornamento
#define DEBUG false
#define LOG false

#define DHT22_PIN A3

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

byte mac[] = { 
  0xDA, 0xAA, 0xBE, 0xEF, 0xAE, 0xDE  };

char server[] = "is0eir.altervista.org"; //sito web

EthernetClient client;

String strURL = "";
String errors = "";

void setup()
{

  delay(30000); //attendo la connessione wifi

  if(LOG){
    Serial.begin(9600);
  }

  if (!bmp.begin()) {
    errors += "BMP085error";
    while (1) {
    }
  }

  delay(1000); //attendo un secondo

  if(!Ethernet.begin(mac)){
    if(LOG){
      Serial.println("DHPC error");
    }
    while(1);
  }

  //invio al pc il mio IP
  if(LOG){
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
  }

  if(DEBUG){
    sendData(); 
    client.stop();
  }
}

void loop()
{

  if (!DEBUG) {
    sendData();
      
    //read the server response
    while (client.available() && LOG) {
      char c = client.read();
      Serial.print(c);
    }
    
    client.stop();
    delay(UPDATE_INTERVAL);
  }
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

void sendData(){
  double qnh = 0;
  double umidita = 0;
  double temp = 0;

  // READ DATA
  int chk = DHT.read22(DHT22_PIN);
  switch (chk)
  {
  case DHTLIB_OK:  

    qnh = bmp.readSealevelPressure(ALT)/100;
    umidita = DHT.humidity;
    temp = DHT.temperature;


    // DISPLAY DATA
    if(LOG){
      Serial.print("Umidita' (%): ");
      Serial.println(umidita, 1);
      Serial.print("Temperatura (C): ");
      Serial.println(temp, 1);
      Serial.print("Rugiada (C): ");
      Serial.println(puntoDiRugiada(temp, umidita), 1);
      Serial.print("QNH: ");
      Serial.println(qnh, 1);

      Serial.println(server);
      Serial.println();
    }

    break;
  case DHTLIB_ERROR_CHECKSUM: 
    errors += "DHTChecksum"; 
    if(LOG){
      Serial.println("DHT Checksum error"); 
    }
    break;
  case DHTLIB_ERROR_TIMEOUT: 
    errors += "DHTTimeout"; 
    if(LOG){
      Serial.println("DHT Time out error "); 
    }
    break;
  default: 
    errors += "DHTUnknown"; 
    if(LOG){
      Serial.println("DHTUnknown"); 
    }
    break;
  } 

  if (client.connect(server, 80))
  {
    if(LOG){
      Serial.println("Connesso");
    }

    char charBuf[7];

    if(errors.length() != 0){
      strURL = "GET /meteo/receive.php?err=" + errors + " HTTP/1.1";
    } 
    else {

      //creo l'url utilizzando una stringa
      strURL ="GET /meteo/receive.php?temp=";

      dtostrf(temp, 4, 1, charBuf);
      strURL+= charBuf; 

      strURL+= "&hum=";
      dtostrf(umidita, 4, 1, charBuf);
      strURL+= charBuf;

      strURL+= "&dewp=";
      dtostrf(puntoDiRugiada(temp, umidita), 4, 1, charBuf);
      strURL+= charBuf;

      strURL+="&qnh=" ;
      dtostrf(qnh, 4, 1, charBuf);
      strURL+= charBuf; 

      strURL+=" HTTP/1.1";
    }

    //invio la richiesta al server
    client.println(strURL);
    client.println("Host: is0eir.altervista.org");
    client.println("Connection: close");
    client.println();



    delay(1000);
    if(LOG){
      Serial.println(strURL);
    }
  }
  else
  {   
    if(LOG){
      Serial.println("Errore Connessione");
      Serial.println("Disconnessione");
    }
  }
}
















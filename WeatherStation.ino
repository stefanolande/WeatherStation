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

  delay(60000); //waiting for the router

  if(LOG){
    Serial.begin(9600);
  }

  if (!bmp.begin()) {
    errors += "BMP085error";
    while (1) {
    }
  }

  delay(1000);

  //try to connect ethernet
  while(!Ethernet.begin(mac)){
#if LOG
      Serial.println("DHPC error");
#endif
    delay(10000); //wait in case of conncetion failure
  }

#if LOG
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
#endif

#if DEBUG
    sendData(); 
    client.stop();
#endif
}

void loop()
{

#if !DEBUG
    sendData();
      
    //read the server response
    while (client.available() && LOG) {
      char c = client.read();
      Serial.print(c);
    }
    
    client.stop();
    delay(UPDATE_INTERVAL);
#endif
}



double dewPoint(double temperatura, double umidita)
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
#if LOG
      Serial.print("Umidita' (%): ");
      Serial.println(umidita, 1);
      Serial.print("Temperatura (C): ");
      Serial.println(temp, 1);
      Serial.print("Rugiada (C): ");
      Serial.println(dewPoint(temp, umidita), 1);
      Serial.print("QNH: ");
      Serial.println(qnh, 1);

      Serial.println(server);
      Serial.println();
#endif

    break;
  case DHTLIB_ERROR_CHECKSUM: 
    errors += "DHTChecksum"; 
#if LOG
      Serial.println("DHT Checksum error"); 
#endif
    break;
  case DHTLIB_ERROR_TIMEOUT: 
    errors += "DHTTimeout"; 
#if LOG
      Serial.println("DHT Time out error "); 
#endif
    break;
  default: 
    errors += "DHTUnknown"; 
#if LOG
      Serial.println("DHTUnknown"); 
#endif
    break;
  } 

  if (client.connect(server, 80))
  {
#if LOG
      Serial.println("Connesso");
#endif

    char charBuf[7];
    String tempStr;

    if(errors.length() != 0){
      strURL = "GET /meteo/receive.php?err=" + errors + " HTTP/1.1";
    } 
    else {

      //creo l'url utilizzando una stringa
      strURL ="GET /meteo/receive.php?temp=";

      dtostrf(temp, 4, 1, charBuf);
      tempStr = String(charBuf);
      tempStr.trim();
      strURL+= tempStr; 

      strURL+= "&hum=";
      dtostrf(umidita, 4, 1, charBuf);
      tempStr = String(charBuf);
      tempStr.trim();
      strURL+= tempStr; 

      strURL+= "&dewp=";
      dtostrf(dewPoint(temp, umidita), 4, 1, charBuf);
      tempStr = String(charBuf);
      tempStr.trim();
      strURL+= tempStr; 

      strURL+="&qnh=" ;
      dtostrf(qnh, 4, 1, charBuf);
      tempStr = String(charBuf);
      tempStr.trim();
      strURL+= tempStr;  

      strURL+=" HTTP/1.1";
    }

    //invio la richiesta al server
    client.println(strURL);
    client.println("Host: is0eir.altervista.org");
    client.println("Connection: close");
    client.println();



    delay(1000);
#if LOG
      Serial.println(strURL);
#endif
  }
  else
  {   
#if LOG
      Serial.println("Errore Connessione");
      Serial.println("Disconnessione");
#endif
  }
}
















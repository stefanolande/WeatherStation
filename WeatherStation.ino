#include <SPI.h>
#include <Ethernet.h>


#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <dht.h>


#define UPDATE_INTERVAL 900000L //intervallo di aggiornamento


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

//variabili per collegarsi a internet
byte mac[] = { 
  0xDE, 0xAA, 0xBE, 0xEF, 0xAE, 0xDE  };

IPAddress ip(192,168,50,100); //indirizzo IP disponibile sulla rete

IPAddress myDns(192,168,1,1);

char server[] = "is0eir.altervista.org"; //sito web

EthernetClient client;

unsigned long lastConnectionTime = 0; //tempo di ultima connessione al server
boolean lastConnected = false;    
const unsigned long postingInterval = UPDATE_INTERVAL;

String strURL = "";


void setup()
{
  Serial.begin(9600);

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {
    }
  }

  delay(1000); //attendo un secondo

  if(!Ethernet.begin(mac)){
    Serial.println("DHPC error");
    while(1);
  }

  //invio al pc il mio IP
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
}

void loop()
{
  
    if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
  
    // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("Disconnessione");
    client.stop();
  }
  
  
    // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    sendData(); //carico i dati sul server
  }

  lastConnected = client.connected();

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
    
    Serial.println(server);
    Serial.println();

    if (client.connect(server, 80))
    {
      Serial.println("Connesso");

      char charBuf[7];

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
      
      //strURL = "GET /search?q=arduino HTTP/1.1";

      //invio la richiesta al server
      client.println(strURL);
      client.println("Host: is0eir.altervista.org");
      client.println("Connection: close");
      client.println();

      lastConnectionTime = millis();

      delay(1000);
      Serial.println(strURL);
    }
    else
    {
      Serial.println("Errore Connessione");
      Serial.println("Disconnessione");
      client.stop();
    }


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
}









/*
   Auteur : G.Menez
   Fichier : esp_asyncweb_spiffs.ino
   Many sources :
  => https://raw.githubusercontent.com/RuiSantosdotme/ESP32-Course/master/code/WiFi_Web_Server_DHT/WiFi_Web_Server_DHT.ino
  => https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/
  => Kevin Levy
*/
/*====== Import required libraries ===========*/
#include <ArduinoOTA.h>
#include "ArduinoJson.h"
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "classic_setup.h"
#include "sensors.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "SPIFFS.h"
#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include <time.h>
#include <HTTPClient.h>
#include "ArduinoJson.h"
#include <PubSubClient.h>
#include <Wire.h>


void setup_OTA(); // from ota.ino


/*===== MQTT TOPICS ===============*/
#define TOPIC_TEMP "my/sensors/projecttemperature"
#define TOPIC_LED  "my/sensors/projectlight"
#define TOPIC "iot/M1Miage2022"


/*===== ESP GPIO configuration ==============*/
/* ---- LED         ----*/
const int LEDpin = 19; // LED will use GPIO pin 19
/* ---- Light       ----*/
const int LightPin = A5; // Read analog input on ADC1_CHANNEL_5 (GPIO 33)
/* ---- Temperature ----*/
OneWire oneWire(23); // Pour utiliser une entite oneWire sur le port 23
DallasTemperature TempSensor(&oneWire) ; // Cette entite est utilisee par le capteur de temperature

/*====== ESP Statut =========================*/
// Ces variables permettent d'avoir une representation
// interne au programme du statut "electrique" de l'objet.
// Car on ne peut pas "interroger" une GPIO pour lui demander !
String LEDState = "off";

/*====== Process configuration ==============*/
// Set timer
unsigned long loop_period = 10L * 1000; /* =>  10000ms : 10 s */

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Thresholds
short int Light_threshold = 250; // Less => night, more => day

// Host for periodic data report
String target_ip = "";
int target_port = 1880;
int target_sp = 0; // Remaining time before the ESP stops transmitting


// MQTT Broker/server
const char* mqtt_server = "test.mosquitto.org"; // anynomous Ok in 2021

/*===== ESP is MQTT Client =======*/
WiFiClient espClient;           // Wifi 
PubSubClient client(espClient); // MQTT client
/*====== Some functions =====================*/

int numberBoucle = 0;


String processor(const String& var) {
  /* Replaces "placeholder" in  html file with sensors values */
  /* accessors functions get_... are in sensors.ino file   */
  //Serial.println(var);
  if (var == "TEMPERATURE") {
    return get_temperature(TempSensor);
  }
  else if (var == "LIGHT") {
    return get_light(LightPin);
  }
  else if (var == "SSID") {
    return String(WiFi.SSID());
  }
  else if (var == "MAC") {
    return String(WiFi.macAddress());
  }
  else if (var == "IP") {
    return WiFi.localIP().toString();
  }
  else if (var == "UPTIME") {
    /* lire l'heure courante */
   time_t now = time (NULL);

   /* la convertir en heure locale */
   struct tm tm_now = *localtime (&now);

   /* Creer une chaine JJ/MM/AAAA HH:MM:SS */
   char s_now[sizeof "JJ/MM/AAAA HH:MM:SS"];

   strftime (s_now, sizeof s_now, "%d/%m/%Y %H:%M:%S", &tm_now);

   /* afficher le resultat : */
   return ("'%s'\n", s_now);
  }
  else if (var == "WHERE") {
    return "43.61699047100423,7.064279531128759";
  }
  return String();
}

void setup_http_server() {
  /* Sets up AsyncWebServer and routes */

  // Declaring root handler, and action to be taken when root is requested
  auto root_handler = server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    /* This handler will download statut.html (SPIFFS file) and will send it back */
    request->send(SPIFFS, "/statut.html", String(), false, processor);
    // cf "Respond with content coming from a File containing templates" section in manual !
    // https://github.com/me-no-dev/ESPAsyncWebServer
    // request->send_P(200, "text/html", page_html, processor); // if page_html was a string .
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
    /* The most simple route => hope a response with temperature value */
   char tmp[20];
      sprintf(tmp,"%d",get_temperature(TempSensor)); 
       request->send_P(200, "text/plain", tmp);
  });

  server.on("/light", HTTP_GET, [](AsyncWebServerRequest * request) {
    /* The most simple route => hope a response with light value */

    char tmp[20];
      sprintf(tmp,"%d",get_light(LightPin)); 
       request->send_P(200, "text/plain", tmp);
  });

  // This route allows users to change thresholds values through GET params
  server.on("/set", HTTP_GET, [](AsyncWebServerRequest * request) {
    /* A route with a side effect : this get request has a param and should
        set a new light_threshold ... used for regulation !
    */
    if (request->hasArg("light_threshold")) {
      Light_threshold = atoi(request->arg("light_threshold").c_str());
      request->send_P(200, "text/plain", "Threshold Set !");
    }
  });

  server.on("/target", HTTP_POST, [](AsyncWebServerRequest * request) {
    /* A route receiving a POST request with Internet coordinates
        of the reporting target host.
    */
    Serial.println("Receive Request for a ""target"" route !");
    if (request->hasArg("ip") &&
        request->hasArg("port") &&
        request->hasArg("sp")) {
      target_ip = request->arg("ip");
      target_port = atoi(request->arg("port").c_str());
      target_sp = atoi(request->arg("sp").c_str());
    }
    request->send(SPIFFS, "/statut.html", String(), false, processor);
  });

  // If request doesn't match any route, returns 404.
  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404);
  });

  // Start server
  server.begin();
}

/*---- Arduino IDE paradigm : setup+loop -----*/
void setup() {
  Serial.begin(9600);
  while (!Serial); // wait for a serial connection. Needed for native USB port only

  // Connexion Wifi
  connect_wifi();
  print_network_status();

  // OTA
  setup_OTA();

  // Initialize the LED
  setup_led(LEDpin, OUTPUT, LOW);

 // Initialize the output variables as outputs
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, LOW);// Set outputs to 

  // Init temperature sensor
  TempSensor.begin();

  // Initialize SPIFFS
  SPIFFS.begin(true);

  setup_http_server();


    // set server of our client
  client.setServer(mqtt_server, 1883);
  // set callback when publishes arrive for the subscribed topic
  client.setCallback(mqtt_pubcallback); 
}

void loop() {
  int tempValue;

  ArduinoOTA.handle();

  tempValue = atoi(get_temperature(TempSensor).c_str());

  // Use this new temp for regulation updating ?
  if(target_ip != NULL || target_ip != "" ){
  httpUpload();
  
   }
      mqtt_upload(); 

   

  delay(5);
  //target_sp
}

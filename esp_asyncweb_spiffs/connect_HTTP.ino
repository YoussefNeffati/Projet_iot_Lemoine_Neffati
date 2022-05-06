 #include <stdio.h>
#include <time.h>
#include <HTTPClient.h>
#include "ArduinoJson.h"

 void httpUpload() {
 
 HTTPClient http;    //Declare object of class HTTPClient
 
 http.begin("http://" + target_ip + ":" + target_port + "/target");      //Specify request destination
    http.addHeader("Content-Type", "application/json");  //Specify content-type header
    

    int httpCode = http.POST(getJsonTP2());   //Send the request 
    Serial.println(httpCode);
    http.end();

 }


 

//==== MQTT Credentials =========
#ifdef MQTT_CRED
char *mqtt_id     = "Youssef";
char *mqtt_login  = "darkvador";
char *mqtt_passwd = "6poD2R2";
#else
char *mqtt_id     = "Youssef";
char *mqtt_login  = NULL;
char *mqtt_passwd = NULL;
#endif


/*============== CALLBACK ===================*/
void mqtt_pubcallback(char* topic, 
                      byte* message, 
                      unsigned int length) {
  /* 
   * Callback if a message is published on this topic.
   */
  Serial.print("Message arrived on topic : ");
  Serial.println(topic);
  Serial.print("=> ");

  // Byte list to String and print to Serial
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic,
  // you check if the message is either "on" or "off".
  // Changes the output state according to the message

  if (String(topic) == TOPIC_LED) {
    Serial.print("so ... changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      //setup_led(LEDpin, OUTPUT, HIGH);
      digitalWrite(LEDpin, HIGH);
    }
    else if (messageTemp == "off") {
      Serial.println("off");
      setup_led(LEDpin, OUTPUT, LOW);

    }
  }
}


/*============= SUBSCRIBE =====================*/
void mqtt_mysubscribe(char *topic) {
  /*
   * Subscribe to a MQTT topic 
   */
  while (!client.connected()) { // Loop until we're reconnected

    Serial.print("Attempting MQTT connection...");
    // Attempt to connect => https://pubsubclient.knolleary.net/api
    if (client.connect(mqtt_id, /* Client Id when connecting to the server */
                        mqtt_login,    /* No credential */ 
                        mqtt_passwd)) {
      Serial.println("connected");
      // then Subscribe topic
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      
      Serial.println(" try again in 5 seconds");
      delay(5000); // Wait 5 seconds before retrying
    }
  }
}




void mqtt_upload(){
  if (!client.connected()) { 
    mqtt_mysubscribe((char *)(TOPIC));
  }
  /*--- Publish Temperature periodically   */
  delay(5);
  // Convert the value to a char array
  char tempString[200];
  // Serial info
  Serial.print("Published Temperature : "); Serial.println(tempString);
  // MQTT Publish
  client.publish(TOPIC,GetJson().c_str());


  /* Process MQTT ... une fois par loop() ! */
  client.loop(); 
}

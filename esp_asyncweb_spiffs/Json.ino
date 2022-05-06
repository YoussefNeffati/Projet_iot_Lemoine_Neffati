

  String GetJson() {
  StaticJsonDocument<1000> jsondoc;
  jsondoc["status"]["temperature"] = get_temperature();
  jsondoc["status"]["light"] = get_light();


  jsondoc["info"]["ssid"] = String(WiFi.SSID());
  jsondoc["info"]["ident"] = String(WiFi.macAddress());
  jsondoc["info"]["ip"] = WiFi.localIP().toString();

  jsondoc["reporthost"]["target_ip"] = target_ip;
  jsondoc["reporthost"]["target_port"] = target_port;
  jsondoc["reporthost"]["sp"] = target_sp;


  String json;
  serializeJson(jsondoc, json);
  return(json);

  }

  String getJsonTP2(){
  DynamicJsonDocument doc(1024);
  doc["NOBODY"] = "la003759";
  doc["NOMAC"] = String(WiFi.macAddress());
  String json;
  serializeJson(doc, json);
  return(json);
  }

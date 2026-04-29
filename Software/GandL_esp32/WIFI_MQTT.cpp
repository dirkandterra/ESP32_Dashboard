//WIFI_MQTT
//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
#include "WIFI_MQTT.h"
#include "EE.h"

uint32_t retryTimeMQTT=0;
WiFiClient IntrepidDash;
PubSubClient client(IntrepidDash);
WiFiServer TelnetServer(8266);
char IPAddr[17]="0.0.0.0";
NodeRedData ServerData = {0,0,0,0,0,0,0,0,0,0,0,0};
IPAddress IP;
String SSIDName="";
//*WIFI STUFF*
// Replace with your network credentials
const char* ssidAP     = WIFI_SSID;
const char* passwordAP = WIFI_PWD;
void callbackMQTT(String topic, byte* message, unsigned int length);

void setup_MQTT(void){
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callbackMQTT);
}

void setup_WIFI(){ 
  // Connect to Wi-Fi
  int storedWifiRetry=20;
  WiFi.begin(EE.ssid, EE.pwd);
  while ((WiFi.status() != WL_CONNECTED) && (storedWifiRetry>0)) {
    delay(500);
    Serial.print(".");
    storedWifiRetry--;
  }
  if(storedWifiRetry){
      Serial.println("");
      Serial.println("WiFi connected.");
      Serial.println("IP address: ");
      SSIDName=EE.ssid;
      IP = WiFi.localIP();
      Serial.println(IP);
  }
  else{
    Serial.println("WiFi Failed on " + String(EE.ssid) + "!");
    WiFi.softAP(ssidAP, passwordAP);
    IP = WiFi.softAPIP();
    SSIDName=ssidAP;
    Serial.print("AP IP address: ");
    Serial.println(IP);
  } 
}
// ################ MQTT ROUTINES ##################

// This functions connects your ESP8266 to your MQTT broker
void checkMQTTConnection(uint32_t now) {
  // Try to reconnect every 20 sec if not connected or timed out
  if (!client.connected() || !client.loop()){
    if ((now>retryTimeMQTT) || retryTimeMQTT==0){
      retryTimeMQTT=now+20000;
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect("IntrepidEsp32",MOSQUITTO_USR,MOSQUITTO_PWD)) {
        Serial.println("connected");
        // Subscribe or resubscribe to a topic
        // You can subscribe to more topics (to control more LEDs in this example)
        client.subscribe("intrESP32/forecast");
        client.subscribe("esp32/GarageTemp");
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 20 seconds");
      }
    }
  }
}
// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// check the topic and handle the message
void callbackMQTT(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  char *token;
  char data[100];
  uint8_t *ptr8;
  int i=0;
  
  for (i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  if(topic=="esp32/GarageTemp"){
    ServerData.garageTemp=(uint16_t)round(messageTemp.toFloat());
    Serial.println(ServerData.currentTemp);
  }
  if(topic=="intrESP32/forecast"){
    messageTemp.toCharArray(data, messageTemp.length()+1);
    token = strtok(data, ","); // Get the first token
    if (token != NULL) {
      ServerData.currentTemp = (uint16_t)round(atof(token)*10); // Convert the token to a float
    }
    token = strtok(NULL, ","); // Get the next
    if (token != NULL) {
      ServerData.currentWind = (uint16_t)round(atof(token)); // Convert the token to a float
    }
    ptr8=ServerData.forecast;
    for(i=0;i<10;i++){
      token = strtok(NULL, ","); // Get the next
      if (token != NULL) {
        *ptr8 = (uint8_t)round(atof(token)); // Convert the token to a float
      }
      ptr8++;
    }
    token = strtok(NULL, ","); // Get the next
    if (token != NULL) {
      ServerData.precipPercent = (uint8_t)round(atof(token)); // Convert the token to a float
    }

  }
}
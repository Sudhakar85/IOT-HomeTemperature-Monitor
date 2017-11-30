#include <ESP8266WiFi.h>
#include "DHT.h"

// Board Settings
#define DHTPIN D2
#define DHTTYPE DHT11

// WIfi Settings
const char* ssid = "SSID";
const char* password = "******";
char* homeIpAddress;
IPAddress ip(192, 168, 1, 100);  // Desired IP Address
IPAddress gateway(192, 168, 1, 1); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0);

WiFiServer server(80);
WiFiClient apiClient;

// Sensor settings
DHT dht(DHTPIN, DHTTYPE);

// API Settings
String apiKey = "******************";
const char* apiServer = "api.thingspeak.com";

void setup() {

  Serial.begin(115200);
  delay(10);
  dht.begin();

  WiFi.config(ip, gateway, subnet);  
  WiFi.begin(ssid,password);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  server.begin();
  Serial.println("Server Started");
  Serial.println(WiFi.localIP());

  delay(2000);
}

void loop() {

  WiFiClient client = server.available();

  if(client && client.available())
    {
        float temp = dht.readTemperature(true,true);
        float h = dht.readHumidity();
        Serial.println(temp);
        Serial.println(h);
      
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println(""); //  do not forget this one
        client.println("<!DOCTYPE HTML>");
        client.println("<html style='height:100%;width:100%;color:white;background-color:#9FA8DA;'>");
        client.println("<body style='min-height: 100%;width:100%;'>");
        client.println("<h2>Sudhakar Home</h2>");  
        client.println("<h3>Current Room Temperacture : " + String(temp) +" F,  Humidity : " + String(h) +"</h3>");
        client.println("<iframe width='450' height='260' style='border: 1px solid #cccccc;' src='https://thingspeak.com/channels/278373/charts/1?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&type=line&days=15'></iframe>");        
        client.println("<iframe width='450' height='260' style='border: 1px solid #cccccc;' src='https://thingspeak.com/channels/278373/charts/2?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&type=line&days=15'></iframe>");
        client.println("</body></html>");
        Serial.println("client disconnected");
        client.stop();
        delay(10);
    }
    else
    {
        postData();        
        delay(2000);
    }
}

void postData()
{
      float t = dht.readTemperature(true,true);
      float h = dht.readHumidity();

      if(isnan(t) || isnan(h))
        return;
      
      String postStr = apiKey;
      postStr +="&field1=";
      postStr += String(t);
      postStr +="&field2=";
      postStr += String(h);
      postStr += "\r\n\r\n";

      if (apiClient.connect(apiServer,80)) 
      {
          apiClient.print("POST /update HTTP/1.1\n");
          apiClient.print("Host: api.thingspeak.com\n");
          apiClient.print("Connection: close\n");
          apiClient.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
          apiClient.print("Content-Type: application/x-www-form-urlencoded\n");
          apiClient.print("Content-Length: ");
          apiClient.print(postStr.length());
          apiClient.print("\n\n");
          apiClient.print(postStr);

          Serial.println("Post success");
          apiClient.stop();
      }
} 

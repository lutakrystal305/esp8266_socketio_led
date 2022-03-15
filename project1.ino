#include<Arduino.h>
#include<WebSocketsClient.h>
#include <SocketIOclient.h>
#include<DHT.h>
#include<SPI.h>
#include<Wire.h>
#include<SSD1306.h>
#include<Adafruit_GFX.h>
#include<Adafruit_SSD1306.h>

#include <ArduinoJson.h>
#include <Hash.h>


//#include<WiFiClientSecure.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
//#include <ESP8266HTTPClient.h>

#define DHTPIN 14
#define DHTTYPE DHT11


DHT dht(DHTPIN, DHTTYPE);

ESP8266WiFiMulti WiFiMulti;
//_client.tcp = new WiFiClientSecure();
const char* ssid = "ATNT";
const char* password = "motdenchin";
SSD1306 display(0x3c, 4, 5);
const String host = "https://min-api.cryptocompare.com/data/price?fsym=BTC&tsyms=USD";

//WebSocketsClient webSocket;
SocketIOclient socketIO;

void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
//  //Serial.print(payload);
    switch(type) {
        case sIOtype_DISCONNECT:
            Serial.printf("[IOc] Disconnected!\n");
            break;
        case sIOtype_CONNECT:
            Serial.printf("[IOc] Connected to url: %s\n", payload);
            // join default namespace (no auto join in Socket.IO V3)
            socketIO.send(sIOtype_CONNECT, "/");
            break;
        case sIOtype_EVENT:
        {
            Serial.printf("[IOc] get event: %s\n", payload);
            String txt = (const char *) &payload[0];
            Serial.println((const char *) &payload[2]);
            Serial.println(txt);
            Serial.println(txt[2]);
            if (txt[2] == '~') {
              Serial.println("True request");
              digitalWrite(0, HIGH); // Khi client phát sự kiện "LED_ON" thì server sẽ bật LED
              digitalWrite(12, HIGH);  
              digitalWrite(13, HIGH);
            } else if (txt[2] == '!') {
              Serial.println("OFF");
              digitalWrite(0, LOW); // Khi client phát sự kiện "LED_OFF" thì server sẽ tắt LED
              digitalWrite(12, LOW);
              digitalWrite(13, LOW);
            }
        }
            break;
        case sIOtype_ACK:
            Serial.printf("[IOc] get ack: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_ERROR:
            Serial.printf("[IOc] get error: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_BINARY_EVENT:
            Serial.printf("[IOc] get binary: %u\n", length);
            hexdump(payload, length);
            break;
        case sIOtype_BINARY_ACK:
            Serial.printf("[IOc] get binary ack: %u\n", length);
            hexdump(payload, length);
            break;
    }
}

unsigned long messageTimestamp = 0;
void showTemp(float a, float b) {
  uint64_t now = millis();
  if(now - messageTimestamp > 5000) {
    messageTimestamp = now;
    DynamicJsonDocument doc(1024);
    JsonArray array = doc.to<JsonArray>();
          
    // add evnet name
    // Hint: socket.on('event_name', ....
    array.add("toEsp");
  
    // add payload (parameters) for the event
    JsonObject param1 = array.createNestedObject();
    param1["now"] = a ; //(uint32_t) now;
    param1["now1"] = b ;
    // JSON to String (serializion)
    String output;
    serializeJson(doc, output);
  
          // Send event        
    socketIO.sendEVENT(output);
  
          // Print JSON for debugging
    Serial.println(output);    
  } 
  //socketIO.sendEVENT(a);
  display.clear();
  display.drawString(0, 0, "Temperature:" + String(a));
  display.drawString(0, 18, "Humidity:" + String(b));
  display.display();
}
void setup() {
  // put your setup code here, to run once:
  pinMode(0, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(0, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  
  dht.begin();
  
  Serial.println(".");
  display.init();
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0,0, "Connecting to");
  display.drawString(0, 18, ssid);
  display.display();
  WiFiMulti.addAP(ssid, password);
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  display.clear();
  display.drawString(0, 0, "Connected");
  display.display();
  //WiFiClientSecure.setInsecure(); //websocket.beginSocketIOSSL
  socketIO.beginSSL("myvxk.sse.codesandbox.io", 443, "/socket.io/?EIO=4");
  //webSocket.beginSocketIOSSL("myvxk.sse.codesandbox.io", 443, "/socket.io/?EIO=4", NULL);
  socketIO.setExtraHeaders("");
  socketIO.onEvent(socketIOEvent);
  //webSocket.onEvent(webSocketEvent);
}

//unsigned long messageTimestamp = 0;
void loop() {
  // put your main code here, to run repeatedly:
  socketIO.loop();
  //webSocket.loop();
  //uint64_t now = millis();
  float temp = dht.readTemperature();
  float humi = dht.readHumidity();
  //delay(10000);
  if (isnan(temp) || isnan(humi)) {
    //Serial.println(temp);
    //Serial.println("Failed to read from DHT sensor!");
    return;
  }
  showTemp(temp, humi);
}

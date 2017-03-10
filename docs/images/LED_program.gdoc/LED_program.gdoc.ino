int inPin = 12;
int outPin = 13;
#include <ESP8266WiFi.h>

//WiFi login
const char* WIFI_ID = "chinnu-guest";
const char* PASSWORD = "5106813202";

//IFTT access information
const char* HOST = "maker.ifttt.com";
const int HTTP_PORT = 80;
const char* IFTTT_KEY = "b39ILa-_sUmnrrgS6mvJ_y";

//Event names to be sent in web request to signal specific notification to phone
const char* EVENT_LEAK = "ToiletLeak";
const char* EVENT_CLEAR = "ToiletLeakCleared";

//Time interval between input checks
const int INTERVAL = 2*1000;
//Number of intervals to wait before sending web request
const int NUM_INTERVAL = 6;



void setup() {
  Serial.begin(115200);
  Serial.print("\n connecting to wifi ");
  Serial.println(WIFI_ID);
  WiFi.begin(WIFI_ID, PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  pinMode(inPin,INPUT_PULLUP);
  pinMode(outPin,OUTPUT);
 
}

//sends web request to maker.ifttt.com
void sendEvent(const char* eventName) {
  WiFiClient client;
  if (!client.connect(HOST, HTTP_PORT)) {
    Serial.println( "Connection Failed");
    return;
  }

  String url = String ("/trigger/") + eventName + "/with/key/" + IFTTT_KEY;
  Serial.print("Sending Event ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" + 
        "Host: " + HOST + "\r\n" +
        "Connection: close\r\n\r\n");

  client.stop();  
}



void loop() {
  int state;
  delay(INTERVAL);

  //if toilet is filled then return
  state = digitalRead(inPin);
  if (state == LOW) {
    return;
  }

  //repeated checks to see if toilet is filling for longer than NUM_INTERVAL
  for (int i = 0; i < NUM_INTERVAL; i++) {
      
      //read tank level (toilet state)
      state = digitalRead(inPin);
      
      // if tank is not filling (tank is full), then exit for loop  
      if (state == LOW) {
        Serial.println("Toilet is filled");
        return;
      }
      
      //tank is filling so blink LED and repoll
      digitalWrite(outPin, HIGH);
      delay(500);
      digitalWrite(outPin, LOW);

      Serial.print("Tank is filling for ");
      Serial.print((i+1)*INTERVAL/1000);
      Serial.println(" seconds.");

      //sleep for interval time
      delay(INTERVAL);
  }

  //toilet is leaking, so send event and turn on LED
  sendEvent(EVENT_LEAK);
  digitalWrite(outPin, HIGH);
  Serial.println("Toilet is leaking");

  //continuously checking tank level until leak is cleared (tank is full)
  while (state != LOW) {
    state = digitalRead(inPin);
    delay(INTERVAL);
  }

  //Leak is clear so notifying that leak has been cleared and turning off LED
  sendEvent(EVENT_CLEAR);
  digitalWrite(outPin, LOW);
  Serial.println("Leak is clear");
}

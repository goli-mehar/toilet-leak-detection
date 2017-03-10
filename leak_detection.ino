int inPin = 12;
int outPin = 13;
#include <ESP8266WiFi.h>

//WiFi login
const char* WIFI_ID = "Mehar's iPhone";
const char* PASSWORD = "14082197612";

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

const int STATE_FILLING = LOW;



void setup() {
  Serial.begin(115200);
  Serial.print("\n connecting to wifi ");
  Serial.println(WIFI_ID);
  WiFi.begin(WIFI_ID, PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  pinMode(inPin,INPUT_PULLUP);
  pinMode(outPin,OUTPUT);

  //Two beeps to signal that WiFi is connected
  digitalWrite(outPin, LOW);
  delay(200);
  digitalWrite(outPin, HIGH);
  delay(200);
  digitalWrite(outPin, LOW);
  delay(200);
  digitalWrite(outPin, HIGH); //Speaker needs default high
  
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
 
 
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
  if (state != STATE_FILLING) {
    return;
  }

  //repeated checks to see if toilet is filling for longer than NUM_INTERVAL
  for (int i = 0; i < NUM_INTERVAL; i++) {
      
      //read tank level (toilet state)
      state = digitalRead(inPin);
      
      // if tank is not filling (tank is full), then exit for loop  
      if (state != STATE_FILLING) {
        Serial.println("Toilet is filled");
        return;
      }
      
      //tank is filling so blink LED and repoll
      digitalWrite(outPin, LOW);
      delay(100);
      digitalWrite(outPin, HIGH);

      Serial.print("Tank is filling for ");
      Serial.print((i+1)*INTERVAL/1000);
      Serial.println(" seconds.");

      //sleep for interval time
      delay(INTERVAL);
  }

  //toilet is leaking, so send event and turn on LED
  sendEvent(EVENT_LEAK);
  digitalWrite(outPin, LOW);
  Serial.println("Toilet is leaking");

  //continuously checking tank level until leak is cleared (tank is full)
  while (state == STATE_FILLING) {
    state = digitalRead(inPin);
    delay(INTERVAL);
  }

  //Leak is clear so notifying that leak has been cleared and turning off LED
  sendEvent(EVENT_CLEAR);
  digitalWrite(outPin, HIGH);
  Serial.println("Leak is clear");
}

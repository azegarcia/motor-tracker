#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

const int RXPin = 4, TXPin = 5;
SoftwareSerial neo6m(RXPin, TXPin);
TinyGPSPlus gps;


// wifi 2.4GHz credentials
#define WIFI_SSID "DORM2G"
#define WIFI_PASSWORD "palasyoniskye"

// create firebase project. get host and auth.
#define FIREBASE_HOST "motoracc-tracker-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "kSLlkxtCFscZ375XgZjHTKdifH705qpjeFZPAni8"

unsigned long previousMillis = 0;
const long interval = 60000;

unsigned long previousMillis1 = 0;
const long interval1 = 2000;

String pushKey;

String formattedDate;
String dayStamp;
String timeStamp;

const long utcOffsetInSeconds = 28800;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

FirebaseData fbdo;
int flag = 0;
void setup() {
  Serial.begin(115200);
  timeClient.begin();
  neo6m.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  delay(1000);

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  delay(2000);
  Firebase.set(fbdo, "/latest/trigger/", 0);
}

static void smartdelay_gps(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (neo6m.available())
      gps.encode(neo6m.read());
  } while (millis() - start < ms);
}

void send_data(float x, float y) {
  String coord = String(x,6) + "," + String(y,6);
  while (!timeClient.update())
  {
    timeClient.forceUpdate();
  }

  formattedDate = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
  Serial.print("TIME: ");
  Serial.println(timeStamp);

  Firebase.set(fbdo, "/latest/date/", dayStamp);
  Firebase.set(fbdo, "/latest/time/", timeStamp);
  Firebase.set(fbdo, "/latest/GPS/", coord);


  Firebase.push(fbdo, "/data/", "keyActive");
  pushKey = fbdo.pushName();
  Firebase.set(fbdo, "/data/" + pushKey + "/date/", dayStamp);
  Firebase.set(fbdo, "/data/" + pushKey + "/time/", timeStamp);
  Firebase.set(fbdo, "/data/" + pushKey + "/GPS/", coord);

  Firebase.set(fbdo, "/latest/key/", pushKey);
  Firebase.set(fbdo, "/latest/trigger/", 1);
  flag = 1;
}

void loop() {
  unsigned long currentMillis1 = millis();
  if (currentMillis1 - previousMillis1 >= interval1) {
    previousMillis1 = currentMillis1;
    if (Firebase.getInt(fbdo, "/latest/trigger/")) {
      flag = fbdo.intData();
    }
  }

  if (flag == 0) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      smartdelay_gps(1000);
      if (gps.location.isValid()) {
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();

        //-------------------------------------------------------------
        //    Send to Serial Monitor for Debugging
        Serial.print("LAT:  ");
        Serial.println(latitude);  // float to x decimal places
        Serial.print("LONG: ");
        Serial.println(longitude);
        //-------------------------------------------------------------

        send_data(latitude, longitude);
      } else {
        Serial.println("No valid GPS data found.");
      }
    }
  }
}

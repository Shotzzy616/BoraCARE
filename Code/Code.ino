#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


#define WIFI_SSID "Shotzzy"
#define WIFI_PASSWORD "cosmos1234"

#define API_KEY "AIzaSyB8bgRYUTJTOHbAEtlE0YQn45UivsXncB0"
#define DATABASE_URL "https://esp32-v1-b7211-default-rtdb.europe-west1.firebasedatabase.app/" 

#define LED 2
#define DHTPIN 13
#define DHTTYPE DHT11

FirebaseData fbdo, fbdo_s1;
FirebaseAuth auth;
FirebaseConfig config;

DHT dht(DHTPIN, DHTTYPE);

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
float temperature;
float humidity;
bool ledStatus;

void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!Firebase.RTDB.beginStream(&fbdo_s1, "/LED/Digital")){
    Serial.printf("stream 1 begin error, %s\n\n", fbdo_s1.errorReason().c_str());}
}
void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    if (Firebase.RTDB.setFloat(&fbdo, "Sensor/Humidity", humidity)) {
      Serial.println();Serial.print(humidity);
      Serial.print(" - successfuly saved to: " + fbdo.dataPath());
      Serial.println(" (" + fbdo.dataType() + ") ");
    }
    else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.setFloat(&fbdo, "Sensor/Temprature", temperature)) {
      Serial.println();Serial.print(temperature);
      Serial.print(" - successfuly saved to: " + fbdo.dataPath());
      Serial.println(" (" + fbdo.dataType() + ") ");
    }
    else {
      Serial.println("FAILED: " + fbdo.errorReason());
    }
  }
  if (Firebase.ready() && signupOK){
    if (!Firebase.RTDB.readStream(&fbdo_s1)){
      Serial.printf("stream 1 read error, %s\n\n", fbdo_s1.errorReason().c_str());}
    if (fbdo_s1.streamAvailable()) {
      if (fbdo_s1.dataType() == "boolean") {
        ledStatus = fbdo.boolData();
        Serial.println("Successfuly read from " + fbdo_s1.dataPath() + ": " + ledStatus + " (" + fbdo.dataType() + ")");
        digitalWrite(LED, ledStatus);
      }
    }
  }
}

#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <WiFiClientSecure.h>
#define WIFI_SSID "Galaxy M31F428"
#define WIFI_PASSWORD "1sampai8"
#define relay1 26 // pompa 1
#define relay2 5  // pompa 2
#define relay3 4  // RELAY TO SENSOR PH1
#define relay4 2  // RELAY TO SENSOR PH1
#define sensorpH1 32 // pin sensorpH1
#define sensorpH2 33 // pin sensorpH2
const int SoilSensor1 = 35; // pin sensor soil 1
const int SoilSensor2 = 34; // pin sensor soil 2

#define API_KEY "AIzaSyDUvmdFs3ueUszT3txsNlsSDs4WFgggJN8"
#define DATABASE_URL "https://cabai-f6c80-default-rtdb.asia-southeast1.firebasedatabase.app/"

const char* host = "script.google.com";
const int httpsPort = 443;
WiFiClientSecure client;
String GAS_ID = "AKfycbyne1aNfBSIKojpc_cSlc-XU0RWLjZCRyFzbhR0QHx6ScXJupdRsH7N2xzkJEPZVEkv";

int sensorValue1 = 0;       
float outputValue1 = 0.0;        
int sensorValue2= 0;        
float outputValue2= 0.0; 
bool pompa1 = false;
bool pompa2 = false;
bool pompaAuto = false;

FirebaseData fbdo;

FirebaseAuth auth;

FirebaseConfig config;
unsigned long dataMillis = 0;
unsigned long dataMillis2 = 0;
int count = 0;
bool signupOK = false;


void setup()
{
  Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

   

    /* Assign the API key (required) */
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    Firebase.reconnectNetwork(true);
    fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
       if (Firebase.signUp(&config, &auth, "", ""))
    {
        Serial.println("ok");
        signupOK = true;

        /** if the database rules were set as in the example "EmailPassword.ino"
         * This new user can be access the following location.
         *
         * "/UserData/<user uid>"
         *
         * The new user UID or <user uid> can be taken from auth.token.uid
         */
    }
    else
        Serial.printf("%s\n", config.signer.signupError.message.c_str());
    config.token_status_callback = tokenStatusCallback;
     Firebase.begin(&config, &auth);
     // put your setup code here, to run once:
  pinMode (sensorpH1, INPUT);
  pinMode (sensorpH2, INPUT);
  pinMode (relay1, OUTPUT);
  pinMode (relay2, OUTPUT);
  pinMode (relay3, OUTPUT);
  pinMode (relay4, OUTPUT); 

  // Mengatur relay off pada awalnya
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);

  client.setInsecure();
}

void loop()
{
// put your main code here, to run repeatedly:
  // pembacaan sensor soil
  int kelembabanTanah;
  int hasilPembacaan = analogRead(SoilSensor1);
  kelembabanTanah = map(hasilPembacaan, 4095, 0, 0, 100);

  int kelembaban;
  int hasil = analogRead(SoilSensor2);
  kelembaban = map(hasil, 4095, 0, 0, 100);

  int yl;
  yl = ((kelembabanTanah+kelembaban)/2);

      // Mengatur relay3 on dengan delay 10 detik
  digitalWrite(relay3, LOW);

      // Mengatur relay4 on dengan delay 10 detik
  digitalWrite(relay4, LOW);

      // Membaca nilai ADC dari sensor pH tanah 1
  sensorValue1 = analogRead(sensorpH1);
  outputValue1 = (-0.0023*sensorValue1)+10.335;

      // Membaca nilai ADC dari sensor pH tanah 2
  sensorValue2 = analogRead(sensorpH2);
  outputValue2 = (-0.0023*sensorValue2)+10.404;
  
      // Mengatur relay3 off dengan delay 5 detik
  digitalWrite(relay3, HIGH);

      // Mengatur relay4 off dengan delay 5 detik
  digitalWrite(relay4, HIGH);

  float ph;
  ph = ((outputValue1+outputValue2)/2);


  if (millis() - dataMillis > 2000 && signupOK && Firebase.ready())
    {
        dataMillis = millis();
        Firebase.setInt(fbdo, "soilmoisture/soil1", kelembabanTanah) ? "ok" : fbdo.errorReason().c_str();
        Firebase.setInt(fbdo, "soilmoisture/soil2", kelembaban) ? "ok" : fbdo.errorReason().c_str();
        Firebase.setInt(fbdo, "soilmoisture/average", yl) ? "ok" : fbdo.errorReason().c_str();
        Firebase.setInt(fbdo, "pHtanah/pH1", outputValue1) ? "ok" : fbdo.errorReason().c_str();
        Firebase.setInt(fbdo, "pHtanah/pH2", outputValue2) ? "ok" : fbdo.errorReason().c_str();
        Firebase.setInt(fbdo, "pHtanah/average", ph) ? "ok" : fbdo.errorReason().c_str();

        if(Firebase.getBool(fbdo, "pompa/1")){
            pompa1 = fbdo.boolData();
            Serial.print(pompa1);
            Serial.println(" - saved to: " + fbdo.dataPath());
          } else {
              Serial.println(fbdo.errorReason().c_str());
            }

              if(Firebase.getBool(fbdo, "pompa/2")){
            pompa2 = fbdo.boolData();
            Serial.print(pompa2);
            Serial.println(" - saved to: " + fbdo.dataPath());
          } else {
              Serial.println(fbdo.errorReason().c_str());
            }

              if(Firebase.getBool(fbdo, "pompa/auto")){
            pompaAuto = fbdo.boolData();
            Serial.print(pompaAuto);
            Serial.println(" - saved to: " + fbdo.dataPath());
          } else {
              Serial.println(fbdo.errorReason().c_str());
            }
    }
    

    if (pompaAuto == 1){
      if (yl < 60.00){
        digitalWrite(relay1,LOW);
       }else{
          digitalWrite(relay1,HIGH);
         }
         
      if (ph < 5.5){
        digitalWrite(relay2,LOW);
      }else{
        digitalWrite(relay2,HIGH);
      }
    } else if(pompa1 == 1){
        digitalWrite(relay1,LOW);
      }
     else if(pompa2 == 1){
        digitalWrite(relay2,LOW);
      }
    else{
      digitalWrite(relay1,HIGH);
      digitalWrite(relay2,HIGH);
    }

  Serial.print("Nilai ADC Kelembaban Tanah 1");
  Serial.print(hasilPembacaan);
  Serial.print("Persentase Kelembaban Tanah 1 = ");
  Serial.print(kelembabanTanah);
  Serial.println("%");

  Serial.print("Nilai ADC Kelembaban Tanah 2");
  Serial.print(hasil);
  Serial.print("Persentase Kelembaban Tanah 2 = ");
  Serial.print(kelembaban);
  Serial.println("%");

  Serial.print("Presentase Kelembaban Tanah = ");
  Serial.print(yl);
  Serial.println("%");

  // Mengirimkan nilai ADC ke Serial Monitor
  Serial.print("Nilai ADC: ");
  Serial.println(sensorValue1);
  Serial.print("output Ph1= ");
  Serial.println(outputValue1);
  
  // Mengirimkan nilai ADC ke Serial Monitor
  Serial.print("Nilai ADC: ");
  Serial.println(sensorValue2);
  Serial.print("output Ph2= ");
  Serial.println(outputValue2);

  Serial.print("Presentase pH Tanah = ");
  Serial.print(ph);
  delay(3000);

  sendData(ph, outputValue1, outputValue2, yl,kelembabanTanah, kelembaban);
}

void sendData(float ph, float outputValue1,float outputValue2, int yl, int kelembabanTanah, int kelembaban){
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

 String string_kelembabanTanah = String (kelembabanTanah);
 String string_kelembaban = String (kelembaban);
 String string_yl = String (yl);
 String string_outputValue1 = String (outputValue1);
 String string_outputValue2 = String (outputValue2);
 String string_ph = String (ph);
 String string_pompa1 = String (pompa1);
 String string_pompa2 = String (pompa2);
 String string_pompaauto = String (pompaAuto);

 String url = "/macros/s/" + GAS_ID + "/exec?averagepH=" + string_ph + "&pH1=" + string_outputValue1 + "&pH2=" + string_outputValue2 + "&average=" + string_yl + "&soil1=" + string_kelembabanTanah + "&soil2=" + string_kelembaban + "&pompa1=" + string_pompa1 + "&pompa2=" + string_pompa2 + "&pompaauto=" + string_pompaauto;
 Serial.print("requesting URL: ");
 Serial.println(url);

 client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP32\r\n" +
         "Connection: close\r\n\r\n");

 Serial.println("request sent"); 

 while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp32/Arduino CI successfull!");
  } else {
    Serial.println("esp32/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
}

#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Network credentials
const char* ssid = "ZTE_2.4G_3uNiGZ";
const char* password = "ebWhbGEe";
const char* uid = "";
const char* farmId = "";

#define SendKey 0  //Button to send data Flash BTN on NodeMCU
#define POT_PIN A0 // Potentiometer

// Firebase project credentials
#define FIREBASE_HOST "iot-farminc-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "rIAZZbuXVI2jz95CIdhhiaA9WClprI7huHc34T9n"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

FirebaseData firebaseData;

int LEDpins[] = {16, 5, 4};

void setup() {
  Serial.begin(9600);
  delay(3000);
  WiFi.disconnect();
  pinMode(SendKey,INPUT_PULLUP);  //Btn to send data
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting to......");
    Serial.println(ssid);
    delay(500);
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectNetwork(true);

  for (int i = 0; i < sizeof(LEDpins) / sizeof(LEDpins[0]); i++) {
    pinMode(LEDpins[i], OUTPUT);
    digitalWrite(LEDpins[i], LOW);
  }

  // Test write to Firebase
  if (Firebase.setString(fbdo, "/test/message", "Hello from ESP8266")) {
    Serial.println("Data sent to Firebase!");
  } else {
    Serial.println("FAILED: " + fbdo.errorReason());
  }
}

void loop() {
  if (Firebase.getString(fbdo, "/led_status")) {
    String command = fbdo.stringData();
    Serial.println("Command from Firebase: " + command);

    if (command == "ON") {
      for (int i = 0; i < sizeof(LEDpins) / sizeof(LEDpins[0]); i++) {
        digitalWrite(LEDpins[i], HIGH);
      }
    } else if (command == "OFF") {
      for (int i = 0; i < sizeof(LEDpins) / sizeof(LEDpins[0]); i++) {
        digitalWrite(LEDpins[i], LOW);
      }
    }
  } else {
    Serial.print("Firebase get failed: ");
    Serial.println(fbdo.errorReason());
  }

  int pot_val = analogRead(POT_PIN);
  unsigned long ts = millis();
  String path = String("/users/") + 
                uid +
                "farms/" +
                "/readings/" +
                String(ts);
  if (Firebase.setInt(fbdo, path, pot_val)) {
    Serial.printf("Sent %d @ %lu\n", pot_val, ts);
  } else {
    Serial.printf("Err: %s\n", fbdo.errorReason().c_str());
  }

  delay(3000); // poll every 3 seconds
}

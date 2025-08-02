#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// Network credentials
const char* ssid = "David";
const char* password = "12345678";
const char* uid = "NoOch2JDjMP6jf39jQghIUYLua33";
const char* farmId = "jVRKwU5P5x2s3zfcYhED";
const char* cropId = "9401f505-62e5-488f-8aa3-c18c26e2755e";

#define SendKey 0  //Button to send data Flash BTN on NodeMCU
#define SOIL_MOISTURE A0 // Potentiometer

// Firebase project credentials
#define FIREBASE_HOST "iot-farminc-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "rIAZZbuXVI2jz95CIdhhiaA9WClprI7huHc34T9n"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

FirebaseData firebaseData;

int LEDpins[] = {16, 5, 4};
int relay = 14;

const unsigned long INTERVAL = 10UL * 60UL * 1000UL;  // 10 minutes in ms
unsigned long prevMillis = 0; 

void setup() {
  Serial.begin(9600);
  delay(3000);
  WiFi.disconnect();
  pinMode(SendKey,INPUT_PULLUP);  //Btn to send data
  Serial.println();

  for (int i = 0; i < sizeof(LEDpins) / sizeof(LEDpins[0]); i++) {
    pinMode(LEDpins[i], OUTPUT);
    digitalWrite(LEDpins[i], LOW);
  }

  pinMode(relay, OUTPUT);

  // no wifi connection: red light
  digitalWrite(LEDpins[2], HIGH);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting to......");
    Serial.println(ssid);
    delay(500);
  }

  // wifi connected: green light
  digitalWrite(LEDpins[2], LOW);
  digitalWrite(LEDpins[1], HIGH);

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

  // Test write to Firebase
  if (Firebase.setString(fbdo, "/test/message", "Hello from ESP8266")) {
    Serial.println("Data sent to Firebase!");
  } else {
    Serial.println("FAILED: " + fbdo.errorReason());
  }
}

void loop() {
  String valveReading = String("/users/") + 
                uid +
                "/farms/" +
                farmId +
                "/crops/" +
                cropId +
                "/irrigate";
  if (Firebase.getString(fbdo, valveReading)) {
    String command = fbdo.stringData();
    Serial.println("Command from Firebase: " + command);

    if (command == "ON") {
      // close relay to turn on solenoid valve
      digitalWrite(relay, LOW);
      digitalWrite(LEDpins[0], LOW);
    } else if (command == "OFF") {
      // open relay to turn off solenoid valve
      digitalWrite(relay, HIGH);
      digitalWrite(LEDpins[0], HIGH);
    }
  } else {
    Serial.print("Firebase get failed: ");
    Serial.println(fbdo.errorReason());
  }

  unsigned long currentMillis = millis();
  if (prevMillis == 0 || currentMillis - prevMillis >= INTERVAL) {
    int sm_val = analogRead(SOIL_MOISTURE);
    String dataPath = String("/users/") + uid
                + "/farms/" + farmId 
                + "/crops" + cropId
                + "/soil-moisture/"
                + String(currentMillis);

    if (Firebase.setInt(fbdo, dataPath, sm_val)) {
      Serial.printf("Sent %d @ %lu\n", sm_val, currentMillis);
    } else {
      Serial.printf("Err: %s\n", fbdo.errorReason().c_str());
    }
    prevMillis = currentMillis;
  }

  delay(1000); // poll every 1 second: I don't mind 1 second affecting my timing
}

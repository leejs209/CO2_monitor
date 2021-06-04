
#include <ErriezMHZ19B.h>
#include <SoftwareSerial.h>


// Pin defines
#define MHZ19B_TX_PIN        9
#define MHZ19B_RX_PIN        10

// Create software serial object
SoftwareSerial mhzSerial(MHZ19B_TX_PIN, MHZ19B_RX_PIN);

// Create MHZ19B object with software serial
ErriezMHZ19B mhz19b(&mhzSerial);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.begin(9600);
    mhzSerial.begin(9600);
    while ( !mhz19b.detect() ) {
        Serial.println(F("Detecting MH-Z19B sensor..."));
        delay(2000);
    };
    Serial.println(F("보정 시작"));
    mhz19b.startZeroCalibration();
    Serial.println(F("보정 완료"));
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}

#include <SD.h>

#include <DHT.h>
#include <DHT_U.h>

#include <ErriezMHZ19B.h>
#include <SoftwareSerial.h>
#include "RTClib.h"


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define chipSelect 5
#define DHTPIN 2     // 온도센서 핀
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT_Unified dht(DHTPIN, DHTTYPE);

RTC_DS1307 rtc;

// Pin defines
#define MHZ19B_TX_PIN        25
#define MHZ19B_RX_PIN        26

// Create software serial object
SoftwareSerial mhzSerial(MHZ19B_TX_PIN, MHZ19B_RX_PIN);

// Create MHZ19B object with software serial
ErriezMHZ19B mhz19b(&mhzSerial);

unsigned long time_now = 0;
int period = 15000; //측정 주기

void setup()
{
    // Initialize serial
    Serial.begin(9600);
    // Serial.println("CO2 & Temp & Humidity Logger");

    
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
    }

    display.display();
    
    // CO2 센서
    mhzSerial.begin(9600);
    while ( !mhz19b.detect() ) {
        Serial.println(F("Detecting MH-Z19B sensor..."));
        delay(2000);
    };

    // 24시간마다 자동으로 보정되는 기능 혜제 (대신 400ppm의 야외에서 startZeroCalibration()으로 직접 보정해야 함)
    
    mhz19b.setAutoCalibration(false);
    //  rtc setup
    if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
      Serial.flush();
      abort();
    }
  
    if (! rtc.isrunning()) {
      Serial.println("RTC is NOT running, let's set the time!");
      // When time needs to be set on a new device, or after a power loss, the
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

    // 온도센서 setup
    dht.begin();
    Serial.println("timestamp,co2,temp,humidity");
  
    Serial.print("Initializing SD card...");
  
    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {
      Serial.println("Card failed, or not present");
      // don't do anything more:
      while (1);
    }
    Serial.println("card initialized.");

//     pinMode(LED_BUILTIN, OUTPUT);
}



void loop()
{
    time_now = millis();
    
//    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
//    delay(100);                       // wait for a second
//    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    int16_t result;
    String print_val = "";
    String val = "";
    String co2_disp = "";
    String temp_disp = "";
    String hum_disp = "";
    float t;
    float h;
    
    //temp read
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println(F("Error reading temperature!"));
    }
    else {
      t = event.temperature;
    }
    
    // Get humidity event and print its value.
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      Serial.println(F("Error reading humidity!"));
    }
    else {
      h = event.relative_humidity;
    }

    //CO2 read
    if (mhz19b.isReady()) {
        // Read CO2 from sensor
        result = mhz19b.readCO2();

        // Print result
        if (result < 0) {
            // Print error code
            switch (result) {
                case MHZ19B_RESULT_ERR_CRC:
                    Serial.println(F("CRC error"));
                    break;
                case MHZ19B_RESULT_ERR_TIMEOUT:
                    Serial.println(F("RX timeout"));
                    break;
                default:
                    Serial.print(F("Error: "));
                    Serial.println(result);
                    break;
            }
        } else {
            DateTime now = rtc.now();
            print_val += now.timestamp(DateTime::TIMESTAMP_FULL);
            print_val += ",";
            val += String(result);
            val += ",";
            val += t;
            val += ",";
            val += h;
            print_val += val;
            
            Serial.println(print_val);
    
            // 디스플레이에 표시
            co2_disp += "CO2: ";
            co2_disp += String(result);
            co2_disp += " ppm";

            temp_disp += "Temp: ";
            temp_disp += t;
            temp_disp += " C";

            hum_disp += "Humidity: ";
            hum_disp += h;
            hum_disp += " %";
            
            display.clearDisplay();
            display.setTextSize(1);      // Normal 1:1 pixel scale
            display.setTextColor(SSD1306_WHITE); // Draw white text
            //display.setCursor(0, 0);
            //display.println(now.year() + '-' + now.month() + '-' + now.day() + ' ' + now.hour() + ':' + now.minute() + ':' + now.second());
            display.setCursor(0, 0);     // Start at top-left corner
            display.println(co2_disp);
            display.setCursor(0, 10);
            display.println(temp_disp);
            display.setCursor(0, 20);
            display.println(hum_disp);
            display.display();
            
            File dataFile = SD.open("/datalog.txt", FILE_WRITE);
            
            // if the file is available, write to it:
            
            if (dataFile) {
              dataFile.println(print_val);
              dataFile.close();            
            }
            // if the file isn't open, pop udp an error:
            else {
              Serial.println("error opening datalog.txt");
            }
        }
    }
    while(millis() < time_now + period){
    }
}

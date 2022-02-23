


#include <NTPClient.h>
#include <WiFiUdp.h>
#define wifi D5
const long utcOffsetInSeconds = 10800;
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
#define BLYNK_TEMPLATE_ID "Enter Template ID"
#define BLYNK_DEVICE_NAME "Irrigation Control Vs"
#define BLYNK_AUTH_TOKEN "Enter Authorization Token"
#define BLYNK_FIRMWARE_VERSION "0.1.2"
// Comment this out to disable prints and save space
//#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
String phoneNumber = "+254790668724";
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Enter ssid";
char pass[] = "50010003";

BlynkTimer timer;
bool irrigation_started = false;
int irrigation_duration = 30;
int second_duration = 30;
int first_irrigation_start_hour = 10;
int first_irrigation_start_minute = 51;
int second_irrigation_start_hour = 18;
int second_irrigation_start_minute = 30;
int irrigationStopM = 0;
int irrigation2StopM = 0;
int irrigation2StopH = 0;

int pump = D6;
int valve = D7;
int TimeH;
int TimeM;
String DateDay;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void resetMCU()
{

#if defined(ESP8266) || defined(ESP32)
  ESP.restart();
#else
#error "MCU reset procedure not implemented"
#endif
  for (;;) {}
}
bool realTimeset = false;

BLYNK_WRITE(InternalPinDBG) {
  if (String(param.asStr()) == "reboot") {
    Serial.println("Rebooting...");

    // TODO: Perform any neccessary preparation here,
    // i.e. turn off peripherals, write state to EEPROM, etc.

    // NOTE: You may need to defer a reboot,
    // if device is in process of some critical operation.

    resetMCU();
  }
}

BLYNK_WRITE(V0)
{
  first_irrigation_start_hour = param.asInt();
}
BLYNK_WRITE(V1) {
  first_irrigation_start_minute = param.asInt();
}

BLYNK_WRITE(V2) {
  second_irrigation_start_hour = param.asInt();
}
BLYNK_WRITE(V3) {
  second_irrigation_start_minute = param.asInt();
  //  Serial.print("value set");
}
BLYNK_WRITE(V4) {
  irrigation_duration = param.asInt();
}
BLYNK_WRITE(V7) {
  second_duration = param.asInt();
}
// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);

  Blynk.syncVirtual(V4);
  Blynk.syncVirtual(V7);
  String first_time = String(first_irrigation_start_hour) + ":" + String(first_irrigation_start_minute);
  String second_time = String(second_irrigation_start_hour) + ":" + String(second_irrigation_start_minute);
  String irrigationTimes = first_time + " and " + second_time;
}


void myTimerEvent()
{
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.reconnect();
  }
  checkTime();
  sendMoisture();
  delay(50);
}
void timeKeeper() {
  if (WiFi.status() == WL_CONNECTED) {
    timeClient.update();
    TimeH = timeClient.getHours();
    TimeM = timeClient.getMinutes();
    DateDay = daysOfTheWeek[timeClient.getDay()];
    realTimeset = true;
    Serial.println("Time set from internet" + String(TimeH) + ":" + String(TimeM));
    Blynk.virtualWrite(V11, String(TimeH) + ":" + String(TimeM));
  }
  else {
    TimeM++;
    if (TimeM > 59) {
      TimeH++;
      TimeM = 0;
      if (TimeH > 23) {
        TimeH = 0;

      }
    }
  }

}
void setup ()
{ delay(100);
  Serial.begin(9600); // Set serial port speed
  WiFi.begin(ssid, pass);
  //WiFi.begin(ssid, pass);
  pinMode(wifi, OUTPUT);
  for (int i = 0; i < 30; i++)  {
    if (WiFi.status() == WL_CONNECTED) {
      digitalWrite(wifi, HIGH);
      Serial.print("Connected");
      Blynk.config(auth);  // in place of Blynk.begin(auth, ssid, pass);
      Blynk.connect(3333);  // timeout set to 10 seconds and then continue without Blynk
      timeClient.begin();
      timeClient.update();
      TimeH = timeClient.getHours();
      TimeM = timeClient.getMinutes();
      DateDay = daysOfTheWeek[timeClient.getDay()];
      realTimeset = true;
      while (Blynk.connect() == false) {
        // Wait until connected
      }
      Serial.println("Connected to Blynk server");
      break;
    }
    else {
      digitalWrite(wifi, HIGH);
      delay(500);
      digitalWrite(wifi, LOW);
      Serial.print(".");

    }
  }
  if (WiFi.status() != WL_CONNECTED) {
    //  do something if wifi not connected
  }

  //function to be called every second for keeping things running
  timer.setInterval(1000L, myTimerEvent);
  //  Timer function is called every minute
  timer.setInterval(60000L, timeKeeper);
  pinMode(pump, OUTPUT);
  pinMode(valve, OUTPUT);
  //turnoff the valves and pump
  digitalWrite(valve, HIGH);
  digitalWrite(pump, HIGH);
  String startTime = String(TimeH) + ":" + String(TimeM) + "  " + String(DateDay);
  Blynk.virtualWrite(V8, startTime);//send MCU last start time


}

void loop () {

  Blynk.run();//run blynk

  timer.run();//run the timer

}
void setIrrigationSchedule() {
  if (first_irrigation_start_minute + irrigation_duration > 59) {
    irrigationStopH = first_irrigation_start_hour + 1 ;
    irrigationStopM = (first_irrigation_start_minute + irrigation_duration) - 60 ;
  }
  else {
    irrigationStopH = first_irrigation_start_hour;
    irrigationStopM = first_irrigation_start_minute + irrigation_duration;
  }

  if (second_irrigation_start_minute + second_duration > 59) {
    irrigation2StopM = (second_irrigation_start_minute + second_duration) - 60;
    irrigation2StopH = second_irrigation_start_hour + 1;
  }
  else {
    irrigation2StopM = second_irrigation_start_minute + second_duration;
    irrigation2StopH = second_irrigation_start_hour;
  }

}
void checkTime() {
  setIrrigationSchedule();
  if (realTimeset) {
    if (TimeH == irrigationStopH && TimeM == irrigationStopM && irrigation_started == true) {
      stopIrrigation();
      Serial.println("Irrigation Stopped");
      irrigation_started = false;
      String Time = String(TimeH) + ":" + String(TimeM);
      Blynk.virtualWrite(V9, Time);
    }
    else if (TimeH == irrigation2StopH && TimeM == irrigation2StopM && irrigation_started == true) {
      stopIrrigation();
      Serial.println("Irrigation Stopped");
      irrigation_started = false;
      String Time = String(TimeH) + ":" + String(TimeM);
      Blynk.virtualWrite(V9, Time);
    }
    else if (TimeH == first_irrigation_start_hour && TimeM == first_irrigation_start_minute && !irrigation_started) {
      startIrrigation();
      Serial.println("irrigating till:" + String(irrigationStopH) + ":" + String(irrigationStopM));
      irrigation_started = true;
      String Time = String(first_irrigation_start_hour) + ":" + String(first_irrigation_start_minute);
      Blynk.virtualWrite(V5, Time);
      Blynk.virtualWrite(V6, DateDay);
      delay(50);

    }
    else if (TimeH == second_irrigation_start_hour && TimeM == second_irrigation_start_minute && !irrigation_started) {
      startIrrigation();
      Serial.println("irrigating till:" + String(irrigation2StopH) + ":" + String(irrigation2StopM));
      irrigation_started = true;

      String Time = String(second_irrigation_start_hour) + ":" + String(second_irrigation_start_minute);
      Blynk.virtualWrite(V5, Time);
      Blynk.virtualWrite(V6, DateDay);
    }
  }
}

void startIrrigation() {
  digitalWrite(pump, LOW);
  digitalWrite(valve, LOW);
}
void stopIrrigation() {
  digitalWrite(pump, HIGH);
  digitalWrite(valve, HIGH);
}

void sendMoisture() {
  int moisture = analogRead(A0);
  Serial.println(moisture);
  Blynk.virtualWrite(V10, moisture);
}

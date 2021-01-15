#include <Arduino.h>
//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include "FirebaseESP8266.h" // Install Firebase ESP8266 library

#include <ESP8266WiFi.h> // esp8266 library

#include <Adafruit_Sensor.h>
#include <DHT.h> // Install DHT11 Library and Adafruit Unified Sensor Library

#include <Servo.h>
//todo quang firebase
// #define FIREBASE_HOST "esp-blinkled-v1-default-rtdb.firebaseio.com"
// #define FIREBASE_AUTH "DcDFvZcUxMBPHl5Kjzk8niELfvLmmgAC8a58nlvD"
//todo thinh firebase
#define FIREBASE_HOST "thinh-2221d-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "tTQByiyqjz8f7YxsXsxVI15Wh3VnJXbtArD6Te9X"
//wifi password
#define WIFI_SSID "iPhone"       // input your home or public wifi name
#define WIFI_PASSWORD "77777777" //password of wifi ssid

#define DHTPIN 3 // Connect Data pin of DHT to RX
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
int led = D1; // Connect LED to D5
int led2 = 4;
const int trigPin = 12; //D4->2 D6->12
const int echoPin = 14; //D3->0 D5->14
const int servoPin = D7;
// defines variables
int duration;
long distance;
// init alculate distance function
int calculateDistance();
//calculate dht
void sensorUpdate();
//servo
Servo servo;
int previousDegree = 20;
int degree = 20;
int servoUpdate(int degree);
int servoResult;
unsigned long previousTimer = 0;
unsigned int interval = 100;

//todo Define FirebaseESP8266 data object
FirebaseData firebaseData;
FirebaseData ledData;
int led;
FirebaseJson json;

void setup()
{

  Serial.begin(115200);
  dht.begin();

  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input

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

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  servo.attach(servoPin); //D7
  servo.write(20);
}

void loop()
{
  unsigned long currentTimer = millis();
  sensorUpdate();

  if (Firebase.getString(ledData, "/FirebaseIOT/led"))
    if (Firebase.getFloat(ledData, "/FirebaseIOT/led"))
    //todo setString
    //todo setInt
    {
      Serial.println(ledData.stringData());
      if (ledData.stringData() == "ON")
      {
        digitalWrite(led, HIGH);
      }
      else if (ledData.stringData() == "OFF")
      {
        digitalWrite(led, LOW);
      }
    }

  if (Firebase.getString(ledData, "/FirebaseIOT/led2"))
  {
    Serial.println(ledData.stringData());
    if (ledData.stringData() == "ON")
    {
      digitalWrite(led2, HIGH);
    }
    else if (ledData.stringData() == "OFF")
    {
      digitalWrite(led2, LOW);
    }
  }
  //todo control servo
  if (Firebase.getString(ledData, "/FirebaseIOT/servo"))
  {
    Serial.println(ledData.stringData());
    if (ledData.stringData() == "LEFT")
    {
      if (currentTimer - previousTimer >= interval)
      {
        previousTimer = currentTimer;
        degree += 5;
        if (degree >= 160)
        {
          degree = 160;
        }
      }
    }
    else if (ledData.stringData() == "RIGHT")
    {
      if (currentTimer - previousTimer >= interval)
      {
        previousTimer = currentTimer;
        degree -= 5;
        if (degree <= 20)
        {
          degree = 20;
        }
      }
    }
    else if (ledData.stringData() == "STOP")
    {
      Serial.println("stop servo");
      degree = 70;
    }
    else
    {
      Serial.println("Failed to update servo firebase!");
    }
  }
  servoResult = servoUpdate(degree);
  distance = calculateDistance();
  Serial.print("Distance: ");
  Serial.println(distance);
  delay(100);
}

int calculateDistance()
{
  //todo calculate distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance = duration * 0.034 / 2;

  //todo sending data to Firebase
  if (Firebase.setFloat(firebaseData, "/FirebaseIOT/distance", distance))
  {
    Serial.println("PASSED");
    // Serial.println("PATH: " + firebaseData.dataPath());
    // Serial.println("TYPE: " + firebaseData.dataType());
    // Serial.println("ETag: " + firebaseData.ETag());
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
  }
  return distance;
}

int servoUpdate(int degree)
{
  servo.write(degree);
  Serial.print("servo turn ");
  if (degree > previousDegree)
  {
    Serial.print("right ");
  }
  else
  {
    Serial.print("left ");
  }
  previousDegree = degree;
  Serial.print(degree);
  Serial.println(" degree");
  return degree;
}

void sensorUpdate()
{
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("C  ,"));
  Serial.print(f);
  Serial.println(F("F  "));

  if (Firebase.setFloat(firebaseData, "/FirebaseIOT/temperature", t))
  {
    Serial.println("PASSED");
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
  }

  if (Firebase.setFloat(firebaseData, "/FirebaseIOT/humidity", h))
  {
    Serial.println("PASSED");
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
  }
}

/////////////////////////////////////////////////////////////
if (Firebase.getString(ledData, "/FirebaseIOT/led"))
{
  Serial.println(ledData.stringData());
  if (ledData.stringData() == "ON")
  {
    digitalWrite(led, HIGH);
  }
  else if (ledData.stringData() == "OFF")
  {
    digitalWrite(led, LOW);
  }
}
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Change the address to 0x3F if required

// Initialize the Software Serial for SIM800
SoftwareSerial sim800(6, 7); // rx, tx on board, tx rx on GSM
#include <dht.h>        // Include library
#define outPin 5        // Defines pin number to which the sensor is connected
dht DHT;                // Creates a DHT object
// Moisture Sensor pins
const int pumpRelayPin = 3;
const int fanRelayPin = 2;

#define sensorPower 4
#define sensorPin A0
#define soilWet 500   // Define max value we consider soil 'wet'
#define soilDry 800   // Define min value we consider soil 'dry'

int count = 0;
int count1=0;

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);
  pinMode(pumpRelayPin, OUTPUT);
  pinMode(fanRelayPin, OUTPUT);

  pinMode(sensorPower, OUTPUT);
  // Initially keep the sensor OFF
  digitalWrite(sensorPower, LOW);
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.print("...Greenhouse...");

  // Welcome message
  delay(2000);
  lcd.clear();
}

void loop() {
  int readData = DHT.read11(outPin);

  float t = DHT.temperature;        // Read temperature
  float h = DHT.humidity;           // Read humidity

  Serial.print("Temperature = ");
  Serial.print(t);
  Serial.print("°C | ");
  //  Serial.print((t * 9.0) / 5.0 + 32.0);  // Convert celsius to fahrenheit
  //  Serial.println("°F ");
  //  Serial.print("Humidity = ");
  //  Serial.print(h);
  //  Serial.println("% ");
  //  Serial.println("");

//  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(t);
  lcd.print("C");
  delay(1000);

  //  lcd.print("Hum: ");
  //  lcd.print(h);
  //  lcd.print("%");
  int moisture = readSensor();
  int moisturePercentage = map(moisture,soilWet, soilDry, 100, 0);
  Serial.print("Analog Output: ");
  Serial.println(moisture);
  Serial.println(moisturePercentage);
  lcd.setCursor(0, 0);
  lcd.print("Moisture: ");
  lcd.print(moisturePercentage);
  lcd.print("%");
  delay(1000);

  // Determine status of our soil
  if (moisturePercentage > 40) {
    Serial.println("Status: Soil is too wet");
    digitalWrite(pumpRelayPin, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Pump OFF      ");
    delay(1000);
    count1 = 0;
  }
  else {
    digitalWrite(pumpRelayPin, HIGH); // Turn on the pump
    Serial.println("Status: Soil is too dry - time to water!");
    lcd.setCursor(0, 1);
    lcd.print("Pump ON       ");
    delay(1000);
    if (count1 <2){
        sendSMS("Water pump activated due to low soil moisture.");
        count1 ++;
        Serial.println("Calling");
    
  }
  }
  if (t >= 22) { // Threshold for high temperature
    digitalWrite(fanRelayPin, HIGH); // Turn on the fan
     if (count <2){
    sendSMS("Fan activated due to high temperature.");
    count ++;
    }
    lcd.setCursor(0, 1);
    lcd.print("Fan ON        ");
    delay(1000);
  } else {
    digitalWrite(fanRelayPin, LOW); // Turn off the fan
    lcd.setCursor(0, 1);
    lcd.print("Fan OFF       ");
    delay(1000);
    count =0;
  }

  // Delay before the next reading
//  delay(2000);

//  delay(1000);  // Take a reading every second for testing
  // Normally you should take reading perhaps once or twice a day
  Serial.println();
}
int readSensor() {
  digitalWrite(sensorPower, HIGH);  // Turn the sensor ON
  delay(10);              // Allow power to settle
  int val = analogRead(sensorPin);  // Read the analog value form sensor
  digitalWrite(sensorPower, LOW);   // Turn the sensor OFF
  return val;             // Return analog moisture value
}
void sendSMS(String message) {
  sim800.println("AT+CMGF=1");    // Set the SIM800 to SMS mode
  delay(1000);
  sim800.println("AT+CMGS=\"+2347025478297\""); // Replace with the recipient's phone number
  delay(1000);
  sim800.print(message);
  delay(1000);
  sim800.write(26); // ASCII code for CTRL+Z to send the SMS
  delay(1000);
  //  sim800.println("ATD+2349155160253;");
  //   sim800.println("ATD+2348108265487;");
}

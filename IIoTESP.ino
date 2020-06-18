#include <ESP8266WiFi.h>                    // Including the ESP8266 WiFi library
#include "DHT.h"                            // Include DHT sensor library header file

#define DHTTYPE DHT11                       // Define sensot type - DHT 11

const char* ssid = "Maharshi";              // Network Configuration Details
const char* password = "268Maharshi315";
char server[] = "192.168.1.45";             // Server Configuration Details
WiFiClient client;                          // = server.available();

const int DHTPin = 5;                       // DHT Sensor configuration
DHT dht(DHTPin, DHTTYPE);

// IO Configuration of Hardware
// Inputs: Active High:   GPIO13, GPIO12, GPIO14, GPIO0, GPIO4, GPIO5, GPIO9, GPIO10
// Outputs: Active Low:   GPIO6, GPIO7, GPIO11, GPIO8

int button0 = 4;// D2(gpio4)                // Initialize Inputs
int button1 = 14; // D5(gpio14)
//int in1 = 13; // DXX(gpio13)
//int in2 = 12; // D6(gpio12)
//int in3 = 14; // D5(gpio14)
//int in4 = 0; // DXX(gpio0)
//int in5 = 4; // D2(gpio4)
//int in6 = 5; // DXX(gpio5)
//int in7 = 9; // DXX(gpio9)
//int in8 = 10; // DXX(gpio10)

//int out1 = 6; // DXX(gpio6)
//int out2 = 7; // DXX(gpio7)
//int out3 = 11; // DXX(gpio11)
//int out4 = 8; // DXX(gpio8)

// ================ Initialize constants ==================================================
//
int updateDelay = 15000;                    // Time delay for updating values on server database. In msec.
int inputConfiguration[]={4,14};            // Define the hardware pins arranging from Input 1 to Input 8
int outputConfiguration[]={};               // Define the hardware pins arranging from Output 1 to Output 4
int softDebounceDelay = 150;                // Software debounce for input. in msec.
int waitCountValue = 500000;                // Server update delay count
int waitCountForDHTSend = waitCountValue;
int inputState[2]={};                       // Current input state array
int inputLastState[2]={};                   // Previous input state array
int inputCounter[2]={};                     // Input signal counter
int noOfInputs = 2;                         // No. of inputs used
float humidityData;
float temperatureData;
String dataString;                          // Records message string to be sent to server

// ================ Functions ============================================================
//
void initIOs(){                             // Initialize IO pins in input / output mode
  for (int i = 0; i < noOfInputs; i++) {
    pinMode(inputConfiguration[i], INPUT);
  }
}

void initWiFi(){                            // Initialize and connect to Wi-Fi
  Serial.print("\nConnecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {   // Wait until Wifi is connected
    delay(500);
    Serial.print(".");  
  }
  Serial.print("\nWiFi connected - ");
  Serial.print("ESP IP: ");
  Serial.println(WiFi.localIP());           // Print IP assigned to ESP device
}

void counterCheck() {                       // Update input sensing counters
  for (int i = 0; i < noOfInputs; i++) {
    inputState[i] = digitalRead(inputConfiguration[i]);
    if (inputState[i] != inputLastState[i]) {
      if (inputState[i] == HIGH) {
         inputCounter[i]++;
         Serial.print("Input ");
         Serial.print(i);
         Serial.print(" : ");
         Serial.println(inputCounter[i]);
         delay(softDebounceDelay);
      }
      inputLastState[i] = inputState[i];  
    }
  }
}

String generateString(){                    // Generate message string to be sent to server. Data order to be defined as per the xml file used by server.
  dataString = "GET /testcode/dht.php?temperature=";  
  dataString.concat(temperatureData);
  dataString.concat("&humidity=");
  dataString.concat(humidityData);
  for (int i = 0; i < noOfInputs; i++){
    dataString.concat("&input");
    dataString.concat(i+1);
    dataString.concat("=");
    dataString.concat(inputCounter[i]);
  }
  dataString.concat(" HTTP/1.1");           // Space before HTTP/1.1
  return dataString;
}

void sendToServerDB(String dataToSend){    // Sending data to server using HTTP request
  if (client.connect(server,80)){
    Serial.println("Connected. Sending Update...");
    client.print(dataToSend);              // Make an HTTP request
    client.println();
    client.println("Host: 192.168.1.80");
    client.println("Connection: close");
    client.println();   
  } else {
    //if you didn't get a connection to server
    Serial.println("Connection Failed");
  }
}

// ================ Setup function ======================================================
//
void setup() {                            // Setup system
  Serial.begin(115200);                   // Initialize serial terminal @ 115200 bitrate
  dht.begin();                            // Initialize DHT11 Sensor
  initIOs();
  initWiFi();  
}

// ================ Looping function ====================================================
//
void loop() {                             // Run until stopped
  counterCheck();
  if (waitCountForDHTSend == 0){
    humidityData = dht.readHumidity();
    temperatureData = dht.readTemperature();
    sendToServerDB(generateString());  
    waitCountForDHTSend = waitCountValue;
  } else {
    waitCountForDHTSend--;
  }  
}

// ================ End =================================================================

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h> // DHT22 Library

// ESP8266 Web Server initialization
ESP8266WebServer server(80);

// ------------------ Settings ------------------
// Ei du'ti apnar nijer home/office Wi-Fi details diye bodlan
const char* ssid = "A H M Habib";  
const char* password = "12345678"; 

// Pin Definitions
#define DHTPIN D2           // DHT22 Data pin connected to D2 (GPIO4)
#define DHTTYPE DHT22       // Sensor type is DHT22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);   // Initialize DHT sensor

int relayPin = D1;          // Relay control pin connected to D1 (GPIO5)

// ------------------ Control Variables ------------------
bool autoMode = false;      // Is Auto Mode ON?
int thresholdValue = 65;    // Target Humidity (%) - Default 65%
int HYSTERESIS_BAND = 5;    // Hysteresis Band (%) to prevent relay chattering (65-5 = 60%)
float humidityValue = 0.0;  // Current humidity reading
bool relayState = false;    // Current state of the relay (true = ON, false = OFF)

// ------------------ HTML UI (User Interface) ------------------ 
String htmlPage() {
    // ... (HTML, CSS, JavaScript same as before, but added Threshold display) ...
    String page = R"=====(
    <html>
    <head>
      <title>Smart Humidifier Dashboard</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { background:#0d0d0d; color:white; font-family:Arial; text-align:center; }
        .card { background:#1a1a1a; width:90%; max-width:400px; margin:20px auto; padding:20px; border-radius:12px; box-shadow: 0 4px 8px rgba(0,0,0,0.5); }
        button { padding:12px 25px; border:none; border-radius:10px; margin:10px 5px; font-size:16px; cursor:pointer; transition: background 0.3s; }
        .on{background:#00cc66;color:white;}
        .off{background:#cc0000;color:white;}
        .stat{color:#ffcc00; font-size:24px;}
        input{padding:10px;width:70%;border-radius:8px;border:none;margin-top:10px;text-align:center;}
        h1{font-size:3.5em;}
      </style>
      <script>
        setInterval(() => {
            fetch("/update").then(r => r.json()).then(data => {
                document.getElementById("hum").innerHTML = data.humidity;
                document.getElementById("relay").innerHTML = data.relay;
                document.getElementById("auto").innerHTML = data.auto;
                document.getElementById("thresh").innerHTML = data.threshold + "%";
            });
        }, 1500); // Update every 1.5 seconds
      </script>
    </head>
    <body>
      <div class="card">
        <h2>💧 Smart Humidifier Control</h2>
        <p>Target: <span id="thresh" class="stat">--</span></p>
        <h1 id="hum">--</h1>

        <h3>Relay (Humidifier): <span id="relay" class="stat">--</span></h3>
        <button class="on" onclick="location.href='/on'">MANUAL ON</button>
        <button class="off" onclick="location.href='/off'">MANUAL OFF</button>

        <hr style="border-color:#333;">

        <h3>Auto Mode: <span id="auto" class="stat">--</span></h3>
        <button onclick="location.href='/auto_on'" class="on">AUTO ON</button>
        <button onclick="location.href='/auto_off'" class="off">AUTO OFF</button>

        <form action="/set" method="GET">
            <input type="number" name="th" placeholder="Set New Threshold (%)" required/>
            <button class="on" type="submit" style="width:20%">SET</button>
        </form>
      </div>
    </body>
    </html>
    )=====";
    return page;
}

// ------------------ Core Logic Function ------------------ 

void handleUpdate(){
    // Read humidity from DHT22 sensor
    float h = dht.readHumidity();

    // Check if the reading was successful
    if (isnan(h)) {
        Serial.println("DHT Read Error! Using last known value.");
    } else {
        humidityValue = h; // Update with float value for better precision
    }
    
    // --- Hysteresis-based Auto Control Logic ---
    if (autoMode) {
        // 1. Turn ON if humidity drops significantly below the threshold
        if (humidityValue < (thresholdValue - HYSTERESIS_BAND)) {
            relayState = true;
            digitalWrite(relayPin, LOW); // Active-LOW: ON
        } 
        // 2. Turn OFF if humidity reaches the target threshold
        else if (humidityValue >= thresholdValue) {
            relayState = false;
            digitalWrite(relayPin, HIGH); // Active-HIGH: OFF
        }
        // Note: Between (Threshold - Hysteresis) and Threshold, the relay state remains unchanged.
    }

    // Prepare JSON data for the web dashboard refresh
    String json = "{\"humidity\":\"" + String(humidityValue, 1) + 
                  "%\", \"relay\":\"" + (relayState ? "ON" : "OFF") +
                  "\", \"auto\":\"" + (autoMode ? "ON" : "OFF") + 
                  "\", \"threshold\":\"" + String(thresholdValue) + "\"}";

    server.send(200, "application/json", json);
}


// ------------------ Setup & Wi-Fi Configuration ------------------ 

void setup() {
    Serial.begin(115200);
    delay(10);

    // Initialize Relay Pin (Active-LOW logic: HIGH = OFF)
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, HIGH);

    // Initialize DHT sensor
    dht.begin();

    // Start Wi-Fi Connection (STA Mode)
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    // Wait for connection
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 30) {
        delay(500);
        Serial.print(".");
        attempt++;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nFailed to connect to WiFi! Starting in AP Mode for Debug.");
        // Fallback to AP Mode if STA fails (Optional, but robust)
        WiFi.softAP("HUMIDIFIER_SETUP", "12345678");
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("\nWiFi Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    }


    // ------------------ Server Endpoints ------------------

    server.on("/", [](){
        server.send(200, "text/html", htmlPage());
    });

    server.on("/update", handleUpdate);

    // Manual ON/OFF
    server.on("/on", [](){
        autoMode = false; // Disable auto mode for manual override
        relayState = true;
        digitalWrite(relayPin, LOW); 
        server.sendHeader("Location", "/");
        server.send(302, "text/plain", "");
    });

    server.on("/off", [](){
        autoMode = false; // Disable auto mode for manual override
        relayState = false;
        digitalWrite(relayPin, HIGH);
        server.sendHeader("Location", "/");
        server.send(302, "text/plain", "");
    });

    // Auto Mode Toggle
    server.on("/auto_on", [](){
        autoMode = true;
        server.sendHeader("Location", "/");
        server.send(302, "text/plain", "");
    });

    server.on("/auto_off", [](){
        autoMode = false;
        server.sendHeader("Location", "/");
        server.send(302, "text/plain", "");
    });

    // Set Threshold
    server.on("/set", [](){
        if(server.hasArg("th")){
             thresholdValue = server.arg("th").toInt();
             if (thresholdValue < HYSTERESIS_BAND + 5) {
                 thresholdValue = HYSTERESIS_BAND + 5; // Minimum threshold limit
             }
        }
        server.sendHeader("Location", "/");
        server.send(302, "text/plain", "");
    });

    server.begin();
}

// ------------------ Main Loop ------------------ 
void loop() {
    server.handleClient();
    
    // Optional: Re-check DHT reading frequently even when not requested by the web client
    // To ensure the global humidityValue is up-to-date for the Auto Mode logic in handleUpdate()
    // handleUpdate() already handles the logic, but we can separate the reading part if needed.
}

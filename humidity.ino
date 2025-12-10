#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

// ESP8266 Web Server initialization
ESP8266WebServer server(80);

// ------------------ SETTINGS ------------------
const char* ssid = "YOUR_HOME_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// Pin Definitions
#define DHTPIN D2           // DHT22 Data pin connected to D2 (GPIO4)
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

int relayPin = D1;          // Relay control pin connected to D1 (GPIO5)

// ------------------ CONTROL & SAFETY VARIABLES ------------------
bool autoMode = false;
int thresholdValue = 65;
int HYSTERESIS_BAND = 5;

// Sensor Readings & Safety
float humidityValue = 0.0;
float temperatureValue = 0.0;
bool relayState = false;
int dhtErrorCount = 0;
const int MAX_DHT_ERRORS = 5; 
bool isSensorReady = false; // NEW: Flag to indicate first successful read

// ------------------ MIND-BLOWING HTML UI (UNCHANGED) ------------------ 
String htmlPage() {
    String page = R"=====(
    <html>
    <head>
      <title>✨ Smart Climate Hub ✨</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        @import url('https://fonts.googleapis.com/css2?family=Roboto:wght@300;700&display=swap');
        
        body { background:#0a0a0a; color:#f0f0f0; font-family: 'Roboto', sans-serif; text-align:center; margin: 0; padding: 0;}
        .container { display: flex; flex-direction: column; align-items: center; justify-content: center; min-height: 100vh; }
        .card { background:#1c1c1c; width:90%; max-width:400px; padding:30px; border-radius:20px; box-shadow: 0 10px 30px rgba(0, 255, 255, 0.15); border: 1px solid #333; margin-top: 10px;}
        
        h2 { color: #00ffff; font-weight: 300; margin-bottom: 5px; }
        h1 { font-size: 5em; margin: 0; font-weight: 700; color: #f0f0f0; }
        
        .data-grid { display: flex; justify-content: space-around; padding: 15px 0; margin-bottom: 15px; border-bottom: 1px solid #333; }
        .data-item { font-size: 0.9em; }
        .data-value { font-size: 1.5em; font-weight: 700; color: #ff9900; }
        
        .status-box { background:#333; border-radius: 8px; padding: 8px; margin: 10px 0; font-weight: 700; transition: background 0.5s; }
        
        button { padding:12px 25px; border:none; border-radius:8px; margin:8px 5px; font-size:16px; font-weight: bold; cursor:pointer; transition: background 0.3s, transform 0.1s; width: 45%; box-sizing: border-box;}
        button:active { transform: scale(0.98); }
        
        .on{background:#00e676;color:#1c1c1c;}
        .off{background:#ff5252;color:#1c1c1c;}
        
        .input-group { display: flex; justify-content: space-between; margin-top: 20px; }
        input{padding:10px;border-radius:8px;border:none;flex-grow: 1; margin-right: 10px; background:#2c2c2c; color:white; text-align: center;}
      </style>
      <script>
        function updateData() {
            fetch("/update").then(r => r.json()).then(data => {
                const humElement = document.getElementById("hum");
                const relayBox = document.getElementById("relay-box");
                const autoBox = document.getElementById("auto-box");
                const errorBox = document.getElementById("error-box");

                document.getElementById("temp-val").innerHTML = data.temperature;
                document.getElementById("thresh-val").innerHTML = data.threshold + "%";
                humElement.innerHTML = data.humidity;

                // Relay Status Update
                relayBox.innerHTML = data.relay === "ON" ? "HUMIDIFYING ON 💧" : "IDLE OFF ⏸️";
                relayBox.style.backgroundColor = data.relay === "ON" ? "#00e676" : "#ff5252";
                relayBox.style.color = "#1c1c1c";

                // Auto Mode Status
                autoBox.innerHTML = data.auto === "ON" ? "AUTO MODE ACTIVE 🤖" : "MANUAL CONTROL ✋";
                autoBox.style.backgroundColor = data.auto === "ON" ? "#00aaff" : "#999";
                autoBox.style.color = "#1c1c1c";

                // Error Status
                if (data.error_status.includes("CRITICAL")) {
                    errorBox.innerHTML = "SENSOR FAILURE! 🔴";
                    errorBox.style.backgroundColor = "#ff0000";
                    errorBox.style.color = "white";
                } else if (data.error_status.includes("READY")) { // NEW: Sensor Ready state
                    errorBox.innerHTML = "SENSOR INITIALIZING...";
                    errorBox.style.backgroundColor = "#ffc107"; // Yellow
                    errorBox.style.color = "black";
                } else if (!data.error_status.includes("OK")) {
                    errorBox.innerHTML = data.error_status;
                    errorBox.style.backgroundColor = "#ff9900";
                    errorBox.style.color = "black";
                } else {
                    errorBox.innerHTML = "SYSTEM OK ✅";
                    errorBox.style.backgroundColor = "#1c1c1c";
                    errorBox.style.color = "#f0f0f0";
                }
            });
        }
        setInterval(updateData, 1500); // Update every 1.5 seconds
        window.onload = updateData;
      </script>
    </head>
    <body>
      <div class="container">
        <div class="card">
          <h2>SMART CLIMATE HUB</h2>
          
          <div class="data-grid">
            <div class="data-item">Target<br><span id="thresh-val" class="data-value">--</span></div>
            <div class="data-item">Temp<br><span id="temp-val" class="data-value" style="color:#00ffff;">--</span></div>
          </div>
          
          <h1><span id="hum">--</span>%</h1>
          
          <div id="error-box" class="status-box">SYSTEM OK ✅</div>

          <div id="relay-box" class="status-box" style="background:#ff5252;">IDLE OFF ⏸️</div>
          <div style="display: flex; justify-content: space-between;">
            <button class="on" onclick="location.href='/on'">MANUAL ON</button>
            <button class="off" onclick="location.href='/off'">MANUAL OFF</button>
          </div>

          <div id="auto-box" class="status-box" style="background:#999;">MANUAL CONTROL ✋</div>
          <div style="display: flex; justify-content: space-between;">
            <button class="on" onclick="location.href='/auto_on'">AUTO ON</button>
            <button class="off" onclick="location.href='/auto_off'">AUTO OFF</button>
          </div>

          <form action="/set" method="GET" class="input-group">
              <input type="number" name="th" placeholder="Set Threshold (%) (e.g. 60)" required/>
              <button class="on" type="submit" style="width: 25%;">SET</button>
          </form>
        </div>
      </div>
    </body>
    </html>
    )=====";
    return page;
}

// ------------------ Core Logic Function (Bug Fixes Applied) ------------------ 
void handleUpdate(){
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    String errorStatus = "OK";

    if (isnan(h) || isnan(t)) {
        if (isSensorReady == false) {
             errorStatus = "SENSOR READY..."; // New: Indicate initial startup phase
        }
        dhtErrorCount++;
        if (isSensorReady) { // Only count errors after a successful read
            errorStatus = "DHT Error (" + String(dhtErrorCount) + "/" + String(MAX_DHT_ERRORS) + ")";
        }
        
        // --- SMART SAFEGUARD LOGIC (If sensor fails permanently) ---
        if (dhtErrorCount >= MAX_DHT_ERRORS && isSensorReady) {
            errorStatus = "CRITICAL";
            if (autoMode) {
                autoMode = false;
                relayState = false;
                digitalWrite(relayPin, HIGH); // FORCED OFF
            }
        }
    } else {
        humidityValue = h;
        temperatureValue = t;
        dhtErrorCount = 0; // Reset error count
        isSensorReady = true; // Sensor is successfully reading data
    }
    
    // --- Hysteresis-based Auto Control (Bug Fix: Only run if sensor is ready) ---
    if (autoMode && isSensorReady && dhtErrorCount < MAX_DHT_ERRORS) { 
        
        // Turn ON condition: If currently OFF AND Humidity is low enough
        if (relayState == false && humidityValue < (thresholdValue - HYSTERESIS_BAND)) {
            relayState = true;
            digitalWrite(relayPin, LOW); 
        } 
        // Turn OFF condition: If currently ON AND Humidity is high enough
        else if (relayState == true && humidityValue >= thresholdValue) {
            relayState = false;
            digitalWrite(relayPin, HIGH);
        }
    }

    // Prepare JSON data
    String json = "{\"humidity\":\"" + String(humidityValue, 1) + "%\", " +
                  "\"temperature\":\"" + String(temperatureValue, 1) + "°C\", " +
                  "\"relay\":\"" + (relayState ? "ON" : "OFF") + "\", " +
                  "\"auto\":\"" + (autoMode ? "ON" : "OFF") + "\", " +
                  "\"threshold\":\"" + String(thresholdValue) + "\", " +
                  "\"error_status\":\"" + errorStatus + "\"}";

    server.send(200, "application/json", json);
}

// ------------------ Setup & Loop (UNCHANGED) ------------------ 
void setup() {
    Serial.begin(115200); delay(10);
    pinMode(relayPin, OUTPUT); digitalWrite(relayPin, HIGH);
    dht.begin();
    
    // Wi-Fi Connection (STA Mode)
    Serial.print("Connecting to WiFi: "); Serial.println(ssid);
    WiFi.begin(ssid, password);
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 30) { delay(500); Serial.print("."); attempt++; }
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nFailed to connect to WiFi! Starting in AP Mode for Debug.");
        WiFi.softAP("SMART_HUB", "12345678");
        Serial.print("AP IP address: "); Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("\nWiFi Connected!");
        Serial.print("IP Address: "); Serial.println(WiFi.localIP());
    }

    // Server Endpoints
    server.on("/", [](){ server.send(200, "text/html", htmlPage()); });
    server.on("/update", handleUpdate);
    server.on("/on", [](){ autoMode = false; relayState = true; digitalWrite(relayPin, LOW); server.sendHeader("Location", "/"); server.send(302, "text/plain", ""); });
    server.on("/off", [](){ autoMode = false; relayState = false; digitalWrite(relayPin, HIGH); server.sendHeader("Location", "/"); server.send(302, "text/plain", ""); });
    server.on("/auto_on", [](){ autoMode = true; server.sendHeader("Location", "/"); server.send(302, "text/plain", ""); });
    server.on("/auto_off", [](){ autoMode = false; server.sendHeader("Location", "/"); server.send(302, "text/plain", ""); });
    server.on("/set", [](){
        if(server.hasArg("th")){ thresholdValue = server.arg("th").toInt(); if (thresholdValue < HYSTERESIS_BAND + 5) { thresholdValue = HYSTERESIS_BAND + 5; }}
        server.sendHeader("Location", "/"); server.send(302, "text/plain", "");
    });
    server.begin();
}

void loop() {
    server.handleClient();
}

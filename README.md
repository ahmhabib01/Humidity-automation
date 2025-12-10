

---

# 🚀 **Humidity Automation** 💧

## 🎯 **Project Goal**

To build an extremely reliable, Wi-Fi-controlled, and aesthetically pleasing **Automated Humidifier Controller** using the ESP8266, eliminating the common problems of relay chattering and sensor failure through smart safety logic.

---

## ✨ **Edge**

| Feature | Impact | Status |
| :--- | :--- | :--- |
| 🌐 **Offline Dominance** | Full dashboard control without any active internet connection (zero dependency on Google APIs). | ✅ **DONE** |
| 🛡️ **Fail-Safe Logic** | Critical sensor errors automatically force the humidifier OFF, preventing accidental flooding or burnout. | ✅ **DONE** |
| 🧠 **Intelligent Stability** | Uses **Hysteresis** to stabilize switching behavior, maximizing relay life. | ✅ **DONE** |
| 🎨 **Hyper-Responsive UI** | Fast, dark-mode, mobile-friendly dashboard served instantly from the ESP8266's memory. | ✅ **DONE** |

---

## ⚙️ **Core Technology**

| Component | Role | Pins Used (NodeMCU) |
| :--- | :--- | :--- |
| **Microcontroller** | ESP8266 | Wi-Fi Station/AP Mode, Web Server |
| **Sensor** | DHT22 | **D2 (GPIO4)**: Humidity & Temp Data |
| **Actuator** | 1-Channel Relay | **D1 (GPIO5)**: Humidifier Control (Active-LOW) |

---

## 🌐 **Connectivity**

The device attempts to connect to your home Wi-Fi first (STA Mode). If credentials are not set or connection fails, it falls back to a self-created Access Point (AP).

| Mode | Network Name | Access IP | Access Requirement |
| :--- | :--- | :--- | :--- |
| **STA Mode** (Home Wi-Fi) | Your Router's SSID | `192.168.x.xxx` | Device must be on the same network. |
| **AP Mode** (Direct Link) | **`BDC_2.0`** (Pass: `ahmhabib01`) | **`192.168.4.1`** | **No Internet Required.** Connect directly to this network. |

---



## 🛠️ **Setup Guide**

### 1\. 🔌 Hardware Wiring

**Carefully connect the components as detailed below:**

| Component Pin | ESP8266 Pin (GPIO) | Notes |
| :--- | :--- | :--- |
| **DHT22 VCC** | **3.3V** | Power for the sensor. |
| **DHT22 GND** | **GND** | Ground connection. |
| **DHT22 Data** | **D2 (GPIO4)** | Sensor Data Input. |
| **Relay VCC** | **VIN (5V)** / **VCC** | Relay Coil Power Supply. |
| **Relay GND** | **GND** | Relay Ground. |
| **Relay IN** | **D1 (GPIO5)** | Control Signal from ESP. (Active-LOW Logic) |

> ⚠️ **Safety Note:** Connect the humidifier's power line to the relay's **Common (COM)** and **Normally Open (NO)** terminals. Never connect high voltage while the ESP8266 is powered on.

-----

### 2\. 💻 Software and Library Installation

1.  **Open Arduino IDE** and go to **File \> Preferences**.
2.  Add the following URL to "Additional Boards Manager URLs":
    ```
    http://arduino.esp8266.com/stable/package_esp8266com_index.json
    ```
3.  Go to **Tools \> Board \> Boards Manager...** and search for **`esp8266`**. Install the latest package.
4.  Go to **Sketch \> Include Library \> Manage Libraries...** and install the following:
      * **`DHT sensor library`** by Adafruit
      * **`Adafruit Unified Sensor`**

-----

### 3\. ⚙️ Code Configuration and Upload

1.   Download and open [humidity.ino](https://github.com/ahmhabib01/Humidity-automation/blob/main/humidity.ino) with Arduino IDE
2.  **Optional:** Update the Wi-Fi credentials for STA Mode (if using your home network):
    ```cpp
    const char* ssid = "YOUR_HOME_WIFI_NAME";
    const char* password = "YOUR_WIFI_PASSWORD";
    ```
3.  Select the correct board (**e.g., NodeMCU 1.0**) and **Port** under **Tools**.
4.  Click the **Upload** button (Right Arrow icon).
5.  Wait for the `Done uploading.` message.

-----

## 🖥️ **Accessing the Dashboard**

1.  Open the **Serial Monitor** (115200 Baud) and wait for the "IP Address" to be printed.
2.  Open a web browser (on a device connected to the relevant Wi-Fi network) and enter the IP address.

| Mode | Network Name | Access IP | Access Requirement |
| :--- | :--- | :--- | :--- |
| **STA Mode** (Home Wi-Fi) | Your Router's SSID | `192.168.x.xxx` (Dynamic IP) | Device must be on the same home network. |
| **AP Mode** (Direct Link) | **`BDC_2.0`** (Pass: `ahmhabib01`) | **`192.168.4.1`** (Static IP) | Connect directly to the `SMART_HUB` network. |

> 💡 **Tip:** If the dashboard doesn't load in AP Mode, ensure your phone's Mobile Data is temporarily turned **OFF**.

-----

## 🧠 **Core Logic: The Hysteresis Principle**

Humidity Automation uses a $5\%$ hysteresis band to maintain target humidity and prevent short-cycling.

  * **Target (e.g., 65%):** The ideal max humidity.
  * **Humidifier ON:** Only when Humidity $\le$ **$60\%$** ($\text{Target} - 5\%$).
  * **Humidifier OFF:** Only when Humidity $\ge$ **$65\%$** ($\text{Target}$).

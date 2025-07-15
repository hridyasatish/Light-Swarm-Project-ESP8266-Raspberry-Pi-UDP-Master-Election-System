# Light Swarm Project – Distributed Master Election System Using ESP8266 and Raspberry Pi  

Author: Hridya Satish  

---

##  Project Overview  

The Light Swarm project implements a distributed master election protocol using a swarm of ESP8266 microcontrollers and a Raspberry Pi controller. Each ESP8266 module senses ambient light using a photoresistor and periodically broadcasts its reading over Wi-Fi using UDP multicast.  

The Raspberry Pi monitors the swarm, identifies which device reports the highest light reading (the "master"), controls status LEDs for visualization, logs master transitions into CSV files, and generates real-time graphical plots using Matplotlib.  

This system is designed as part of the ECPS 216: Embedded Systems Networking course and demonstrates key embedded system networking concepts such as real-time UDP communication, multicast group management, master election algorithms, and embedded data visualization.

---

## System Architecture  

### ESP8266 Nodes  

- **Function:**  
  Each ESP8266 device reads a light sensor value, adjusts its external LED brightness through PWM, and broadcasts its status.  
- **Master Election Rule:**  
  The device with the highest light sensor value among all active nodes is elected as the master.  
- **Control Indicators:**  
  - External LED (PWM): Displays brightness proportional to light intensity.  
  - Onboard LED: Turns ON if the device is elected as master.  

### Raspberry Pi Controller  

- **Function:**  
  Listens for packets from ESP8266 nodes, identifies the master, controls GPIO-connected LEDs, logs events, and plots real-time graphs.  
- **Control Indicators:**  
  - Red, Yellow, Green LEDs: Indicate which ESP8266 is currently the master.  
  - White LED: Indicates swarm reset activity.  
- **User Input:**  
  - Push Button: Toggles monitoring on/off and sends reset commands to the swarm.  

---

## Communication Protocol  

- **Transport:** UDP Multicast  
- **Multicast Group:** 239.0.0.1  
- **Port:** 3000  

### Packet Format  

| Byte Index | Field             | Description                                  |
|------------|------------------|----------------------------------------------|
| 0          | Start Byte       | Fixed value `0xF0`                           |
| 1          | Packet Type      | `0x00` = Light Update, `0x01` = Reset       |
| 2          | Device ID        | Unique ID for each ESP8266 device            |
| 3          | Master State     | `1` = Master, `0` = Non-Master              |
| 5–6        | Light Sensor Data| 16-bit Integer                               |
| 13         | End Byte         | Fixed value `0x0F`                           |

---

## 🔧 Hardware Configuration  

### ESP8266 Pin Setup  

| Pin            | Purpose                          |
|----------------|----------------------------------|
| A0             | Photoresistor (Analog Input)     |
| D3 (GPIO0)     | External LED (PWM Brightness)    |
| LED_BUILTIN    | Master Indicator (Onboard LED)   |
| VCC/GND        | Power Supply (3.3V, Ground)      |

### Raspberry Pi GPIO Pin Mapping  

| GPIO Pin  | Component         | Purpose                          |
|-----------|-------------------|----------------------------------|
| GPIO24    | Red LED           | Master Indicator (ESP1)          |
| GPIO21    | Yellow LED        | Master Indicator (ESP2)          |
| GPIO20    | Green LED         | Master Indicator (ESP3)          |
| GPIO19    | White LED         | Swarm Reset Indicator            |
| GPIO17    | Push Button       | Start/Stop Monitoring + Reset    |

---

## System Schematics  

### ESP8266 Swarm Node Schematic  

![ESP8266 Swarm Schematic](images/esp8266_swarm_schematic.png)

### Raspberry Pi GPIO Wiring Diagram  

![Raspberry Pi Wiring Diagram](images/raspberry_pi_wiring.png)

---

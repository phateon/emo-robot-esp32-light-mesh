| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | -------- | -------- | -------- |

# ESP Now Mesh with Webserver and LED control

The system provides an esp now mesh network to control various LED strips through esp32 controllers. It setsup a webserver to control the whole system via http commands.

## LED

* main/led

Contains the code to design and run effects.

## ESP Now

* main/espnow

Client and server code for esp now network

## Web Server

* main/webserver

HTTP interface code.

## Utils

* main/utils

Utility code for buttons, 8bit math, etc.

# Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

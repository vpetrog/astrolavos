![Astrolavos Logo](assets/Astrolavos.svg)

A Festival Friend Finder based on [Heltec Wireless Tracker](https://heltec.org/project/wireless-tracker/)
Astrolavos (*Ancient Greek: ἀστρολάβος*) is an astronomical instrument dating to the ancient time and served as a chart for the stars.
You and your friends are unique stars in the festival sky, and Astrolavos will help you find each other (#cringe)


## 1. Introduction
The project aims to be an OS Festival Friend Finder. The tracker will show the direction and distance of the other paired trackers. The devices are connected to each other via LORA.

## 2. Tenets
- **Open Source** Free for anyone to use with their friend
- **Low Cost** There are commercial alternatives that cost north of 70 EUR. This project will try to achieve the same with less than half that price
- **Low Power** In Festivals there is rarely sufficient access to power plugs. **Astrolavos** should be able to operate at least 36 hours (~3 festival days) with a single charge
- **No Phone dependency** You are at a festival disconnect and enjoy the moment.

## 2. Bill of Materials
- [Heltec Wireless Tracker](https://heltec.org/project/wireless-tracker/)
- [GY-271 Magnetometer](https://www.az-delivery.de/en/products/gy-271-kompassmodul-kompass-magnet-sensor-fuer-arduino-und-raspberry-pi?variant=18912984432736)
- Any 3000mAh  LiPO 1S/1C with JST 1.25 Connector [like this one](https://www.amazon.de/-/en/dp/B0F18ST3K5?ref=ppx_yo2ov_dt_b_fed_asin_title)

## 3. Usage

The display has a health bar on bottom that show the battery level, the GNSS status and the status of the magnetometer (magnetic compass). The main display has has up to 6 lines each one showing the distance and direction to a paired device.

There are two modes of operation:

- **Active Mode**: When you are looking for your friends, the device will show the distance and direction to each of them. The display is on and doing some calculations on the background and as a result is consuming more power.

- **Isolation Mode**: When you don't care about your friends, and just want to dance. The device will keep exchanging data, as your friends might actually care about you. In this mode the display is off and the device is consuming less power. It is recommended to be on this mode when you are not actively looking for your friends. It can be activated bt pressing the small USR button on the top left corner of the device. The effect might take up to 2 seconds to take effect. In order to switch back to active mode, you need to press the button again.

- **Setup Mode**: The user can enter the setup up mode by pressing the button on the top left corner of the device, when the device is booting. Setup mode allow the user to calibrate the magnetometer. The calibration parameters are stored in the Non-Volatile Memory of the device, so they will be available even after a power cycle. The calibration is done by rotating the device in all directions, until the calibration is complete. The display will show the progress of the calibration and will indicate when it is done. After the setup is completed, the device will reboot.

In addition to this there is an ***I WANT TO MEET*** option which can be activated by pressing the button the bottom right corner of the device. This will send a message to all paired devices that you want to meet them. The message will be shown on their displays as well as a clear indication on you own display, that you are transmitting this message. To stop transmitting this message, you need to press the button again.


## 4. Software

### 4.1 Introduction
The software is distributed under MIT licence (see [License.md](License.md)). It is developed using the esp-idf framework on Platformio. There is no official support of the Heltec boards for esp-idf, yet here we are using it. This unorthodox approach is justified by our Low Power tenet. With Arduino we are very limited when it comes to power optimisations, whereas with RTOS, we hope to achieve a lower Power Cost.

### 4.2 Current State
The project is in very, very, very alpha and under heavy development. Expect functionality breaking changes, git history modifications and all the goodies that come with a greenfield project.

### 4.3 Status and Next Plan
------------------ 🚀 MVP 🚀 -------------------
- Dev Enviroment Setup and Board Definitions ✅
- Basic TFT Library Driver ✅
- GNSS Driver ✅
- LORA Driver 🚧
- Magnetometer Driver ✅
- Integration and Main App 🚧

---------------- 🎯 Stretch 🎯 -----------------
- *Stretch Goal* Power Optimisation ✅
- *Stretch Goal* Battery Monitoring ✅
- *Stretch Goal* Isolation Mode ✅
- *Stretch Goal* *I WANT TO MEET* mode of operation ✅
- *Stretch Goal* Magnetometer Calibration and Setup Mode ✅
- *Stretch Goal* Staleness Detections and Data Freshness 🎯

----------------- ✨ Future Plans ✨ ------------------
- Menu ✨
- Pairing without Code ✨
- Better Visuals, Integration with [lvgl](https://lvgl.io/)

### 4.4 Architecture
Each hardware component (GNSS module, Magnetometer, Battery Monitor, LoRa radio) is handled by its own dedicated task. These tasks operate independently and push their results into a shared application state that acts as the central source of truth. This decoupled design ensures that sensor readings and communication are cleanly isolated, enabling easier debugging, unit testing, and future extensions as well trying to keep our sanity with the intricacies of the sleep modes of the ESP32.

The Astrolavos Main task acts as the system’s brain: it reads the current heading, battery status, and both local and remote coordinates from the shared state to compute direction and distance to each peer device. It then updates the screen to reflect this information. LoRa communication is handled by two separate tasks: the transmitter periodically reads the device’s own coordinates and broadcasts them, while the receiver updates the positions of nearby devices as messages arrive. The diagram below illustrates the flow of data among these components.
```

      +------------------+     +---------------------+     +-------------------------+
      |    GNSS Task     |     | Magnetometer Task   |     | Battery Monitor Task    |
      |------------------|     |---------------------|     |-------------------------|
      | Get coordinates  |     | Get heading         |     | Read battery level      |
      | Update state     |     | Update state        |     | Update state            |
      +--------+---------+     +----------+----------+     +-----------+-------------+
               \                     |                             /
                \                    |                            /
                 \                   |                           /
                  \                  |                          /
                   \                 |                         /
                    \                |                        /
                     \               |                       /
                      v              v                      v
                            +------------------+
                            |   Shared State    |
                            +--------+----------+
                                     |
          +--------------------------+---------------------------+
          |                          |                           |
          v                          v                           v
  +---------------+         +----------------+         +-------------------+
  | LoRa TX Task  |         | Astrolavos Main|         |  LoRa RX Task     |
  |---------------|         |----------------|         |-------------------|
  | Read from     |         | Calculate      |         | Receive           |
  | state         |         | distances,     |         | coordinates       |
  | Broadcast     |         | headings       |         | Update state      |
  +---------------+         | Update display |         +-------------------+
                            +----------------+

```

### 5. Configuration
To use the device, you need to flash the firmware onto the Heltec Wireless Tracker.The user names and number of defines, are statically conffigure in the `lib/Astrolavos/AstrolavosDeviceConfig.cpp` file. This file is now comitted to the repo, but in the future it will be generated by a script and later by a GUI with no code editing required.

### 6. Contributing
get in touch with @vpetrog, contributions are more than welcomed.
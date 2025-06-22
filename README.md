![Astrolavos Logo](assets/Astrolavos.svg)

A Festival Friend Finder based on [Heltec Wireless Tracker](https://heltec.org/project/wireless-tracker/)

[![CI/CD Pipeline](https://github.com/vpetrog/astrolavos/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/vpetrog/astrolavos/actions/workflows/ci.yml) [![Hackaday Project](https://img.shields.io/badge/Hackaday-Project-blue)](https://hackaday.io/project/203354-astrolavos)

Astrolavos (*Ancient Greek: ἀστρολάβος*) is an astronomical instrument dating to the ancient time and served as a chart for the stars.
You and your friends are unique stars in the festival sky, and Astrolavos will help you find each other (#cringe)


## 1. Introduction
The project aims to be an Open SOURCE Festival Friend Finder. The tracker will show the direction and distance of the other paired trackers. The devices are connected to each other via LORA.

## 2. Tenets
- **Open Source** Free for anyone to use with their friend
- **Low Cost** There are commercial solutions that cost north of 190 EUR. This project doe not to overthrow them. They are professional they know what they are doing. This project is an Open source alternative solution to the problem of finding the location of someone without cellular connectivity. Finding your friends at festivals can be sometimes be difficult or othertimes... well let's say stressful. Having a low cost option, will allow more people to use it feel safe and enjoy the festival at its fullest.
- **Low Power** In Festivals there is rarely sufficient access to power plugs. **Astrolavos** should be able to operate at least 36 hours (~3 festival days) with a single charge.
- **No Phone dependency** You are at a festival disconnect and enjoy the moment.

## 3. Bill of Materials
- [Heltec Wireless Tracker](https://heltec.org/project/wireless-tracker/)
- [GY-271 Magnetometer](https://www.az-delivery.de/en/products/gy-271-kompassmodul-kompass-magnet-sensor-fuer-arduino-und-raspberry-pi?variant=18912984432736)
- Any 3000mAh  LiPO 1S/1C with JST 1.25 Connector [like this one](https://www.amazon.de/-/en/dp/B0F18ST3K5?ref=ppx_yo2ov_dt_b_fed_asin_title)

## 4. Usage

* For a detailed guide with photos check the [Hackaday Project Page](https://hackaday.io/project/203354-astrolavos)

The display has a health bar on bottom that show the battery level, the GNSS status and the status of the magnetometer (magnetic compass). The main display has has up to 6 lines each one showing the distance and direction to a paired device.

There are two modes of operation:

- **Active Mode**: When you are looking for your friends, the device will show the distance and direction to each of them. The display is on and doing some calculations on the background and as a result is consuming more power.

- **Isolation Mode**: When you don't care about your friends, and just want to dance. The device will keep exchanging data, as your friends might actually care about you. In this mode the display is off and the device is consuming less power. It is recommended to be on this mode when you are not actively looking for your friends. It can be activated by pressing the button  next to power switch. The effect might take up to 2 seconds to take effect. In order to switch back to active mode, you need to press the button again.

- **Setup Mode**: The user can enter the setup up mode by pressing the Isolation Mode Button, when the device is booting. Setup mode allow the user to calibrate the magnetometer. The calibration parameters are stored in the Non-Volatile Memory of the device, so they will be available even after a power cycle. The calibration is done by rotating the device in all directions, until the calibration is complete. The display will show the progress of the calibration and will indicate when it is done. After the setup is completed, the device will reboot.

In addition to this there is an ***I WANT TO MEET*** option which can be activated by pressing the button at the bottom right corner of the device. This will send a message to all paired devices that you want to meet them. The message will be shown on their displays as well as a clear indication on you own display, that you are transmitting this message. To stop transmitting this message, you need to press the button again.


## 5. Software

### 5.1 Introduction
The software is distributed under MIT licence (see [License.md](License.md)). It is developed using the esp-idf framework on Platformio. There is no official support of the Heltec boards for esp-idf, yet here we are using it. This unorthodox approach is justified by our Low Power tenet. With Arduino we are very limited when it comes to power optimisations, whereas with RTOS, we hope to achieve a lower Power Cost.

### 5.2 Current State
The project is in very, very, very alpha and under heavy development. Expect functionality breaking changes, git history modifications and all the goodies that come with a greenfield project.

### 5.3 Status and Next Milestones
------------------ 🚀 MVP 🚀 -------------------
- Dev Enviroment Setup and Board Definitions ✅
- Basic TFT Library Driver ✅
- GNSS Driver ✅
- LORA Driver ✅
- Magnetometer Driver ✅
- Integration and Main App ✅

---------------- 🎯 Stretch 🎯 -----------------
- *Stretch Goal* Power Optimisation ✅
- *Stretch Goal* Battery Monitoring ✅
- *Stretch Goal* Isolation Mode ✅
- *Stretch Goal* *I WANT TO MEET* mode of operation ✅
- *Stretch Goal* Magnetometer Calibration and Setup Mode ✅
- *Stretch Goal* Staleness Detections and Data Freshness 🎯

----------------- ✨ Future Plans ✨ ------------------

Check the [Future Plans on Issues](https://github.com/vpetrog/astrolavos/issues?q=is%3Aissue%20state%3Aopen%20label%3Aenhancement)

### 5.4 Architecture
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

### 6. Configuration
To use the device, you need to flash the firmware onto the Heltec Wireless Tracker.The user names and number of defines, are statically configure in the `lib/Astrolavos/AstrolavosDeviceConfig.cpp` file. This file is now committed to the repo, but in the future it will be generated by a script and later by a GUI with no code editing required.

If you want to debug/develop without other devices, add `	-DASTROLAVOS_MOCKUP_LORA_RECEIVER` in the `platformio.ini` file. This will allow you to run the device in a mockup mode, where it will generate random coordinates and headings for the other devices.

### 7. Contributing
get in touch with @vpetrog, contributions are more than welcomed. There is a very basic CI pipeline that builds the project using PIO, runs cppcheck and checks the formatting with clang-format.

At this point I owe an apology to all the contributors about the .clang-format template. It is probably one of the ugliest formatting templates you have ever seen. I was supposed to add the kernel style but something went wrong halfway.

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

## 3. Schematics
TBF

## 4. Software

### 4.1 Introduction
The software is distributed under MIT licence (see [License.md](License.md)). It is developed using the esp-idf framework on Platformio. There is no official support of the Heltec boards for esp-idf, yet here we are using it. This unorthodox approach is justified by our Low Power tenet. With Arduino we are very limited when it comes to power optimisations, whereas with RTOS, we hope to achieve a lower Power Cost.

### 4.2 Current State
The project is in very, very, very alpha and under heavy development. Expect functionality breaking changes, git history modifications and all the goodies that come with a greenfield project.

### 4.3 Status and Next Plan
- Dev Enviroment Setup and Board Definitions ✅
- Basic TFT Library Driver ✅
- GNSS Driver ✅
- LORA Driver 🚧
- Magnetometer Driver ✅
- Integration and Main App 🚧

------------------ 🚀 MVP 🚀 -------------------

- *Stretch Goal* Power Optimisation 🚧
- *Stretch Goal* Battery Monitoring ✅
- *Stretch Goal* Staleness Detections and Data Freshness 🎯
- *Stretch Goal* Isolation Mode 🎯

------------------  Future Plans  -------------------

- Menu ✨
- Pairing without Code ✨
- Better Visuals, Integration with [lvgl](https://lvgl.io/)

### 5. Contributing
get in touch with @vpetrog, contributions are more than welcomed.
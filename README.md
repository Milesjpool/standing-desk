# Standing Desk

A little [PlatformIO](https://platformio.org/), Arduino-based project, to let me control my Flexispot standing-desk via a REST API\*.

[![Grafana Badge](https://img.shields.io/badge/Grafana-grey?style=flat-square&logo=grafana)](https://grafana.aesop.casa/public-dashboards/4751c3c137974d77a0905190a4f2ccc0)

Required hardware:

- Flexispot [E7 pro](https://staticprod.sys.flexispot.co.uk/dev/trantor/attachments/E7-PRO-UK%20.pdf) (Or similar?)
- [Adafruit Huzzah32](https://learn.adafruit.com/adafruit-huzzah32-esp32-feather)
- RJ-45 cable + jack/breakout board.

## Other resources

Optional software:

- [Home assistant](https://www.home-assistant.io/)
- CURL or other REST client software.

Inspiration drawn from:

- Hardware: https://www.devmire.com/2021/10/03/reverse-engineering-a-standing-desk-for-fun-and-profit/
- Pinout: https://www.mikrocontroller.net/topic/493524
- Height-decoding: https://github.com/grssmnn/ha-flexispot-standing-desk/tree/master?tab=readme-ov-file
- Commands: https://github.com/iMicknl/LoctekMotion_IoT

## Functionality

### Current

- `GET /` returns welcome message and local IP address.
- `GET /height` returns last height reading, and reading 'staleness'.
- `POST /height/preset/{preset-id}` moves desk to preset height.
- `DELETE /height` stops any movement in progress.

### Todo

- Improve height query responsiveness.
- Move desk to specific height (`PUT /height/{height_mm}`?)
- Save presets from API?
- Continuous UP/DOWN button presses? Web-sockets?
- Voice controls?

---

\*or as close to REST as I can get with [the package](https://github.com/espressif/arduino-esp32/tree/b05f18dad55609ae2a569be81c7535021b880cf3/libraries/WebServer) available - not sure how to decode a request body yet...

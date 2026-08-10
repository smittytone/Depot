# Depot 1.3.0

Multi-bus clients for macOS and Linux, and bus-host adaptor board firmware for the RP2040 or RP2350.

It is used as the basis for I&sup2;C client apps for generic I&sup2;C usage, and to operate Holtek HT16K33-controlled matrix and segment LED displays, and 1-Wire apps for generic 1-Wire usage and to use the Analog Devices DS18B20 sensors.

![Driving a pair of LTP305 LED matrices as a CPU activity indicator](images/matrix_002.webp)

The client code is largely written in C, and will compile and run on both macOS and Linux. The exceptions are the DS18B20 command-line utility and DS18B20 GUI app, both of which are written in Swift — and the latter is built for the macOS GUI.

Why Depot? Because it’s a place in which you’ll find lots of buses.

**Note** This repo supersedes and extends [cli2c](https://github.com/smittytone/cli2c), the previous repo for this code, now archived. However, Depot maintains the older repo’s versioning.

## Hardware Requirements

### Bus Host Board

The bus host board is a [Raspberry Pi Pico](https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html), [Raspberry Pi Pico 2](https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html#pico2), [Adafruit QTPy RP2040](https://www.adafruit.com/product/4900), [Adafruit QT2040 Trinkey](https://www.adafruit.com/product/5056), [Adafruit Feather RP2040](https://www.adafruit.com/product/48840), [SparkFun ProMicro RP2040](https://www.sparkfun.com/products/18288), [Pimoroni Tiny 2040](https://shop.pimoroni.com/products/tiny-2040?variant=39560012234835) or [Arduino Nano RP2040 Connect](https://store.arduino.cc/products/arduino-nano-rp2040-connect-with-headers).

It runs the included firmware and connects to a host computer via USB.

![Driving a segment display with the Arduino Nano](images/nano_seg.webp)

The build system will use the standard Pico SDK environment variable `PICO_BOARD`, if set, to configure compilation for a particular supported board. If the variable is undefined, or set to a board not yet supported by Depot, the firmware will be built for all supported boards.

| Board | `PICO_BOARD` Value |
| :-- | :-- |
| Raspberry Pi Pico | `pico` |
| Raspberry Pi Pico 2 | `pico2` |
| Adafruit Feather RP2040 | `adafruit_feather_rp2040` |
| Adafruit QTPy RP2040 | `adafruit_qtpy_rp2040` |
| Adafruit Trinkey RP2040 | `adafruit_trinkey_qt2040` |
| Arduino Nano RP2040 Connect | `arduino_nano_rp2040_connect` |
| Sparkfun ProMicro | `sparkfun_promicro` |
| Pimoroni Tiny 2040 | `pimoroni_tiny2040` |

* There’s more information [in this blog post](https://blog.smittytone.net/2023/03/16/meet-depot-an-rp2040-based-multi-bus-adaptor-for-macs-and-linux-pcs/).

## Firmware and Client App Build Pre-requisites

### macOS

```shell
xcode-select --install
brew install cmake ninja libusb picotool
brew install --cask gcc-arm-embedded
git clone https://github.com/raspberrypi/pico-sdk.git
git -C pico-sdk submodule update --init
export PICO_SDK_PATH=/path/to/pico-sdk
export PICOTOOL_FETCH_FROM_GIT_PATH="$PICO_SDK_PATH/tools"
```

### Linux

```shell
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib libusb-1.0-0-dev
git clone https://github.com/raspberrypi/pico-sdk.git
git -C pico-sdk submodule update --init
export PICO_SDK_PATH=/path/to/pico-sdk
export PICOTOOL_FETCH_FROM_GIT_PATH="$PICO_SDK_PATH/tools"
```

## Build the Client Apps

### macOS

You can build the code from the accompanying Xcode project files:

* `cli2c.xcodeproj` — Contains `cli2c`, `segment` and `matrix`.
* `cliwire.xcodeproj` — Contains `cliwire`, `ds18b20` and `sensor` (GUI).

In each case:

1. Archive the project.
1. Save the build products on the desktop.
1. Copy the binary artifacts to your preferred location listed in `$PATH`.

### Linux

1. Navigate to the repo directory.
1. `cd linux`
1. `cmake -S . -B build`
1. `cmake --build build`
1. Copy the binary artifacts to your preferred location listed in `$PATH`.

## Build and Deploy the Bus Host Firmware

**Important** WE STRONGLY RECOMMEND YOU BUILD THE FIRMWARE WITH PICO SDK 1.5.0 OR ABOVE.

1. Navigate to the repo directory.
1. Optionally enter `export PICO_BOARD=x`, where x is a board name ([see above](#bus-host-board)).
1. `cmake -S . -B build`
1. `cmake --build build`
1. Write the firmware depending on which board you are using:
    * `./deploy.sh /path/to/device build/firmware/pico/firmware_pico_rp2040.uf2`
    * `./deploy.sh /path/to/device build/firmware/qtpy/firmware_qtpy_rp2040.uf2`
    * `./deploy.sh /path/to/device build/firmware/promicro/firmware_promicro.uf2`
    * `./deploy.sh /path/to/device build/firmware/tiny/firmware_tiny2040.uf2`
    * `./deploy.sh /path/to/device build/firmware/trinkey/firmware_trinkey2040.uf2`
    * `./deploy.sh /path/to/device build/firmware/nano/firmware_arduino_nano.uf2`

The deploy script tricks the RP2040/RP2350-based board into booting into disk mode, then copies over the newly build firmware. When the copy completes, the board automatically reboots. This saves a lot of tedious power-cycling with the BOOT button held down.

### Debug vs Release

You can switch between build types when you make the `cmake` call in step 3, above. A debug build is made by default, but you can make this explicit with

```shell
cmake -S . -B build -D CMAKE_BUILD_TYPE=Debug
```

For a release build, which among various optimisations omits UART debugging code, call:

```shell
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
```

Follow both of these commands with the usual

```shell
cmake --build build
```

## What’s What

The contents of this repo are:

```
/depot
|
|___/client                         // Client-side code, written in C
|   |___/cli2c                      // A generic CLI tool for any I&sup2;C device
|   |___/matrix                     // An HT16K33 8x8 matrix-oriented version of cli2c
|   |___/segment                    // An HT16K33 4-digit, 7-segment-oriented version of cli2c
|   |___/cliwire                    // A generic CLI tool for any 1-Wire device
|   |___/common                     // Code common to all versions
|   |___/i2c                        // I2C driver code
|   |___/onewire                    // 1-Wire driver code
|   |___/ds18b20                    // A DS18B20-oriented version of cliwire
|   |___/sensor                     // A Swift/C macOS GUI app that uses the 1-Wire and serial driver code.
|
|___/firmware                       // The RP2040/RP2350 host firmware, written in C
|   |___/common                     // Code common to multiple versions
|   |___/feather                    // An Adafruit Feathr RP2040 version
|   |___/nano                       // An Arduino Nano RP2040 Connect version
|   |___/pico                       // The Raspberry Pi Pico version
|   |___/pico2                      // The Raspberry Pi Pico 2 version
|   |___/promicro                   // A SparkFun ProMicro RP2040 version
|   |___/qtpy                       // An Adafruit QTPy RP2040 version
|   |___/tiny                       // A Pimoroni Tiny 2040 version
|   |___/trinkey                    // An Adafruit QT2040 Trinkey version
|
|___/examples                       // Demo apps
|   |___cpu_chart_matrix.py         // CPU utilisation display for 8x8 matrix LEDs
|   |___cpu_chart_segment.py        // CPU utilisation display for 4-digit segment LEDs
|   |___cpu_chart_ltp305_cli2c.py   // CPU utilisation display for twin LTP305 matrices
|   |___mcp9808_temp_cli2c.py       // Periodic temperature reports from an MCP9808 sensor
|   |___mcp9808_temp_disp_cli2c.py  // Periodic temperature reports from an MCP9808 sensor
|                                   // and presented on a 4-digit segment LED
|
|___/linux                          // Linux build settings (CMake) for the client apps
|
|___CMakeLists.txt                  // Top-level firmware project CMake config file
|___pico_sdk_import.cmake           // Raspberry Pi Pico SDK CMake import script
|
|___firmware.code-workspace         // Visual Studio Code workspace for the board firmware
|___cli2c.xcodeproj                 // Xcode project for cli2c, matrix and segment
|___cliwire.xcodeproj               // Xcode project for cliwire
|
|___deploy.sh                       // A .uf2 deployment script that saves pressing
|                                   // RESET/BOOTSEL buttons. LEGACY: now largely replaced
|                                   // by the official picotool
|
|___README.md
|___LICENSE.md
```

## Devices

Under macOS, RP2040/RP2350-based boards will appear in `/dev` as `cu.usbmodemXXXXX` or similar. You can use my [`dlist`](https://github.com/smittytone/dlist) utility to save looking up and keying in these names, which can vary across boots.

Under Linux, specifically Raspberry Pi OS, boards appear as `/dev/ttyACM0`. You may need to add your user account to the group `dialout` in order to access the port:

1. Run `sudo adduser smitty dialout`
1. Log out and then back in again.

## Client Apps

The following client apps are included in the repo. They are documented [on my documentation site](https://smittytone.net/docs/depot_i2c.html).

| App | Description | Platform | Docs |
| --- | --- | --- | --- |
| `cli2c` | A command line utility for generic I&sup2;C use | macOS, Linux | [Link](https://smittytone.net/docs/depot_i2c.html#cli2c) |
| `matrix` | A specific driver for HT16K33-based 8x8 LED matrices | macOS, Linux | [Link](https://smittytone.net/docs/depot_i2c.html#matrix) |
| `segment` | A specific driver for HT16K33-based 4-digit, 7-segment LEDs | macOS, Linux | [Link](https://smittytone.net/docs/depot_i2c.html#segment) |
| `cliwire` | A generic 1-Wire command line utility | macOS, Linux | [Link](https://smittytone.net/docs/depot_1wire.html#cliwire) |

**Note** These are essentially demo apps, and are required by the examples listed below. When built for debug release they will emit logging for the traffic passing to and from the bus host board. This can be disabled by using Xcode to compile a release build by choosing **Archive** from the **Product** menu.

## Full Examples

The [`examples`](examples/) folder contains Python 3.x scripts that make use of the above apps:

* `cpu_chart_matrix.py` — A rudimentary side-scrolling CPU activity chart. Requires an HT16K33-based 8x8 matrix LED. Requires the `matrix` CLI tool in your `$PATH` (see above).
* `cpu_chart_segment.py` — A CPU activity numerical percentage readout. Requires an HT16K33-based 4-digit, 7-segment LED. Requires the `segment` CLI tool in your `$PATH` (see above).
* `mcp9808_temp_cli2c.py` — Second-by-second temperature readout. Requires an MCP9808 temperature sensor breakout. Requires the `cli2c` CLI tool in your `$PATH` (see above).
* `mcp9808_temp_disp_cli2c.py` — A version of the previous example that presents the temperature on an attached HT16K33-based 4-digit, 7-segment LED. I connected both on chained STEMMA ports connected to an Adafruit Feather RP2040 board. It’s a good example of driving two I&sup2;C devices on the same bus. Requires the `cli2c` and `segment` CLI tools in your `$PATH` (see above).
* `cpu_chart_ltp305_cli2c.py` — A version of the side-scrolling CPU activity chart. Requires a [Pimoroni LED Matrices + Driver](https://shop.pimoroni.com/products/led-dot-matrix-breakout). Requires the `cli2c` CLI tool in your `$PATH` (see above).

All the examples run at the command line and take the path to the adaptor device as a required argument and an I&sup2;C address as a second, optional argument (if you are not using each device’s default address). For example:

```shell
python examples/cpu_chart_ltp305_cli2c.py /dev/cu.usbserial-0101 0x63
```

**Note** These Python examples use the `psutil` library, which is not provided as standard. To try the examples, set up a virtual environment:

```shell
cd depot
python -m venv .python
source .python/bin/activate
pip install psutil
python examples/cpu_chart_ltp305_cli2c.py /dev/cu.usbserial-0101 0x63
...
deactivate
```

## Acknowledgements

This work was inspired by James Bowman’s ([@jamesbowman](https://github.com/jamesbowman)) [`i2ccl` tool](https://github.com/jamesbowman/i2cdriver), which was written as a macOS/Linux/Windows command line tool to connect to his [I2CMini board](https://i2cdriver.com/mini.html).

My own I&sup2;C driver code started out based on James’ but involves numerous changes and (I think) improvements. I also removed the Windows code and some functionality that I don’t need (I&sup2;C capture, monitoring). Finally, it targets fresh firmware I wrote from the ground up to run on an RP2040/RP2350-based board, not the I2CMini.

Why? Originally I was writing an HT16K33 driver based directly on James’ code, but I accidentally broke the pins off my I2CMini — only to find it is very hard to find new boards. James’ firmware is written in a modern version of Forth, so I had no choice but to learn Forth, or write code of my own. I chose the latter.

Thanks are also due to Hermann Stamm-Wilbrandt ([@Hermann-SW](https://github.com/Hermann-SW)) for the basis for the [deploy script](#build-and-deploy-the-bus-host-firmware).

The 1-Wire driver is based on code I produced for the ~~Twilio~~KORE Wireless Electric Imp IoT platform some years ago.

## Release Notes

See [CHANGELOG.md](CHANGELOG.md).

*You can find release notes for previous versions [here](https://github.com/smittytone/cli2c)*

## Licences and Copyright

All client apps are © 2026 Tony Smith (@smittytone) and licensed under the terms of the MIT Licence.

The RP2040/RP2350 firmware is © 2026, Tony Smith (@smittytone). It is licensed under the terms of the MIT Licence.

# Nanosatellite Flight Software

[![Build Status](http://jenkins.spel.cl/buildStatus/icon?job=SUCHAI-Flight-Software-pipeline)](http://jenkins.spel.cl/job/SUCHAI-Flight-Software-pipeline/)
[![GitHub tag](https://img.shields.io/github/tag/spel-uchile/SUCHAI-Flight-Software.svg)]()
[![license](https://img.shields.io/github/license/spel-uchile/SUCHAI-Flight-Software.svg)](https://www.gnu.org/licenses/gpl-3.0.en.html)

SUCHAI Flight software was originally developed to be used in the 
[SUCHAI](http://spel.ing.uchile.cl/suchai.html) nanosatellite (1U 
[CubeSat](https://en.wikipedia.org/wiki/CubeSat)). SUCHAI was launch into orbit 
in June 2017 and has been working properly.

The main idea was to design a highly modular and extensible flight software architecture to help the
development of CubeSats projects that are composed by large and heterogeneous 
teams. Provides a ready-to-use nanosatellite flight software, a flexible way to add/remove functionalities or paylodads, and supports an incremental development.

The software architecture is based on the command processor design pattern. Developers
can extend the functionalities by adding new commands to the system (with low 
impact in the whole software) or adding new clients that can request any of the available
commands depending on their custom control strategy. Commands and control modules 
can be added or removed with zero impact in the software main functionalities.
Once a command is implemented, it can be used in the software itself and also
as a telecommand.

Current implementation uses the [LibCSP](https://github.com/libcsp/libcsp) to communicate subsytems and the ground station. Linux port can use the LibCSP with [ZMQ](http://zeromq.org/) interface.

Visit http://spel.ing.uchile.cl to get latest news about SUCHAI project.
Visit http://www.freertos.org/ to get FreeRTOS source code and documentation.
Visit https://github.com/libcsp/libcsp to get the latest version of the LibCSP.

More: 
- Paper (en, open access): https://ieeexplore.ieee.org/document/8758807
- Bachelor thesis (es): http://tesis.uchile.cl/handle/2250/115307

## Key features

* Highly extensible and modular command processor architecture.
* Ported to FreeRTOS and Linux.
* Designed for medium-range microcontrollers such as ATMEL AVR32, Espressif ESP32.
* Ported and tested on Raspberry PI.
* Can be used as Flight Software, Ground Station Software or general purpose embedded system firmware.
* Flight inheritance: SUCHAI I (Jun 2017)

## Build status

- Build and test status: https://jenkins.spel.cl/
- Build visual status: https://data.spel.cl/
- Last software architecture image generated:

<p align="center">
<img src="https://data.spel.cl/viz_svg/last.svg" alt="Last architecture extracted" height="500px"/>
</p>

## Build notes

SUCHAI flight software was designed to run in multiple embedded architectures 
using FreeRTOS. It was also ported to Linux to facilitate the development and debugging. 
Currently, it has been tested in the following OS/Architectures:

* Ubuntu 18.04 x86_64
* Manjaro x86_64
* ArchLinux x86_64
* Debian Stretch RPi 3 (armv7l)
* FreeRTOS ESP32
* FreeRTOS AVR32 UC3 (avr32uc3)
* FreeRTOS Nanomind A3200 (avr32uc3)

### Linux build

#### Requirements
Linux installation requires the following libraries:
* build-essential
* cmake
* pkg-config
* pthread
* cunit (libcunit-dev)
* sqlite3 (libsqlite3-dev)
* zmq (libzmq3-dev)
* unzip
* postgresql
* libpq-dev
* python (python 2)
* python3

#### Clone
Clone this repository

```bash
git clone https://github.com/spel-uchile/SUCHAI-Flight-Software
cd SUCHAI-Flight-Software
```
Go to the folder, locate ```config.h``` file to customize parameters and 
functionalities.

#### Build
Use the ```compile.py``` python script to easily install drivers, create the
settings header ```configure.h``` and build:

```bash
python3 compile.py LINUX --drivers
```

This will download and build the libcsp too. Subsequents builds do not require
the ```--drivers``` params. Use ```--help``` to learn how to customize the
build:

```bash
python3 compile.py --help
```

### Run
Go to the build folder ex: ```cd build_linux``` and execute

```bash
./SUCHAI_Flight_Software
```

#### Using ZMQ interface
In Linux, LibCSP uses the ZMQ interface to communicate different nodes. To pass
messages between zmq_hub interfaces, we required a ZMQ Forwarder Device (Proxy)
running in background. To start the ZMQ Forwarder server:

```bash
cd sandbox
python minzmqhub.py
```

It is possible to change the default ports (8001, 8002) and activate a monitor 
socket (8003) that will print all messages to ```stout``` using:

```bash
cd sandbox
python minzmqhub.py [-i IN_PORT] [-o OUT_PORT] [-m MON_PORT] [--mon]
```

Refs:
- https://github.com/libcsp/libcsp/pull/64
- https://github.com/libcsp/libcsp/issues/68
- http://learning-0mq-with-pyzmq.readthedocs.io/en/latest/pyzmq/devices/forwarder.html

### Build for other architectures
Currently the fight software supports the following architectures (some platforms
may have limited or under development support):

- Nanomind A3200: ```python3 compile.py FREERTOS --arch NANOMIND```
- Atmel AV32UC3 Xplained board: ```python3 compile.py FREERTOS --arch AVR32 --sch_com 0```
- Raspberry Pi: ```python3 compile.py LINUX```
- Esspressif ESP32: ```python3 compile.py FREERTOS --arch ESP32 --sch_com 0```

Please refer to the documentation for more details

## How to cite

Plain text
```
C. E. Gonzalez, C. J. Rojas, A. Bergel and M. A. Diaz, "An architecture-tracking approach to evaluate a modular and extensible flight software for CubeSat nanosatellites," in IEEE Access.
doi: 10.1109/ACCESS.2019.2927931
keywords: {cubesat;embedded software;flight software;nanosatellites;software architecture;software quality;software visualization;open source},
URL: http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=8758807&isnumber=6514899
```

Bibtex
```bibtex
@article{gonzalez2019, 
author={C. E. {Gonzalez} and C. J. {Rojas} and A. {Bergel} and M. A. {Diaz}}, 
journal={IEEE Access}, 
title={An architecture-tracking approach to evaluate a modular and extensible flight software for CubeSat nanosatellites}, 
year={2019}, 
volume={}, 
number={}, 
pages={1-1}, 
keywords={cubesat;embedded software;flight software;nanosatellites;software architecture;software quality;software visualization;open source}, 
doi={10.1109/ACCESS.2019.2927931}, 
ISSN={2169-3536}, 
month={},}
```

## Contact

[![issue tracker](https://img.shields.io/github/issues/spel-uchile/SUCHAI-Flight-Software.svg)](https://github.com/spel-uchile/SUCHAI-Flight-Software/issues) Use the issue tracker to submit questions, requirements and bugs.

[![SPEL Team](https://img.shields.io/twitter/follow/spel_uchile.svg?style=social&label=Follow)](https://twitter.com/SPEL_UCHILE) the SPEL team at Twitter to get latest news about the SUCHAI project


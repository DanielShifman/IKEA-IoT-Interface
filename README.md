[![Python](https://img.shields.io/badge/python-3.10-informational.svg)](https://www.python.org/downloads/release/python-3100/)
[![C++](https://img.shields.io/badge/C++-20-informational.svg)](https://en.cppreference.com/w/cpp/20)
[![Licence](https://img.shields.io/badge/licence-LGPLv3-important.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)

# IKEA Smart Device Interface
## Available in C++ and Python

---
### Introduction
This is a graphical interface for IKEA smart devices. It is available in two versions: C++ and Python.
The while both versions use Qt for the GUI, and are effectively functionally identical (see differences below),
the Python version is simpler and easier to use, while the C++ version is faster and more efficient.

Device attribute modification via command line arguments is supported in both versions
for the sake of automation. If this is your intended use case,
the C++ version is recommended for its faster execution time.

### Requirements
#### C++
Written for C++20, may work with other versions (not guaranteed).

Libraries supplied via git:
- [nlohmann/json](https://github.com/nlohmann/json.git)
- [cpp-httplib](https://github.com/yhirose/cpp-httplib.git)
- [crypto++](https://github.com/weidai11/cryptopp.git)

Libraries not supplied via git submodules:
- [Qt](https://www.qt.io/download)
#### Python
Minimum Python version: 3.10\
All modules are available via pip.\
Please note that Qt is accessed via PyQt5, which is a Python wrapper for Qt.

### Build
#### C++
1. Clone the repository.
2. Clone the libraries listed above.
3. Don't forget to add the libraries to your linker.
##### Windows
The Windows build is written for VC++ 2019. It may work with other versions (not guaranteed).
1. Install Qt.
    - You only need the Qt 5.15.2 MSVC2019 64-bit version prebuilt binaries.
    - Available from the Qt installer for open source users (LGPLv3).
    - Add the Qt bin directory to your PATH.
    - Don't forget to add the library to your linker.
##### Unix-based
The Unix build is written for G++ 13.2.0. It may work with other versions (not guaranteed).
1. Install Qt.
    - You only need the Qt 5.15.2 GCC 64-bit version prebuilt binaries.
    - Available from the Qt installer for open source users (LGPLv3).
    - Add the Qt bin directory to your PATH.
    - Don't forget to add the library to your linker.
##### All platforms
2. Compile the source using your preferred compilation method.
#### Python
1. Clone the repository.
2. Run `pip install -r requirements.txt` to install the required modules.
    - Please note that requirements.txt will allow newer versions of the modules to be installed, although only the versions listed have been tested.

### Usage
*If running on a Unix-based system, please ensure that the program is run as root.*
*If not, some features may not work as intended.*
#### First launch
Upon first launch, the program will create resources subdirectory in the current directory.
This directory will contain the configuration and device ID cache files.
If you would like the program to use a taskbar icon, please place a PNG file named `ikea.png` in the resources subdirectory.
One such file is included in the repository.

You will be prompted to enter the IP address and port of the gateway.\
If you do not know this information:
-  You can find the IP address using various methods such as:
- Your router's DHCP client list.
- [nmap](https://nmap.org/).
- Guessing. (Not recommended)
-  The port it is likely 8443.

The program will then attempt to connect to the gateway.
If the connection is successful, you will be prompted to press the action button on the gateway.
This is to authenticate the program with the gateway and generate a token.

The program will then create a device ID cache file in the resources subdirectory.
This file maps device names to device IDs, and is used to reduce the number of requests sent to the gateway.

##### First and subsequent launches
After loading the configuration and device ID cache files, the program will ping the gateway to check if it is still connected.
If a ping cannot be established, the program will request new networking values and attempt to reconnect.
If issues persist, please see the troubleshooting section below.

##### GUI
If launching with no command line arguments, the program will open a GUI window.
The main window will list all devices connected to the gateway (including the gateway itself).\
Clicking on a device button will open a new window with the device's attributes.
Modifiable attributes will have their values presented in a suitable user interface element.
Other attributes will be displayed as text.

Currently, modifiable attributes are limited to "isOn" and light-specific attributes.

##### Command line arguments
Command line arguments are used to modify device attributes for the sake of automation.
At the moment, only one attribute can be modified at a time via CLI.

The syntax for the command line arguments is as follows:
```
\[path to executable\] \[device name\] \[attribute name\] \[attribute value\]
```
ex.
```bash
./smartHub "Desk Lamp" isOn 1
./smartHub.exe "Desk Lamp" isOn 1
```

If using the Python, an attribute datatype must be specified as well:
```
\[path to executable\] \[device name\] \[attribute name\] \[attribute value\] \[attribute datatype\]
```
ex.
```bash
python main.py "Desk Lamp" isOn true bool
```

If specifying a boolean attribute, the value can be toggled using "auto" as the attribute value:
```bash
./smartHub "Desk Lamp" isOn auto
./smartHub.exe "Desk Lamp" isOn auto
python main.py "Desk Lamp" isOn auto bool
```

Please note: currently only boolean attribute modification has been tested via CLI.

### Currently known differences between C++ and Python versions

| Feature                | C++                                                                                                                                                         | Python                                                                                                                                                                      |
|------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Light level slider     | <ul><li>Only allows adjusting slider by moving handle</li><li>Changes light level on release of slider handle</li><li>UI responds instantaneously</li></ul> | <ul><li>Allows adjusting slider by moving handle, clicking on slider, mouse scroll</li><li>Changes light level as slider moves</li><li>UI is very slow to respond</li></ul> |
| Command-line arguments | Syntax: `smartHub[.exe] [device name] [attribute name] [attribute value]`                                                                                   | Syntax: `smartHub[.exe] [device name] [attribute name] [attribute value] [value type (defaults to string)]<bool, float, string>`                                            |
| Window Icon            | No                                                                                                                                                          | On Windows                                                                                                                                                                  |

### Troubleshooting
If you were previously able to ping the gateway, but are now unable to do so, please consider the following:
1. The gateway may have been assigned a new IP address.
    - This can happen if the gateway is restarted, or if the router is restarted.
      Either could have occurred if there has been a recent power outage.
    - To prevent this occurring in the future, you can assign the gateway a static IP address.

### Licensing
This project is licensed under the LGPLv3 licence as required by Qt.
If you would like to modify this project and release it, and it still uses Qt, you will be required to release it under the available licences for Qt.
Please see the LICENCE file for more information.
Please see the Qt website for more information on Qt licensing.

### Acknowledgements
Thank you to the following GitHub users for use of their libraries:
- [json](https://github.com/nlohmann/json.git)
    - [Niels Lohmann (nlohmann)](https://github.com/nlohmann)
    - [Other contributors](https://github.com/nlohmann/json/graphs/contributors)
- [cpp-httplib](https://github.com/yhirose/cpp-httplib.git)
    - [Yuji Hirose (yhirose)](https://github.com/yhirose)
    - [Other contributors](https://github.com/yhirose/cpp-httplib/graphs/contributors)
- [crypto++](https://github.com/weidai11/cryptopp.git)
    - [Wei Dai (weidai11)](https://github.com/weidai11)
    - [Other contributors](https://github.com/weidai11/cryptopp/graphs/contributors)
# OpenSourceLogger

This is a data acquisition software made in C++ with ImGui framwork and OpenGL.
The purpose with this software is to collect measurement data and store that data inside a database
and also downloading measurement data from that database. 

In order to use this software, the following need to be known:

* Calibration with straight line equation
* CAN-bus frame
* Sampling measurement
* Comma Separated Value (CSV) file structure
* MySQL connection
* SAE-J1939 protocol
* Unsigned 8-bit, 16-bit, 32-bit maximum values

# Functionality

* 12 x ADC at 16-bit resolution for 0-20mA input with programmable gain
* 5 x Differential ADC at 16-bit for 0-20mA input with programmable gain
* 3 x DAC at 12-bit with 0-20mA output
* 8 x PWM for 0-2.2A with N-channel MOSFET
* 10 x Digital Input
* 1 x CAN-bus channel
* 4 x Input Capture for 0 kHz to 10kHz
* 3 x Encoder for -32768 to 32767 pulses
* 1 x USB port for connecting with [STM32-PLC](https://github.com/DanielMartensson/STM32-PLC)
* MySQL database connection
* SAE-J1939 communication standard with the software [Open-SAE-J1939](https://github.com/DanielMartensson/Open-SAE-J1939)
* CAN-terminal analyzer

# How to use this software

## Building from source

Download Microsoft Visual Studio. I have been using Microsoft Visual Studio 2022 to create this software.
You need a MSVC-compiler as well. The C++ standard of this project is C++20. Clone the project and 
run the `.sln` file.

Also install these following dependencies from `vcpkg`

 -VCPKG dependencies
  -imgui[core,glfw-binding,opengl3-binding]:x64-windows
  -opengl:x64-windows
  -boost-asio:x64-windows
  -mysql-connector-cpp:x64-windows
  -nlohmann-json:x64-windows
  -stb:x64-windows


## Run pre-built binaries

Inside `x64/Release` there is a file called `OpenSourceLogger.exe`. Run that.

# Pictures

![a](https://raw.githubusercontent.com/DanielMartensson/OpenSourceLogger/main/Pictures/Mainview.png)

![a](https://raw.githubusercontent.com/DanielMartensson/OpenSourceLogger/main/Pictures/Databaseview.png)


# Status of the project

* Bug testing - This is just the first version
* Work on design 
* Calibration factor for the analog measurements due to leakage of the zener diodes of STM32-PLC
# ALPHA SERIES MENU

## Table of Contents
1. [Purpose/Overview](#purposeoverview)
2. [Features](#features)
3. [Build on ARM 64 bit Embedded Environment][build-on-arm-64-bit-embedded-environment]
4. [Running the Program](#running-the-program)
   - [How to Run the Menu Program](#how-to-run-the-menu-program)
   - [How to Run a Separate .sh File](#how-to-run-a-separate-sh-file)
5. [Adding More Features](#adding-more-features)
   - [File](#file)
   - [Reference](#reference)
6. [Troubleshoot](#troubleshooting-errors)
7. [Disclaimer](#disclaimer)
8. [Contact Information](#contact-information)

## Purpose/Overview

The ALPHA SERIES MENU is designed to facilitate control over the Alpha Series cameras using a terminal-based interface. This menu, created based on an SDK, is located in the `/bin` directory and is compatible with every Alpha camera model. It provides specialized menu options tailored to each camera model, allowing users to efficiently manage camera settings and operations.

## Features

The menu offers up to 15 different options, depending on camera model compatibility:

- **Change Camera Exposure Mode**
     - Currently Changes the Exposure mode to Manual (M) on the Dial
- **Change Camera Zoom Settings**
     - Zoom in for 6 seconds then Zooms back out for 6 seconds
- **Shoot an Image and Retrieve It**
     - Shoots an image then uploads image to host computer
- **MTP (Media Transfer Protocol)**
     - Gives a list of pictures or videos on the camera, then offeres user to upload onto host computer
- **Date-Time Configuration** 
     - Shows the date-time when runtime of command is executed
- **Get Property Values**
     - Gives option to get a specific property value or get all property values of the camera
- **Single Shoot Continuously**
     - Do single shoot multiple times
- **Image ID Management**
     - Get the image ID of a specific image
- **ISO Setting**
     - Set the ISO setting
- **Shutter Speed Setting**
     - Set the shutter speed setting
- **Still File Format Setting**
     - Set the still file format setting
- **SD Card Format Setting**
     - Do an SD card Format on either slot 1 or 2
- **Movie Record Setting**
     - Record a movie
- **Save Settings**
     - Save settings of camera to a .dat file
- **Load Settings**
     - upload camera setting from a .dat file

## Running the Program

#### System Requirements

Designed to run on both Linux 64x Desktop and Embedded 64-bit ARM NVIDIA Jetson operating systems.
### Build On ARM 64 Bit Embedded Environment
**Please follow these steps bellow in order to run the program in an ARM 64-bit Embedded Environment**
1. Navigate to the `libusb` directory of the project
   ```bash
   $ cd Desktop/alphaSeries/Linux-SDK-Official/libusb
   ```
2. Execute the following .sh file
   ```bash
   $ /autogen.sh
   ```
3. Execute Make
   ```bash
   $ make
   ```
4. Execute make install
   ```ba sh
   $ sudo make install
   ```
5. Your Embedded ARM environment can now succesfully build this project. Please proceed to [How to Run the Menu Program](#how-to-run-the-menu-program)
### Running program on Linux 64x Operating System
**No Extra steps needed to build the project in this environment. Please proceed to [How to Run the Menu Program](#how-to-run-the-menu-program)**
#### How to Run the Menu Program

1. Open a terminal window.
2. Navigate to the directory containing the menu script. Make sure the SDK file is located on your Desktop. Adjust the path if necessary.
   
   ```bash
   cd Desktop/alphaSeries/Linux-SDK-Official/out/bin
3. Set the LD_LIBRARY_PATH to include the path to the SDK libraries.
   ```bash
   export LD_LIBRARY_PATH=~/Desktop/alphaSeries/Linux-SDK-Official/out/lib
4. Execute the menu program.
   ```bash
   ./menu.sh

#### How to Run a Separate .sh File

To run a `.sh` file outside of the menu program, you need to open and authenticate the session before executing the script. After running the script, it is important to close the session. Follow these steps:

1. Open a terminal window.
2. Navigate to the directory containing the menu script. Make sure the SDK file is located on your Desktop. Adjust the path if necessary.
   
   ```bash
   cd Desktop/alphaSeries/Linux-SDK-Official/out/bin
3. Set the LD_LIBRARY_PATH to include the path to the SDK libraries.
   ```bash
   export LD_LIBRARY_PATH=~/Desktop/alphaSeries/Linux-SDK-Official/out/lib
4. Open Session
   ```bash
   ./control open
5. Authenticate Session
   ``` bash
   ./control auth
6. Execute the .sh File (any .sh file in the `/bin` directory)
   ```bash
   ./iso-setting.sh
7. Close session
   ``` bash
   ./control close

**Note:** If you prefer to run the `.sh` file without the menu program, you can manually uncomment the `./control open`, `./control auth`, and `./control close` commands in the respective `.sh` file to automate the process.


## Troubleshooting Errors
Here are solutions to some common errors that may arise when running this program. 
- **Authentication Error** If there is an error with `./control auth`,(you visually see this error when session does not equal 0 in the terminal when you run `./control auth`),
this is probably due to the program misunderstanding which usb port your alpha camera is plugged into.
1.  Please go to the following directory
``` bash
cd Desktop/alphaSeries/Linux-SDK-Official/libremotecameracontrol/ports
```
2. Enter the ports_usb.h file and check lines 11 and 12
``` c++
//#define BULK_MAX_PACKET_SIZE (512) // USB 2.0
#define BULK_MAX_PACKET_SIZE (1024) // USB 3.0
```
3. Uncomment the line of code that is compatible with your usb port. Then re-run:
```bash
./control open
./control auth
```
You should see `session=0` which indicates a succsesful open session

- **Permission Error** Before running the menu program, ensure every file in the bin has executable permission. 
1. To do so, enter the directory
```bash
cd Desktop/alphaSeries/Linux-SDK-Official/out/bin
```
2. Give permission to the permission.sh file
```bash
chmod +x permission.sh
```
3. Run the permission.sh file
```bash
./permission.sh
```
4. Every file in `/bin` should have executable permission. now run the program
```bash
./menu.sh
```
## Disclaimer
This menu has been tested on 4 Alpha camera models and is assumed to be compatible with every Alpha camera. The observed compatibility and supported features are as follows:
- **A7R5:**  All 15 menu options are supported.
- **LR1:**  All 15 menu options are supported.
- **A7R4:** 12 menu options are supported; 13 options are supported if an optical zoom lens is connected.
- **A7M4:**  14 menu options are supported; 15 options are supported if an optical zoom lens is connected.

## Adding More Features

### File

- **Create `.sh` file**: To add new features, create a new `.sh` file in the `/bin` directory.
- **Refer to existing `.sh` files**: Examine existing `.sh` files in the `/bin` directory to understand how to structure your script and implement functionality.

### Reference

- **Sony Camera Features**: Refer to [Sony's Camera Help Guide](https://helpguide.sony.net/ilc/2230/v1/en/contents/TP0003027198.html) to better understand the camera features and settings.
- **Camera Control PTP 3 Reference**: Consult the `Camera Control PTP 3 Reference.pdf` to understand the functions available for your camera model.

## Contact Information

For any questions or support, please reach out to:

**School Email:** cherresb@grinnell.edu

[build-on-arm-64-bit-embedded-environment]: #build-on-arm-64-bit-embedded-environment

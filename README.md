# ALPHA SERIES MENU

## Purpose/Overview

The ALPHA SERIES MENU is designed to facilitate control over the Alpha Series cameras using a terminal-based interface. This menu, created based on an SDK, is located in the `/bin` directory and is compatible with every Alpha camera model. It provides specialized menu options tailored to each camera model, allowing users to efficiently manage camera settings and operations.

## Features

The menu offers up to 15 different options, depending on camera model compatibility:

- **Change Camera Exposure Mode**
- **Change Camera Zoom Settings**
- **Shoot an Image and Retrieve It**
- **MTP (Media Transfer Protocol)**
- **Date-Time Configuration**
- **Get Property Values**
- **Single Shoot Continuously**
- **Image ID Management**
- **ISO Setting**
- **Shutter Speed Setting**
- **Still File Format Setting**
- **SD Card Format Setting**
- **Movie Record Setting**
- **Save Settings**
- **Load Settings**

## Running the Program

### System Requirements

This program is designed to run on an Ubuntu system

### How to Run the Menu Program

1. Open a terminal window.
2. Navigate to the directory containing the menu script. Make sure the SDK file is located on your Desktop. Adjust the path if necessary.
   
   ```bash
   cd Desktop/Linux-SDK-Official/out/bin
3. Set the LD_LIBRARY_PATH to include the path to the SDK libraries.
   ```bash
   export LD_LIBRARY_PATH=~/Desktop/Linux-SDK-Official/out/lib
4. Execute the menu program.
   ```bash
   ./menu.sh

### How to Run a Separate .sh File

To run a `.sh` file outside of the menu program, you need to open and authenticate the session before executing the script. After running the script, it is important to close the session. Follow these steps:

1. Open a terminal window.
2. Navigate to the directory containing the menu script. Make sure the SDK file is located on your Desktop. Adjust the path if necessary.
   
   ```bash
   cd Desktop/Linux-SDK-Official/out/bin
3. Set the LD_LIBRARY_PATH to include the path to the SDK libraries.
   ```bash
   export LD_LIBRARY_PATH=~/Desktop/Linux-SDK-Official/out/lib
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

## Disclaimer
This menu has been tested on three Alpha camera models and is assumed to be compatible with every Alpha camera. The observed compatibility and supported features are as follows:
- **A7R5**  All 15 menu options are supported.
- **A7R4** 12 menu options are supported; 13 options are supported if an optical zoom lens is connected.
- **A93**  14 menu options are supported; 15 options are supported if an optical zoom lens is connected.

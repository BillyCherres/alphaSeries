# ALPHA SERIES MENU

1. [Purpose/Overview](#purposeoverview)
2. [Features](#features)
3. [Running the Program](#running-the-program)
   - [How to Run the Menu Program](#how-to-run-the-menu-program)
   - [How to Run a Separate .sh File](#how-to-run-a-separate-sh-file)
4. [Disclaimer](#disclaimer)

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

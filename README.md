----------------------------------------------------------------------------------------
COMPUTER VISION TABLETOP RPG MAP CREATOR TOOL SETUP GUIDE
----------------------------------------------------------------------------------------

Thank you for reading the set-up guide! This document covers system purpose, achitecture
and technologies, and should enable any reader to navigate the repository & possibly 
build their own versions with this source code. 

This project was created as dissertation project (Jan 2026 - April 2026) by Eva Martens.

The system combines aspects of computer vision and game code to create an application 
that allows users to prototype a 3D phantasy world map, without ever having used a game
engine or 3D modelling before, ideal for DMs leading table top RPGs. 
The idea is that the user creates a "2D map" on a 32x32 green lego plate, that they
poplute with 8 seperate features. The system uses thresholding, segmentation,  
feature and colour detection to differentiate between bricks and uses that data
to spawn corresponding features onto the map.

Unreal Engine 5 was chosen as medium for this project do to it's powerful graphical
capibilities and incredible PCG (procedural content generation) plugins, allowing 
each generated world to be unique. 

The concepts used in this procet are transferable to other similar projects. 
Real-world scenarios include prototyping for games or visualising worlds for 
story telling purposes. 

Heads up: While the code has been optimised for performance, this is not a light weight
application. Please read this guide carefully & also refer to:

**https://dev.epicgames.com/documentation/en-us/unreal-engine/hardware-and-software-specifications-for-unreal-engine**

The computer vision application can be a stand alone app and is platform independent and 
cross-compilabe through CMAKE, though this might require adjusting the CMakeLists.txt 
files and downloading and installing the corresponding compiler versions. 

Unreal Engine 5 natively supports cross-platform compilation. 

----------------------------------------------------------------------------------------
-----------------------------------ARCHITECTURE-----------------------------------------
----------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------

----------------------------------------------------------------------------------------
                                 CORE TECHNOLOGIES
----------------------------------------------------------------------------------------

PYTHON 3          : FLASK, OPEN CV 2, PICAMERA 2
C++ 20 or 23      : CMAKE, OPEN CV 4, GCC
XML               : YAML 
UNREAL ENGINE 5.7 : PCG, 

Visual Studio Code is recomended as code editor with ReSharper (perfect for Unreal 
Engine 5 C++) as plug-in. Intellisense does not comprehend Unreal Engine 5 
specific project structure and use of C++ macros well. 

Thonny as interface for the python scripts on the Rasberry Pi 5 is sufficent and 
should come pre-installed through the operating system imaging tool.

Unreal Engine editor 5.7 was originally used to create this project. Unreal Engine 
5.4 is the minnimum requirement due to the projects use of certain PCG content not 
available in earlier versions.

----------------------------------------------------------------------------------------
          Prerequisits - Camera system - Rasberry Pi 5 with camer module 3
----------------------------------------------------------------------------------------
Setting up the Rasberry Pi with all updates is recommended for security purposes! 
Updating the system and all subsequent package downloads can take up to 2 days. 


If rebuiding this project the following setup is recommended for the rasberry pi:
- fully updated Pi OS (set up headless operation through imanging tool with PC on same network, enable "Rasberry Pi connect")
- install python 3 (or latest)
- install pip install as package manager
    - pip install open cv 2 (python verion)
    - pip install flask
    - pip install picamera 2
- install libcamer & rpi camer packages
- install ngrok for easy & secure server connection
    - ngrok accounts are free, for more permanent architecture a paid account is recommend 
    - connection can use password protections (less secure) or mTLS (using custom certificates)
    - if the system only ever gets accessed form one location a static IP can be set as trusted connection
-----------------------------------------------------------------------------------------



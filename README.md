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

Many concepts used in this project are transferable to other similar projects. 
Real-world scenarios include prototyping for games or visualising worlds for 
story telling purposes. 

Heads up: While the code has been optimised for performance, this is not a light weight
application. Please read this guide carefully & also refer to:

**https://dev.epicgames.com/documentation/en-us/unreal-engine/hardware-and-software-specifications-for-unreal-engine**

The computer vision application can be a stand alone app and is platform independent and 
cross-compilabe through CMAKE, though this might require adjusting the CMakeLists.txt 
files and downloading and installing the corresponding compiler versions. 

Unreal Engine 5 natively supports cross-platform compilation. 

               -,,,__
                \    ``~~--,,__                /   /
                /              ``~~--,,_     //--//
     _,,,,-----,\              ,,,,---- >   (c  c)\
 ,;''            `\,,,,----''''   ,,-'''---/   /_ ;___        -,_
( ''---,;====;,----/             (-,,_____/  /'/ `;   '''''----\ `:.
(                 '               `      (oo)/   ;~~~~~~~~~~~~~/--~
 `;_           ;    \            ;   \   `  ' ,,'
    ```-----...|     )___________|    )-----'''
                \   /             \   \\
                /  /,              `\   \\
              ,'---\ \              ,---`,;,
                    ,,,

----------------------------------------------------------------------------------------
ARCHITECTURE

----------------------------------------------------------------------------------------
                                 CORE TECHNOLOGIES
----------------------------------------------------------------------------------------

PYTHON 3          : FLASK, OPEN CV 2, PICAMERA 2

C++ 20 or 23      : CMAKE, OPEN CV 4, GCC

XML               : YAML 

UNREAL ENGINE 5.7 : PCG, 

File transfer     : WinSCP

----------------------------------------------------------------------------------------
Visual Studio Code is recomended as code editor with ReSharper (perfect for Unreal 
Engine 5 C++) as plug-in. Intellisense does not comprehend Unreal Engine 5 
specific project structure and use of C++ macros well. 

Thonny as interface for the python scripts on the Rasberry Pi 5 is sufficent and 
should come pre-installed through the operating system imaging tool.

Unreal Engine editor 5.7 was originally used to create this project. Unreal Engine 
5.4 is the minnimum requirement due to the projects use of certain PCG content not 
available in earlier versions.

----------------------------------------------------------------------------------------
          Prerequisits - Camera system - Rasberry Pi 5 with camera module 3
----------------------------------------------------------------------------------------
Setting up the Rasberry Pi with all updates is recommended for security purposes! 
Updating the system and all subsequent package downloads can take up to 2 days. 


If rebuilding this project the following setup is recommended for the rasberry pi:
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
      
----------------------------------------------------------------------------------------
            Getting the images from the Rasberry Pi 5
----------------------------------------------------------------------------------------
There are several ways to get the iamges from the Rasberry Pi 5, including connecting 
via an ethernet cable. We do not have to set up a server for the RPI, this was just 
the chosen method based on the system usecases outlined in the dissertation. 

WinSCP allows for simple file transfer between Windows PCs and the Linux based
PiOS, and can be setup easily, if you are running and able to access your PiOS 
(headless or otherwise).

------------------------------------------------------------------------------------------------
*IMPORTANT: NETWORKING SECURITY*

To set up the RPI as server, it's recommended to take a few steps first:
- Change the standard SSH port on your Pi form 20 to something else between 10000 - 60000
  This is so automated attacks don't have easy access to your Rasberry Pi
- Ensure you have a long, secure password on your Pi Connect account!
- If you intend to use (username:password) authentication with ngrok, ensure you require an adequately
  long password. Ngrok does protect you from DDos attacks.
- You can set up ngrok config files to only allow certain IP adresses or use mTLS (recommended)
- DO NOT run ngrok from the root user on your rasberry pi. It is not needed and just introduces
  unneccesary risk
- You do not need to disable your firewall or change router settings. Ngrok allows you to securely forward
  your port without needing to poke a whole in your firewall.

This project uses a Restful server architecture (RestAPI), which just means we are not 
storing any session data on the rasberry pi 5. 
Make sure you have enough free memory on your rasberry pi 5 to take the HDR images and 
compress them while running the server scripts. 

Using TCP transfer on the Rasberry pi 5 may mean you need to allow TCP file transfer via
the command line first. TCP file transfer is stabl, fast and reliable. The scripts in 
this repository send compressed image data. Because of this set up your Rasberry Pi 5 
should not have to deal with any additional junk data floating around. 

MAKE SURE TO CHECK MEMORY USAGE WHEN RUNNING THE SCRIPTS ON YOUR DEVICE THE FIRST TIME. 

Better safe than having to re-setup the whole thing!

Should you have trouble setting up the server, you can also flush the port through the 
command line but BE CAREFUL NOT TO ACCIDENTALLY FLUSH A PORT YOU ARE USEING FOR ANYTHIG ELSE.

------------------------------------------------------------------------------------------------
**Important discalaimer:**
Please take anything you read here or anywhere else about cyber security with a grain of salt.
Do your own research refering to trusted sources and question anything that appears odd! 
Never use any code you do not understand. 

If you are using my code and you made it this far, great! Thank you for taking the time and I hope 
you found what you were looking for. 

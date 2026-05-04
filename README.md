----------------------------------------------------------------------------------------
COMPUTER VISION TABLETOP RPG MAP CREATOR  -  SETUP GUIDE
----------------------------------------------------------------------------------------
Last updated 30.04.26 (Added demo videos for UE5 internal game logic)

Thank you for reading the set-up guide! This document covers system purpose, achitecture
and technologies, and should enable any reader to navigate the repository & possibly 
build their own versions with this source code. 

It also includes a video breakdown of the core logic implmented in UE5 to spawn the procedural mesh and sample it at runtime below.

----------------------------------------------------------------------------------------
PROJECT STAGE
----------------------------------------------------------------------------------------
* Rasberry Pi 5 Camera System                   - Version 1 - complete
* Algorithmic Computer Vision CMake Application - Version 1 - complete
* Unreal Engine 5 Map creation game instnace    - Proof of concept achieved - in progress

Overall: Proof of concept achieved
<img width="1346" height="483" alt="Screenshot 2026-05-02 105024" src="https://github.com/user-attachments/assets/30f71f6c-66d0-4660-b095-4c9d20a49034" />
----------------------------------------------------------------------------------------
                                 CORE TECHNOLOGIES
----------------------------------------------------------------------------------------

PYTHON 3              : FLASK, OPEN CV 2, PICAMERA 2

C++ 20 or 23          : CMAKE, OPEN CV 4, GCC

XML                   : YAML 

UNREAL ENGINE 5.4 - 5.7 : PCG content plugin, Geometry Script Plugin, Procedural Mesh Component Plugin 

File transfer         : WinSCP (used during testing)

----------------------------------------------------------------------------------------

This project was created as as partial fulfilment of a BSc degree in
Computer Science with integrated Year in the Industry (G401F) (Jan 2026 - May 2026) by Eva Martens.

The system combines aspects of computer vision and game code to create an application 
that allows users to prototype a 3D fantasy world map, without ever having used a game
engine or 3D modelling before, ideal for DMs leading table top RPGs or level prototyping. 

The idea is that the user creates a "2D map" on a 32x32 green lego plate, that they
poplute with 8 seperate colored blocks. The computer vision system constists of a custom built camera system, 
which takes 4 HDR images to account for varrying lighting conditions, and a CMake application to process
the information using OpenCV C++.

Example of an early testing plate
<img width="857" height="1017" alt="HDR_early_example_lego_plate" src="https://github.com/user-attachments/assets/27333de2-e6ea-4c0a-99c0-59b3ee070661" />

Example of in-development debug screenshot mapping colours to the procedural mesh inside unreal engine through PCG texture projection
<img width="1173" height="486" alt="ExampleColourMappingOntoProceduralMesh" src="https://github.com/user-attachments/assets/7e5b44a7-78a1-4fc3-9980-af8fdf44c38a" />

The CMake application uses an algorithmic approach applying thresholding, segmentation,  
feature and colour detection to locate the lego plate in the images and then differentiates between bricks, using that data to create:
1) A PNG file mapping each pixels color to the relative location of the corresponding brick 
2) A .csv file containing pixelvalues of a height map generated with the image information
3) A persistent error log file, which the user can choose to reset at the start of the application or not
4) Consol error messages as well as warnings for various parts of the application so failure can be investigated

The CMake applications color detection was based of a literature review and heavily tested and achieves high overall
classification accuracy in varying light environments.

Unreal Engine 5 was chosen as medium for this project due to it's powerful graphical
capibilities and incredible PCG (procedural content generation) plugins, allowing 
each generated world to be unique. 

<img width="960" height="454" alt="Screenshot 2026-05-03 003606" src="https://github.com/user-attachments/assets/27b3c7fe-ff39-438a-a89b-e8708a981228" />

3D assets were taken from the FAB market place and liscensed under the "Personal Project" common liscense. 
A full list of Assets used is made available here: [TO DO - add asset ref doc] 

Heads up: This is not a light weight application. Both computer vision and UE5 are resource intensive. 
The computer vision CMake application for example uses expensive logic for a custom implementation of a colourspace (which significantly improved detection accuracy), which is why the system is not designed to run in paralell in the current version. 

For further guidance on hardware requirements please refer to:
**https://dev.epicgames.com/documentation/en-us/unreal-engine/hardware-and-software-specifications-for-unreal-engine**

All parts of this project can easily be converted into stand alone applications. Additionally, they are (mostly) platform independent and 
cross-compilabe (though this does require accomodating the various compiler versions needed for CMake, UE5, and the Rasberry Pi 5 OS you are using).

```
               -,,,__
                /    ``~~--,,__                /   /
                /              ``~~--,     //--//
     _,,,,-----,/              ,,,,---- >   (c  c)\
     ,;''            `\,,,,----''''   ,,-'''---/   /_ ;___        -,_
( ''---,;====;,----/             (-,,_____/  /'/ `;   '''''----\ `:.
(                 '               `      (oo)/   ;~~~~~~~~~~~~~/--~
 `;_           ;    \            ;   \   `  ' ,,'
    ```-----...|     )___________|    )-----'''
                \   /             \   \\
                /  /,              `\   \\
              ,'---\ \              ,---`,;,
                    ,,,
```

----------------------------------------------------------------------------------------
ARCHITECTURE
----------------------------------------------------------------------------------------

Visual Studio Code is reccomended as code editor with ReSharper (perfect for Unreal 
Engine 5 C++) as plug-in. Intellisense does not comprehend Unreal Engine 5 
specific project structure and use of C++ macros well. 

Thonny editor as interface for the python scripts on the Rasberry Pi 5 is sufficent and 
should come pre-installed through the operating system imaging tool.

Unreal Engine editor 5.7 was originally used to create this project. Unreal Engine 
5.4 or above should also work, as all PCG plugin content used was available for that version
of the engine. 

----------------------------------------------------------------------------------------
Video Demo of game logic
----------------------------------------------------------------------------------------
Spawning the procedural mesh with Blueprint (BP) Actor: 

https://github.com/user-attachments/assets/4513421f-a3d4-4266-a705-c57610de68d2

Setting mesh collisions, box collision around terraine for the PCG graph and the material

https://github.com/user-attachments/assets/40a71b87-bf25-4b4f-9f0e-27625c3b99d7

Using Data Assets and BP Structs to dynamically load the texture file at runtime for texture sampling

https://github.com/user-attachments/assets/e3e5d565-fadf-4542-a600-e199214062bb

Triggering the PCG Graph
(Comment: The warning on the node that turns spatial data into bounds is irrelevant, as the graph generation is triggered at runtime, when there are points available.)

https://github.com/user-attachments/assets/89d2a087-c90d-43ef-8ee0-08b0e93813f8

PCG Graph part 2: Texture projection and point filtering by colour to create bioms seperate

https://github.com/user-attachments/assets/0f5607c4-92bc-470f-8c9e-ca008362c204

End result: Game spawning terrain, sampling the surface, projecting points onto the surface and then executing logic per biom
(Sped up and cut into parts due to file size restrictions)

https://github.com/user-attachments/assets/1c35d1b3-d765-4049-8ac2-68d95e6e7b85


https://github.com/user-attachments/assets/6de279ba-5a36-45a1-bc04-062d8cb887a3


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
There are several ways to get the images from the Rasberry Pi 5, including connecting 
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

Using TCP transfer on the Rasberry pi 5 may mean you need to allow TCP file transfer 
for your chosen port via the command line first. 
TCP file transfer is stabl, fast and reliable. The scripts in 
this repository send compressed image data. Because of this set up your Rasberry Pi 5 
should not have to deal with any additional junk data floating around. 

MAKE SURE TO CHECK MEMORY USAGE WHEN RUNNING THE SCRIPTS ON YOUR DEVICE THE FIRST TIME. 

Better safe than having to re-setup the whole thing!

Should you have trouble setting up the server, you can also flush the port through the 
command line but BE CAREFUL NOT TO ACCIDENTALLY FLUSH A PORT YOU ARE USING FOR ANYTHIG ELSE.

------------------------------------------------------------------------------------------------
**Important discalaimer:**
Please take anything you read here or anywhere else about cyber security with a grain of salt.
Do your own research refering to trusted sources and question anything that appears odd! 
Never use any code you do not understand. 

If you are using my code and you made it this far, great! Thank you for taking the time and I hope 
you found what you were looking for. 
------------------------------------------------------------------------------------------------

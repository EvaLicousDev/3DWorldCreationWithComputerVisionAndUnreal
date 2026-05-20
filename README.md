----------------------------------------------------------------------------------------
COMPUTER VISION TABLETOP RPG MAP CREATOR  -  SETUP GUIDE
----------------------------------------------------------------------------------------
Last updated 16.05.26 (Updated to closer match READ.me submitted for marking)
* Additional images from development and testing available here : https://drive.google.com/drive/u/2/folders/1RrunHnxi9XC2J3ji54-7f3USUNmGu0p3
* Video demos of the UE5 game implementation here : https://drive.google.com/drive/u/2/folders/1bVbAZkOVdeuWMLbCwMZmfZq9j3SybT_7

Thank you for reading the set-up guide! This document covers system purpose, architecture
and technologies, and should enable any reader to navigate the repository & understand how 
to build their own version if they want to. 

It also includes a video breakdown of the core logic implemented in UE5 to spawn the procedural mesh and sample it at runtime below.

----------------------------------------------------------------------------------------
DESIGN SKETCH
----------------------------------------------------------------------------------------
<img width="944" height="511" alt="image" src="https://github.com/user-attachments/assets/883123fd-608f-4026-910c-93dfa06db7ef" />

This repository hosts code related to 3 modular systems that together create a tool in form 
of a computer vision algorithm, that translates predicted locations of defined colours 
to 3D coordinates, that Unreal Engine can use to create a 3D world using procedural
content generation. 

----------------------------------------------------------------------------------------
PROJECT STAGE
----------------------------------------------------------------------------------------
* Raspberry Pi 5 Camera System                   - Version 1 - complete
* Algorithmic Computer Vision CMake Application  - Version 1 - complete
* Unreal Engine 5 Map creation game instance     - Proof of concept achieved - in progress

Overall: Proof of concept achieved
<img width="1346" height="483" alt="Screenshot 2026-05-02 105024" src="https://github.com/user-attachments/assets/30f71f6c-66d0-4660-b095-4c9d20a49034" />
----------------------------------------------------------------------------------------
                                 CORE TECHNOLOGIES
----------------------------------------------------------------------------------------

PYTHON 3                : FLASK, OPEN CV 2, PICAMERA 2

C++ 20 or 23            : CMAKE, OPEN CV 4, GCC

UNREAL ENGINE 5.4 - 5.7 : PCG content plugin, Geometry Script Plugin, Procedural Mesh Component Plugin,
                          (Realtime Mesh Component plugin if the "FirstDraftUE5RealtimeMeshActor" class is used)

File transfer           : WinSCP, Putty (, SSH, PSCP), Samba

All technologies used in this repository are either open source or available for free
under specific licensing conditions. Always check license agreements before using anyone else's code! 

----------------------------------------------------------------------------------------

This project was created as partial fulfilment of a BSc degree in
Computer Science with integrated Year in the Industry (G401) (Jan 2026 - May 2026) by Eva Martens.

The system combines aspects of computer vision and game code to create an application 
that allows users to prototype a 3D fantasy world map, without ever having used a game
engine or 3D modelling before, ideal for DMs leading table top RPGs or level prototyping. 

The idea is that the user creates a "2D map" on a 32x32 green Lego plate or similar building block material, that they
populate with 7-10 separate colored blocks. The computer vision system consists of a custom built camera system, 
which takes 4 HDR images to account for varying lighting conditions, and a CMake application to process
the information using OpenCV C++.

Example of an early testing plate
<img width="857" height="1017" alt="HDR_early_example_lego_plate" src="https://github.com/user-attachments/assets/27333de2-e6ea-4c0a-99c0-59b3ee070661" />

Example of in-development debug screenshot mapping colours to the procedural mesh inside unreal engine through PCG texture projection
<img width="1173" height="486" alt="ExampleColourMappingOntoProceduralMesh" src="https://github.com/user-attachments/assets/7e5b44a7-78a1-4fc3-9980-af8fdf44c38a" />

The CMake application uses an algorithmic approach applying thresholding, segmentation,  
feature and colour detection to locate the building plate in the images and then differentiates between bricks, using that data to create:
1) A PNG file mapping each pixels color to the relative location of the corresponding brick 
2) A .csv file containing pixel values of a height map generated with the image information
3) A persistent error log file, which the user can choose to reset at the start of the application or not
4) Consol error messages as well as warnings for various parts of the application so failure can be investigated

Note the plate has to be located on a black background. 

The CMake applications color detection was based of a literature review and heavily tested and achieves high overall
classification accuracy in varying light environments.

Unreal Engine 5 was chosen as medium for this project due to it's powerful graphical
capabilities and incredible PCG (procedural content generation) plugins, allowing 
each generated world to be unique. Unreal Engine 5 is free to use for students and educators. 

<img width="960" height="454" alt="Screenshot 2026-05-03 003606" src="https://github.com/user-attachments/assets/27b3c7fe-ff39-438a-a89b-e8708a981228" />

3D assets were taken from the FAB market place and licensed under the "Personal Project" common license.
They are NOT included in this repository. If you are re-building your own version of this project,
you will need to include your own assets. 

Please feel encouraged to check out and support the various creators on the FAB market place and their incredible work 
as it is people like them providing assets for projects like this, that make work like this possible!

Heads up: This is not a light weight application. Both computer vision and UE5 are resource intensive. 
The computer vision CMake application for example uses expensive logic for a custom implementation of a colour space (which significantly improved detection accuracy), which is why the system is not designed to run in parallel in the current version. 

For further guidance on hardware requirements please refer to:
**https://dev.epicgames.com/documentation/en-us/unreal-engine/hardware-and-software-specifications-for-unreal-engine**

All parts of this project can easily be converted into stand alone applications. Additionally, they are (mostly) platform independent and 
cross-compliable (though this does require accommodating the various compiler versions needed for CMake, UE5, and the Raspberry Pi 5 OS you are using).

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
TOOLS SETUP
----------------------------------------------------------------------------------------

Visual Studio Code is recommended as code editor with ReSharper (perfect for Unreal 
Engine 5 C++) as plug-in. Intellisense does not comprehend Unreal Engine 5 
specific project structure and use of C++ macros well. 

Thonny editor as interface for the python scripts on the Raspberry Pi 5 is sufficient and 
should come pre-installed through the operating system imaging tool.

Unreal Engine editor 5.7 was originally used to create this project. Unreal Engine 
5.4 or above should also work, as all PCG plugin content used was available for that version
of the engine. 

----------------------------------------------------------------------------------------
Video Demo of game logic
----------------------------------------------------------------------------------------
Spawning the procedural mesh with Blueprint (BP) Actor: 

https://github.com/user-attachments/assets/4513421f-a3d4-4266-a705-c57610de68d2

Setting mesh collisions, box collision around terrain for the PCG graph and the material

https://github.com/user-attachments/assets/40a71b87-bf25-4b4f-9f0e-27625c3b99d7

Using Data Assets and BP Structs to dynamically load the texture file at runtime for texture sampling

https://github.com/user-attachments/assets/e3e5d565-fadf-4542-a600-e199214062bb

Triggering the PCG Graph
(Comment: The warning on the node that turns spatial data into bounds is irrelevant, as the graph generation is triggered at runtime, when there are points available.)

https://github.com/user-attachments/assets/89d2a087-c90d-43ef-8ee0-08b0e93813f8

PCG Graph part 2: Texture projection and point filtering by colour to create biomes seperate

https://github.com/user-attachments/assets/0f5607c4-92bc-470f-8c9e-ca008362c204

End result: Game spawning terrain, sampling the surface, projecting points onto the surface and then executing logic per biom
(Sped up and cut into parts due to file size restrictions)

https://github.com/user-attachments/assets/1c35d1b3-d765-4049-8ac2-68d95e6e7b85


https://github.com/user-attachments/assets/6de279ba-5a36-45a1-bc04-062d8cb887a3


----------------------------------------------------------------------------------------
          Prerequisites - Camera system - Raspberry Pi 5 with camera module 3
----------------------------------------------------------------------------------------
If you do not want to build your own camera system to run this code that is fine. 
It should work fine with pictures taken on a phone that is HDR capable. Just set the corresponding
file path of where they will be located in your version of the OpenCVLegoMapScannerV1 class.

Setting up the Raspberry Pi with all updates is recommended for security purposes! 
Updating the system and all subsequent package downloads can take up to 2 days. 

The PiOS version based of Debian "Trixie" is the latest version out at time of writing,
but "Bookworm" is a lot more mature and better tested. 

If rebuilding this project the following setup is recommended for the raspberry pi:
- fully updated Pi OS (set up headless operation through imaging tool with PC on same network, enable "Raspberry Pi connect")
    - if you are using the latest version of PiOS through the official Raspberry Pi imaging tool, "sudo apt update && sudo apt upgrade -y" will result in many of the following steps possibly being omittable, but verify the following installations regardless:
        - install samba
        - install wsdd (or wsdd2 for Debian "trixie")
        - install python 3 (or latest)
        - install pip install as package manager
            - pip install open cv 2 (python version)
            - pip install picamera 2
            - pip install flask 
        - install rpi camera packages (previously known as libcamera)

If you want to connect wirelessly by setting up a flask server
- install ngrok or any other reverse-proxy service for easy & secure server connection
    - ngrok accounts are free, for more permanent architecture a paid account is recommend 
    - connection can use password protections (less secure) or mTLS (using custom certificates)
    - if the system only ever gets accessed form one location a static IP can be set as trusted connection
 
If you want to connect via ethernet cable with SSH (recommended)
- install putty on your pc or laptop
- change your network sharing adapter settings:
    - select ethernet -> properties -> Ipv4 settings -> declare a static IP address
    - Note: On windows 11, if you also use ICS (internet connection forwarding) the ethernet connections IP address may be changed
      
----------------------------------------------------------------------------------------
            Getting the images from the Raspberry Pi 5
----------------------------------------------------------------------------------------
There are several ways to get the images from the Raspberry Pi 5, including connecting 
via an ethernet cable. You do not have to set up a server for the RPI. 

WinSCP allows for simple file transfer between Windows PCs and the Linux based
PiOS, and can be setup easily, if you are running and able to access your PiOS 
(headless or otherwise).

------------------------------------------------------------------------------------------------
*IMPORTANT: NETWORKING SECURITY*

If you set up a server using NGrok - To set up the RPI as server, it's recommended to take a few steps first:
- Change the standard SSH port on your Pi form 20 to something else between 10000 - 60000
  This is so automated attacks don't have easy access to your Raspberry Pi
- Ensure you have a long, secure password on your Pi Connect account!
- If you intend to use (username:password) authentication with ngrok, ensure you require an adequately
  long password. Ngrok does protect you from DDos attacks.
- You can set up ngrok config files to only allow certain IP addresses or use mTLS (recommended)
- DO NOT run ngrok from the root user on your raspberry pi. It is not needed and just introduces
  unnecessary risk
- You do not need to disable your firewall or change router settings. Ngrok allows you to securely forward
  your port without needing to poke a whole in your firewall.

Make sure you have enough free memory on your raspberry pi 5 to take the HDR images and 
compress them while running the server scripts. 

Using TCP transfer on the Raspberry Pi 5 may mean you need to allow TCP file transfer 
for your chosen port via the command line first. 
TCP file transfer is stable, fast and reliable. The scripts in 
this repository send compressed image data. Because of this set up your Raspberry Pi 5 
should not have to deal with any additional junk data floating around. 

MAKE SURE TO CHECK MEMORY USAGE WHEN RUNNING THE SCRIPTS ON YOUR DEVICE THE FIRST TIME. 

Should you have trouble setting up the server, you can also flush the port through the 
command line but BE CAREFUL NOT TO ACCIDENTALLY FLUSH A PORT YOU ARE USING FOR ANYTHIG ELSE.

------------------------------------------------------------------------------------------------
**Important disclaimer:**
Please take anything you read here or anywhere else about cyber security with a grain of salt.
Do your own research referring to trusted sources and question anything that appears odd! 
Never use any code you do not understand. 

------------------------------------------------------------------------------------------------
Rebuilding the UE5 game
------------------------------------------------------------------------------------------------
Setup a project in UE5 and add the plugins mentioned above. 

It is recommended to set up a project in through the editor and also use the editor tools
to create the initial C++ classes. This is because this dramatically simplifies setting up the dependencies.

To implement your own version of the 3D environment builder, 
adapt "UE5_CPP_ProceduralMeshTerrain.h" and "*.cpp". 

There are several ways of improving on the current implementation, as it is, it spawns a procedural mesh component
reading the ".csv" file output by the "OpenCVLegoMapScannV1" class at runtime. 
Make sure the file path is amended to where your output .csv is located. 

The current scale is set up to spawn a perceptually 4000m x 4000m terrain. In the game version implemented in the 
video demos above a persistent water plane is set up in the level at world scale 7500 uu. 

The heightmap creation in "OpenCVLegoMapScannV1" is based on where you put "Dark blue" coloured bricks or objects.
This is mixed with random noise and then repeated up- and downscaled to create realistic looking terrain where 
the location of the Water is at the deepest parts of the map. 

To use the CPP class, inherit a Blueprint Actor of the given code and spawn it either in the level blue print
or by using another actor to call "spawnActor" node at "onBeginPlay()". 

At them moment the code only creates one Mesh Section for the Procedural Mesh. Ideally the spawning happens before the world
is visible, and "onTick()" is disabled for the Actor spawning the terrain. 

If you use a surface sampler to sample the Mesh, it is also recommended to doublecheck collisions are set! 
The surface sampler should sample "World Dynamic". 

------------------------------------------------------------------------------------------------
If you made it this far, great! Thank you for taking the time and I hope 
you found what you were looking for. 

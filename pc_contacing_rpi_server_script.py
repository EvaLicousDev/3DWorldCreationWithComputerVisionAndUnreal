import requests
import io
import zipfile

#-------------TOKENS HERE-----------------
# PING (server request)
# HRD_IMAGES_TCP (server request)
# NGROK_ADRESS (weblink to rasberry pi)
# USER, AUTH
# IMAGE_FODLER
# TIMEOUT (enough time in seconds for camera to run image capture script & send response)
#---------------TOKENS END-----------------

# This script uses the tokens above [REMOVED FROM GIT HUB] to connect to the rasberry pi which is set up as server to send images 
# The server is hosted on the pi and the corresponding port forwarded through ngrok acting as reverse proxy
# This allows us to use TCP file transfer on a specified port without needing to poke wholes in the firewall
# or otherwise mess with the router settings where the rasbery pi is connected 

# The images arrive in a compressed folder, making the file transer fast and secure with no additional lingering
# memory junk on the Pi server 

# We first ping the server to see if the Pi is reachable, and if successful we download the images

#-------------------------------------------------------------------------------------
# local text file for debugging 
server_log = open("image_request_log.txt", "a")
server_log.write("------------------------START SCRIPT-----------------------------")
server_log.write("About to contact Rasberry pi to get images")

try:

    #----------------------------
    # See if server is online
    response = requests.get(
        PING, 
        auth=(USER, AUTH),
        timeout=TIMEOUT
    )
    status = response.status_code

    #----------------------------
    # Process response 
    if status == SUCCESS:
        # debug info
        print(f"[Status] {status} - succefully connected to server using ngrok web link")
        print(f"[Request] requesting \'take hdr image\' script")
        server_log.write(f"[Status] RPI Camera answered ping with status {status} - serve online and contactable, images will be requested", "a")

        # make download request for hdr images sent in compressed folder
        imageRequest = requests.get(
        HDR_IMGS_TCP , 
        auth=(USER, AUTH),
        timeout=TIMEOUT
        )

        #-----------------------------
        #Process hdr image response
        sendImages = imageRequest.status_code
        hdr_image_zip = imageRequest.content

        if sendImages == SUCCESS:
            image_bytes = imageRequest.content

            # debug info
            print(f"[Status] {sendImages} - sucessfully requested Images") 
            print(f"[Success] - Received {len(image_bytes)} bytes.")
            server_log.write(f"[File] compressed images {len(image_bytes)}", "a")

            # extracting images
            with zipfile.ZipFile(io.BytesIO(hdr_image_zip)) as hdr_zip:
                hdr_zip.extractall(IMAGE_FOLDER)
                print(f{"[File] extracted all images to {IMAGE_FOLDER}"))
        else:
            # debug info
            print(f"Failed with status: {sendImages} - could not get images.")
        #-----------------------------

    else: 
        #server ping failed
        print(f"[Status] {response.text} - could not reach server")
        server_log.write(f"[Status] {response.text} - could not reach server", "a")

    #---------------------------------------------------------------------------------------
except requests.exceptions.Timeout:
    print(f("ERROR: Sever tool longer than {TIMEOUT} seconds to respond with information. Does the image request script take too long to process?"))
    server_log.write(f"[ERROR] [Timeout] set timout for request for TCP file transer was triggered", "a")
except requests.exceptions.ConnectionError:
    print(f("ERROR: There is a problem with the connection to {NGROK_ADRESS}. Please check adress on RPI"))
    server_log.write(f"[ERROR] [Connection Error] There is a problem with the connection to {NGROK_ADRESS}. Please check adress output on Rasberry Pi", "a")
except Exception as e:
    print(f"ERROR: An issue occured. Exception: {e} - please check servers\' logs and local outputs")
    server_log.write(f"[ERROR] [Other] unplanned error occured: {e}. Please check all logs", "a")
#---------------------------------------------------------------------------------------
server_log.write("-----------------------END SCRIPT------------------------------")
server_log.close()

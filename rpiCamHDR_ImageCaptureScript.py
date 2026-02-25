#Script used for capturing images every hour 
import time
import json
import cv2
import numpy as np

from picamera2 import Picamera2
from pathlib import Path
from datetime import datetime

#This code is based of the Rasberry pi documentation for creating HDR images with the mertens merge algorithm
#Also view "https://pip-assets.raspberrypi.com/categories/652-raspberry-pi-camera-module-2/documents/RP-008156-DS-2-picamera2-manual.pdf?disposition=inline"

SECONDS_PER_HOUR = 3600
RATIO = 3.0
OUTPUT_DIR = Path("/home/*/capturedImages/Set2")

#First the camera is prepared
#initialise instance
picam2 = Picamera2()
camera_configuration = picam2.create_video_configuration(main={"size":(1920,1080)})
picam2.configure(camera_configuration)
picam2.start()

#Run for a second to get exposure time avarage (source https://github.com/raspberrypi/picamera2/blob/main/examples/opencv_mertens_merge.py)
time.sleep(2)
metadata = picam2.capture_metadata()
exposure_normal = metadata["ExposureTime"]
gain_normal = metadata["AnalogueGain"] * metadata["DigitalGain"]
picam2.stop()
controls = {"AfMode":2, #continuous autofocus
            "ExposureTime": exposure_normal,
            "AnalogueGain": gain_normal,
            "AwbEnable": True,
            "Saturation":1.3, #increasing saturation and contrast for better results
            "Contrast":1.1}
capture_config = picam2.create_preview_configuration(main={"size": (1920,1080),
                                                           "format": "RGB888"},
                                                     controls=controls)
picam2.configure(capture_config)

def capture_hdr_set():
    #Note if this script is run on the hour there will be issues in naming!
    timestamp = datetime.now().strftime("%d%m_%H%M%S")
    print(f"Capturing HDR set: {timestamp}")
    time.sleep(2)
    
    picam2.set_controls({"ExposureTime": exposure_normal, "AnalogueGain": gain_normal})
    time.sleep(0.5)
    picam2.start()
    normal = picam2.capture_array()
    picam2.stop()
    print(f"-captured normal")


    exposure_short = int(exposure_normal / RATIO)
    picam2.set_controls({"ExposureTime": exposure_short, "AnalogueGain": gain_normal})
    time.sleep(0.5)
    picam2.start()
    short = picam2.capture_array()
    picam2.stop()
    print(f"-captured short")

    exposure_long = int(exposure_normal * RATIO)
    picam2.set_controls({"ExposureTime": exposure_long, "AnalogueGain": gain_normal})
    time.sleep(0.5)
    picam2.start()
    long = picam2.capture_array()
    picam2.stop()
    print(f"-captured long")
    print(f"        EXPOSURE SHORT: {exposure_short}")
    print(f"        EXPOSURE NORM : {exposure_normal}")
    print(f"        EXPOSURE LONG : {exposure_long}")
    
    filename = OUTPUT_DIR / f"hdr_{timestamp}.jpg"
    imagename_hdr = filename
    imagename_normal = OUTPUT_DIR / f"normal_{timestamp}.jpg"
    print(f"-set filenames")
    
    merge = cv2.createMergeMertens()
    merged = merge.process([short, normal, long])
    merged = np.clip(merged * 255, 0, 255).astype(np.uint8)
    cv2.imwrite(f"{imagename_normal}", normal)
    cv2.imwrite(f"{imagename_hdr}", merged)
    print(f"-merged images in capture array and saved them as {imagename_normal} and {imagename_hdr}")
    
        
try:
    while True:
        start_time = time.time()
        capture_hdr_set()
        elapsed = time.time() - start_time
        sleep_time = max(0, SECONDS_PER_HOUR - elapsed)
        time.sleep(sleep_time)
        
except KeyboardInterrupt:
    print("Stopping...")
finally:
    picam2.stop()


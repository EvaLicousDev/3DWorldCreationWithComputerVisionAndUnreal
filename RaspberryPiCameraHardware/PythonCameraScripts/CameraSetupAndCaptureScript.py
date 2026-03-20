import time
import cv2
import numpy as np
from CameraSetupFunctions import HDR_Img_Camera

from picamera2 import Picamera2
from pathlib import Path
from datetime import datetime

## This script should run when the application request a capture array
## We weant to send 8 images with various lenghths of exposure

try:
    #while True:
    #start the camera
    # 1) creates preview so system can be set up
    # 2) creates 4 capture modes based on current light conditions
    camera = HDR_Img_Camera()
    camera.startCam()
    print("Setup complete")

    # capture 8 images, 4 HDR, 4 normal
    camera.capture_hdr_set()
except KeyboardInterrupt:
    print("Finishing up...")
finally:
    print("Stopping Camera Application")
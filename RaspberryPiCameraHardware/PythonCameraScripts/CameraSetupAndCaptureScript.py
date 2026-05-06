import time
import cv2
import numpy as np
from RPICameraModule import HDR_Img_Camera

from picamera2 import Picamera2
from pathlib import Path
from datetime import datetime

## This script should run when the application request a capture array
## We weant to send 8 images with various lengths of exposure

try:
    camera = HDR_Img_Camera()
    print("Setup complete")

    # 4 HDR
    camera.capture_hdr_set()
    
except KeyboardInterrupt:
    print("Input interrupted application")
finally:
    print("Stopping Camera Application")

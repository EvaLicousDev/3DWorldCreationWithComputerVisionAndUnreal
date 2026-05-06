import time
import cv2
import numpy as np

from picamera2 import Picamera2
from pathlib import Path
from datetime import datetime

SECONDS_PER_HOUR = 3600
FRAM_VALUE_ADJUSTMENT_RATIO = 3.0
OUTPUT_DIR = Path("/home/evaliciousdev/cvImages") #shared folder for samba server acessible by windows

__name__ = "camera_setup_module"

DAYLIGHT      = "Daylight"
TWILIGHT      = "Twilight"
HIGHLIGHT     = "High Light"
SUPERLOWLIGHT = "Super LowLight"
imageNames    = [DAYLIGHT, TWILIGHT, HIGHLIGHT, SUPERLOWLIGHT]

#------------------------------------------------------------------------------------
# ----------------------------------CAMERA SETTINGS----------------------------------
#------------------------------------------------------------------------------------
# These are 4 predefined capture modes which capitalise of the pi camera module 3
# Being able to change settings on the fly. The general idea is that different
# light conditions require different exposure levels. While these could be tweaked
# manually for every use, we have predefined settings for the application based
# on experimentation.
#
# The following principles apply to the pre-defined settings:
# - If there is glards we need to lower the image birghtness & set the auto white balange to flurecent or cloud
#     Auto White blanance - AwbMode:
#     0 = Auto, 1 = Tungesten, 2 = Flurecent, #3 = Indoor, #4 = Daylight, 5 = Cloudy
# - Colour Gain messes with the actual RGB value prediction but can be used in very low light for accaptable results
# - Contrast & Stauration are slightly turned up in all modes: The darker the higher these values are
# - Auto focus is always on
# - Increased exposure time is recommended for low light
#------------------------------------------------------------------------------------

class Cam_Controls:
    __name__ = "Camera_Controls"
    
    exposure_normal = 10000 
    gain_normal     = 1.0
    controls        = None
    
    #------------------------------------------------------------------------------------
    def get_controls(self):
        if self.controls is not None:
            return self.controls
        else:
            raise("Cam_controls instance is trying to return empty controls. This is not valid. Function called to soon?")
    
    #------------------------------------------------------------------------------------
    def __init__(self, exposure, gain):
        exposure_normal = exposure
        gain_normal = gain
        print("Creating new camera instance")
        return
    
    #------------------------------------------------------------------------------------
    def __repr__(self) -> str:
        for data in controls:
            print(std(data))
        return f"{type(self).__name__}()"
    
    #------------------------------------------------------------------------------------
    def set_capture_modes(self):
        # any well lit area with causing not too much glare
        controls_daylight = {"AfMode":2, #continuous autofocus
                            "AeEnable": False, #manually control exposure and gain
                            "ExposureTime": self.exposure_normal,
                            "AnalogueGain": self.gain_normal,
                            "ColourGains":(0.0,0.0),
                            "AwbEnable": True,
                            "AwbMode": 5, 
                            "Contrast":1.3,
                            "Saturation":1.1,
                            "Brightness":-0.3
                            }
        # exposed to super bright light
        # this cannot completly offse glare, but does help a lot 
        controls_bright_light = {"AfMode":2, #continuous autofocus
                            "AeEnable": False, #manually control exposure and gain
                            "ExposureTime": int(self.exposure_normal*0.9),
                            "AnalogueGain": int(self.gain_normal*1.2),
                            "ColourGains":(0.0,0.0),
                            "AwbEnable": True,
                            "AwbMode": 2, #Flurecent
                            "Contrast":1.3,
                            "Saturation":1.2,
                            "Brightness":-0.4
                            }
        # evenings and indoors 
        controls_twilight = {"AfMode":2, #continuous autofocus
                            "AeEnable": False, #manually control exposure and gain
                            "ExposureTime": int(self.exposure_normal*1.2),
                            "AnalogueGain": int(self.gain_normal),
                            "AwbEnable": False, #Colour gain is mutually exclusive with Awb modes
                            "Contrast":1.5,
                            "Saturation":1.3,
                            "Brightness":-0.2
                            }
        
        #this one will result in very over exposed images in most light, but works well in darker setting
        controls_super_low_light = {"AfMode":2, #continuous autofocus
                            "AeEnable": False, #manually control exposure and gain
                            "ExposureTime": int(self.exposure_normal*1.5),
                            "AnalogueGain": int(self.gain_normal*1.2),
                            "AwbEnable": False, #Colour gain is mutually exclusive with Awb modes
                            "ColourGains":(1.2,1.2),
                            "Contrast":1.3,
                            "Saturation":1.5,
                            "Brightness":0.7
                            }
        self.controls = [controls_daylight, controls_twilight, controls_super_low_light, controls_bright_light]
        
#------------------------------------------------------------------------------------
#--------------------------------HDR image creator-----------------------------------
#------------------------------------------------------------------------------------
# HDR images are simply images which are merged from multiple frames takes at various
# levels of exposure. It leads to more vibrant colours which is why we use it here to
# combar both low light and reflections
#------------------------------------------------------------------------------------
class HDR_Img_Creator:
    __name__ = "HDR Image Creator"
    
    capture_mode   = None
    capture_arrays = None
    image_name     = None
    normal_name    = None
    metadata_name  = None
    merge_modes    = None
    
    output_image_HDR    = None
    output_image_normal = None
    
    #------------------------------------------------------------------------------------
    def get_normal(self):
        return self.output_image_normal
    #------------------------------------------------------------------------------------
    def get_hdr(self):
        return self.output_image_HDR
    #------------------------------------------------------------------------------------
    def set_up_merge_modes(self, short, short2, normal, long, long2, long3):
        merge_super_low_light = [short2, normal, normal, long, long2, long3]
        merge_twilight        = [short, short2, normal, normal, long]
        merge_daylight        = [short, short2, normal, normal, long, long2]
        merge_high_light      = [short, short2, short2, normal, long]
        self.merge_modes = {DAYLIGHT:  merge_daylight ,
                             TWILIGHT:  merge_twilight,
                             HIGHLIGHT: merge_high_light,
                             SUPERLOWLIGHT: merge_super_low_light }
    #------------------------------------------------------------------------------------    
    def merge_arrays(self, normal, capture_mode, filename):
        allInit = self.capture_arrays is not None and self.capture_mode is not None and self.image_name is not None and self.normal_name is not None and self.metadata_name is not None
        if allInit is not None:
            merging_mode = self.merge_modes.get(capture_mode)
            #merge with Mertens merge algorithm and represent RGB colours as uints
            merger = cv2.createMergeMertens()
            merged_hdr = merger.process(merging_mode)
            self.output_image_HDR    = np.clip((merged_hdr * 255), 0, 255).astype(np.uint8)
            self.output_image_normal = normal
            
            cv2.imwrite(f"{filename}", self.output_image_HDR)
            #cv2.imwrite(f"normal_{capture_mode}", normal)
        else:
            raise("Trying to merge images, but capture array is empty. Capture mode: {self.capture_mode} Image name: {self.image_name}")
    
    #------------------------------------------------------------------------------------
    def __init__(self, array1, array2, array3, array4, array5, array6, filename, normal_name, metadata_name, capture_mode):
        self.capture_arrays = {"Shortest": array1,
                          "Short": array2,
                          "Normal": array3,
                          "Long": array4,
                          "Longer": array5,
                          "Longest": array6}
        self.image_name = OUTPUT_DIR / f"hdr_{capture_mode}.jpg"
        self.normal_name = normal_name
        self.metadata_name = metadata_name
        self.capture_mode = capture_mode
        self.set_up_merge_modes(array1, array2, array3, array4, array5, array6)
        
        print("Creating HDR image...")
        self.merge_arrays(array3, self.capture_mode, self.image_name)
        return
    
    def __repr__(self) -> str:
        return f"{type(self).__name__}(image_name={self.image_name}, meta_data={self.metadata_name})"
        
#------------------------------------------------------------------------------------
#--------------------------------Pi Camera Class--------------------------------------
#------------------------------------------------------------------------------------
## Class which sets up the pi camera
## This uses funcitonality described in https://grobotronics.com/images/companies/1/content_processor/PDF/picamera2-manual.pdf?srsltid=AfmBOor8sF7pTJHrAk-DIa0nR6_vYOHC9IgaNfGq8DhLudSYAWNmkT7a
class HDR_Img_Camera:
    __name__ = "HDR_Camera_Class"
        
    camera_instance = Picamera2()
    configuration = camera_instance.create_video_configuration(main={"size":(1920,1080), "format":"RGB888"})
    metadata = None
    controls = None
    merge_mode = None
    normal_exposure = None
    normal_analgue_gain = None
    
    current_normal_images = {DAYLIGHT:None ,
                             TWILIGHT: None,
                             HIGHLIGHT: None,
                             SUPERLOWLIGHT: None }
    current_hdr_images = {DAYLIGHT:None ,
                          TWILIGHT: None,
                          HIGHLIGHT: None,
                          SUPERLOWLIGHT: None }
    #------------------------------------------------------------------------------------
    def get_output_images(self):
        output_array = []
        for image in self.current_hdr_images:
            output_array.append(image)
        for image in self.current_normal_images:
            output_array.append(image)
        return output_array
    
    #------------------------------------------------------------------------------------
    # The camera module 3 adjusts to environmental conditions automatically unless explicitely told otherwithse
    # The following function capitalises of this to gain the recommended values for normal image capture
    # and then creates an instance of the Cam_controls class
    def startCam(self):
        
        # start and show camera so view can be adjusted
        self.camera_instance.start(show_preview=True)
        self.camera_instance.title_fields = ["ExposureTime", "AnalogueGain"]
        
        #set up remaining data
        time.sleep(2) #give the camera time to auto adjust to light conditions
        self.metadata = self.camera_instance.capture_metadata()
        time.sleep(1)
        self.normal_exposure = self.metadata["ExposureTime"]
        self.normal_analogue_gain = self.metadata["AnalogueGain"] * self.metadata["DigitalGain"]
        
        self.controls = Cam_Controls(exposure = self.normal_exposure, gain = self.normal_analogue_gain)        
        self.controls.set_capture_modes()
        
        #Wait for user to set up camera with preview
        input("Pi camera set up controls and capture modes. Press enter to continue.")
        self.camera_instance.stop_preview()
        self.camera_instance.stop()
        
    #------------------------------------------------------------------------------------
    def capture_hdr_set(self):
        self.camera_instance.configure(self.configuration)
        timestamp = datetime.now().strftime("Fram{frameNumber}_%H_%M_%S")
        metadata_timestamp = datetime.now().strftime("%d_%m_%H_M%")
        print(f"Capturing: {timestamp}")
        time.sleep(1)
        
        # iterate over capture modes and create HDR image
        # we turn camera on and off and pause when controls are updated so the
        # hardware of the sensor & lense can adjust
        modeIndex = 0
        for control_set in self.controls.get_controls():
            self.camera_instance.set_controls(control_set)
            print(f"Camera Settings: {self.camera_instance.stream_configuration('main')}")
            
            shortest_exposure    = int(self.normal_exposure / FRAM_VALUE_ADJUSTMENT_RATIO)
            short_exposure       = int(shortest_exposure*2)
            long_exposure        = int(self.normal_exposure * FRAM_VALUE_ADJUSTMENT_RATIO)
            longer_exposure      = int((self.normal_exposure * FRAM_VALUE_ADJUSTMENT_RATIO) + short_exposure)
            longest_exposure     = int(self.normal_exposure * FRAM_VALUE_ADJUSTMENT_RATIO * FRAM_VALUE_ADJUSTMENT_RATIO)
            time.sleep(0.5)
            
            #normal
            frameNumber = 1 
            self.camera_instance.start()
            normal = self.camera_instance.capture_array()
            self.camera_instance.stop()
            print(f" - captured Frame {frameNumber}")
            
            #shortest
            frameNumber = 2
            self.camera_instance.set_controls({"ExposureTime" : shortest_exposure})
            time.sleep(0.5)
            self.camera_instance.start()
            shortest = self.camera_instance.capture_array()
            print(shortest.shape)
            self.camera_instance.stop()
            print(f" - captured Frame {frameNumber}")
            
            #short 
            frameNumber = 3
            self.camera_instance.set_controls({"ExposureTime" : short_exposure})
            time.sleep(0.5)
            self.camera_instance.start()
            short = self.camera_instance.capture_array()
            self.camera_instance.stop()
            print(f" - captured Frame {frameNumber}")
            
            #long
            frameNumber = 4
            self.camera_instance.set_controls({"ExposureTime" : long_exposure})
            time.sleep(0.5)
            self.camera_instance.start()
            long = self.camera_instance.capture_array()
            self.camera_instance.stop()
            print(f" - captured Frame {frameNumber}")
            
            #longer
            frameNumber = 5
            self.camera_instance.set_controls({"ExposureTime" : longer_exposure})
            time.sleep(0.5)
            self.camera_instance.start()
            longer = self.camera_instance.capture_array()
            self.camera_instance.stop()
            print(f" - captured Frame {frameNumber}")
            
            #longest
            frameNumber = 6
            self.camera_instance.set_controls({"ExposureTime" : longest_exposure, "AnalogueGain": (self.normal_analogue_gain/2)})
            time.sleep(0.5)
            self.camera_instance.start()
            longest = self.camera_instance.capture_array()
            self.camera_instance.stop()
            print(f" - captured Frame {frameNumber}")
            
            filename = OUTPUT_DIR / f"hdr_{timestamp}.jpg"
            metadata_filename = OUTPUT_DIR / f"hdr_{timestamp}_metadata.txt"
            imagename_normal = OUTPUT_DIR / f"normal_{timestamp}.jpg"
            
            creator = HDR_Img_Creator(shortest,
                                      short,
                                      normal,
                                      long,
                                      longer,
                                      longest,
                                      filename,
                                      imagename_normal,
                                      metadata_filename,
                                      imageNames[modeIndex])
            #add to output
            new_hdr = creator.get_hdr()
            new_normal = creator.get_normal()
            self.current_normal_images[imageNames[modeIndex]] = new_normal
            self.current_hdr_images[imageNames[modeIndex]] = new_hdr
            print( f"---> captured and saved {imageNames[modeIndex]} for output" )
            modeIndex += 1
            
        
    def __init__(self):
        print("Creating new camera instance")
        self.startCam()
        return
    
    def __repr__(self) -> str:
        return f"{type(self).__name__}(normal_expoure={self.normal_expoure}, normal_analogue_gain={self.normal_analogue_gain})"

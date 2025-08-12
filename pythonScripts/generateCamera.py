
import numpy as np
import random 
import sys
from pythonLib.scenGenLib import*
import argparse

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="create camera")
    parser.add_argument("--generate_camera", action="store_true", help="Flag to indicate camera generation")
    parser.add_argument("--camera_location", nargs=3, type=float, help="Camera location coordinates (x y z)")
    parser.add_argument("--look_at", nargs=3, type=float, help="Camera look-at coordinates (x y z)")
    parser.add_argument("--camera_file", type=str, help="Path to generated camera JSON file")    
    parser.add_argument("--detector_width", type=float, help="Detector width in mm")
    parser.add_argument("--detector_height", type=float, help="Detector height in mm")


    args = parser.parse_args()
    camera_output_file = args.camera_file
    camear_location = args.camera_location
    camear_look = args.look_at
    random_list = [random.uniform(-5.0, 5.0) for _ in range(3)]
    camear_location[0] = camear_location[0] + random_list[0]
    camear_location[1] = camear_location[1] + random_list[1]
    camear_location[2] = camear_location[2] + random_list[2]
    camear_look[0] = camear_look[0] + random_list[0]
    camear_look[1] = camear_look[1] + random_list[1]
    generate_camera_json(camera_position=camear_location,look_at_point=camear_look,filename=camera_output_file,detector_width=args.detector_width,detector_height=args.detector_height)
import sys
from pythonLib.lenLib import*
import argparse

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Process photon data and compute detector distances.")
    parser.add_argument("--input_file", help="Path to input .txt or .h5 file")
    parser.add_argument("-o", "--output_file", help="Optional output file name")

    parser.add_argument("--fov", help="feild of view of the image formation", type=float)
    parser.add_argument("--image_height", type=int, help="Height of the image in pixels")
    parser.add_argument("--image_width", type=int, help="Width of the image in pixels")
    
    args = parser.parse_args()
    input_file_path = "./test_raw.h5"
    fov = 50
    image_height = 500
    image_width = 500
    
    if args.input_file:
        input_file_path = args.input_file
        input_fov, input_height, input_width = read_file_parameter(input_file_path) 
        fov = input_fov
        image_height = input_height
        image_width = input_width
    # Generate default output filename if not provided
    if args.output_file:
        output_file_name = args.output_file
    else:
        base_name = os.path.basename(input_file_path)
        output_file_name = os.path.splitext(base_name)[0] + "_len.h5"

    if args.fov:
        fov = args.fov

    if args.image_height:
        image_height = args.image_height

    if args.image_width:
        image_width = args.image_width


    # Initialize detector
    mydetector = detector(0.01, fov, image_width, image_height)

    # Read and process photons
    photons, failed_lines = read_raw_data(input_file_path)
    for photon in photons:
        mydetector.photon_to_dector(photon)
    mydetector.output_to_file(output_file_name)

    # Print debug output
    print(f"📏 Min Distance: {mydetector.minDistance}")
    print(f"📏 Max Distance: {mydetector.maxDistance}")
    print(f"✅ Output File: {output_file_name}")

        
         
    




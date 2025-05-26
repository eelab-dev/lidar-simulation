import sys
from pythonLib.formImageLib import *
import argparse


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Process photon data and generate a histogram-based depth image.")

    parser.add_argument("--input_file", help="Path to input photon .h5 file")
    parser.add_argument("--output_image", help="Optional output image file name (e.g., depth_image.png)")

    parser.add_argument("--output_file", help="Optional output h5 file name (e.g., depth_image.png)")
    parser.add_argument("--bin_number", type=int, help="Number of histogram bins")
    parser.add_argument("--min_range", type=float, help="Minimum range value for histogram")
    parser.add_argument("--bin_width", type=float, help="The width of each bin")

    input_file_path = './positiveResult/histogram/test1_len.txt'
    # extract the name of the file generated a new file with the same name
    output_image_name = input_file_path.split('/')[-1]
    output_image_name = output_image_name.split('.')[0]
    output_image_name = output_image_name + '_depth.png'

    range_min = 500
    bin_width = 80 
    args = parser.parse_args()
    input_bin_number = 35

     # Override with CLI args
    if args.input_file:
        input_file_path = args.input_file

    if args.bin_number:
        input_bin_number = args.bin_number

    if args.min_range:
        range_min = args.min_range

    if args.bin_width:
        bin_width = args.bin_width

    if args.output_image:
        output_image_name = args.output_image

    range_max = range_min + input_bin_number * bin_width
    myRange = [range_min,range_max]
    pixels, image_width, image_heigh = decode_file(input_file_path)
    # print(pixels[200][300])
    
    # image = form_image(pixels, image_heigh,image_width)
    image, illegal_photon, stamped_histogram, stamped_collosioin = form_histogram_image(pixels, image_heigh,image_width,bin_number=input_bin_number,range_distance=myRange)
    save_image(image,None,output_image_name,distance_range=myRange)
    if args.output_file:
        outputFile = args.output_file
        save_histogram_to_h5(outputFile, stamped_histogram, stamped_collosioin ,range_min, range_max, image_width, image_heigh, input_bin_number)









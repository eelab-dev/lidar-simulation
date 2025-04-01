from pythonLib.formImageLib import *
from pythonLib.postProcessLib import *
import argparse


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Process photon data and generate a histogram-based depth image.")
    parser.add_argument("--positive_input_file", help="Path to positive_input photon .h5 file")
    parser.add_argument("--negative_input_file", help="Path to negative_input photon .h5 file")
    parser.add_argument("--output_image", help="Path to output image .png")


    args = parser.parse_args()
    if args.positive_input_file and args.negative_input_file and args.output_image:
    
        postive_stamped_histogram, positive_stamped_collosion, positive_metadata = read_histogram_from_h5(args.positive_input_file)
        negative_stamped_histogram,negative_stamped_collosion,negative_metadata = read_histogram_from_h5(args.negative_input_file)
        image_height,image_width,range_max,range_min,bin_number = \
        positive_metadata["image_height"], positive_metadata["image_width"],positive_metadata["range_max"], positive_metadata["range_min"], positive_metadata["bin_number"]
        positive_distance_image = from_distance_from_histogram(positive_stamped_collosion,image_height,image_width,range_max,range_min,bin_number)
        negative_distance_image = from_distance_from_histogram(negative_stamped_collosion,image_height,image_width,range_max,range_min,bin_number)
        difference_image = positive_distance_image - negative_distance_image
        display_image(abs(difference_image), rectangle=None, imageFileName = args.output_image, distance_range = [0,500])
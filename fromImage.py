import sys
from pythonLib.formImageLib import *



if __name__ == "__main__":
    input_file_path = './positiveResult/histogram/test1_len.txt'
    # extract the name of the file generated a new file with the same name
    output_image_name = input_file_path.split('/')[-1]
    output_image_name = output_image_name.split('.')[0]
    output_image_name = output_image_name + '_depth.png'

    output_matlab_file_name = input_file_path.split('/')[-1]

    if (len(sys.argv) == 3):
        input_file_path = sys.argv[1]
        output_image_name = sys.argv[2]

    pixels, image_width, image_heigh = decode_file(input_file_path)
    print(pixels[200][300])
    myRange = [2000,3200]
    # image = form_image(pixels, image_heigh,image_width)
    image, illegal_photon, stamped_histogram, stamped_collosioin = form_histogram_image(pixels, image_heigh,image_width,bin_number=40,range_distance=myRange)
    display_image(image,None,output_image_name,distance_range=myRange)









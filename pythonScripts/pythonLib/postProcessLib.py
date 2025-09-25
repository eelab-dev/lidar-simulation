
import numpy as np




def from_distance_from_histogram(histogram,image_height,image_width,range_max,range_min,bin_number):

    distance_image = np.zeros((image_height, image_width), dtype=np.float32)

    bin_width = (range_max - range_min)/bin_number
    for i in range(image_height):
        for j in range(image_width): 

            max_bin_index = np.argmax(histogram[i, j])
            if histogram[i, j, max_bin_index] > 0:
                distance_image[i, j] = range_min + (max_bin_index + 0.5) * bin_width
    return distance_image



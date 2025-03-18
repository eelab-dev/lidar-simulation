import lens
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import matplotlib.patches as mpatches
import sys
import os
import scipy.io
import h5py

depthmap_colors = np.array([
    [0.0, 0.0, 0.5, 1.0],  # Dark Blue
    [0.0, 0.0, 1.0, 1.0],  # Blue
    [0.0, 0.5, 1.0, 1.0],  # Sky Blue
    [0.0, 1.0, 1.0, 1.0],  # Cyan
    [0.0, 0.75, 0.5, 1.0], # Turquoise
    [0.0, 1.0, 0.5, 1.0],  # Spring Green
    [0.0, 1.0, 0.0, 1.0],  # Green
    [0.5, 1.0, 0.0, 1.0],  # Light Green
    [0.8, 1.0, 0.2, 1.0],  # Yellow-Green
    [1.0, 1.0, 0.0, 1.0],  # Yellow
    [1.0, 0.84, 0.0, 1.0], # Gold
    [1.0, 0.65, 0.0, 1.0], # Orange
    [1.0, 0.55, 0.0, 1.0], # Dark Orange
    [1.0, 0.3, 0.0, 1.0],  # Vermilion
    [1.0, 0.0, 0.0, 1.0],  # Red
    [0.85, 0.0, 0.0, 1.0], # Crimson
    [0.55, 0.0, 0.0, 1.0]  # Dark Red
], dtype=np.float32)

# Normalize the colors for LinearSegmentedColormap
positions = np.linspace(0, 1, depthmap_colors.shape[0])
colors = [(pos, color) for pos, color in zip(positions, depthmap_colors)]


def decode_file(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()
    # print(lines[0])
    resolution_width = int(lines[0])
    resolution_height = int(lines[1])    
    pixel_output_array = [[[] for i in range(resolution_height)] for j in range(resolution_width)]
    line_index = 2
    for i in range(resolution_width):
        for j in range(resolution_height):
            line = lines[line_index]
            line = line[1:-2]
            line = line.split('),(')
            for item in line:
                item = item.split(',')
                if len(item) == 4:
                    # if int(item[3].strip(')')) == 2:
                    pixel_output_array[i][j].append((float(item[2]), int(item[3].strip(')'))))
            line_index += 1
    return pixel_output_array, resolution_width, resolution_height

def get_histogram(pixels, rectangle):

    x = rectangle[0]
    y = rectangle[1]
    width = rectangle[2]
    height = rectangle[3]
    result = []
    for i in range(width):
        for j in range(height):
            for k in range(len(pixels[x+i][y+j])):
                result.append(pixels[x+i][y+j][k][0])
    return result


def form_image(pixels, image_width, image_heigh):
    # form the image from the pixels
    image = np.zeros((image_width, image_heigh))

    for i in range(len(pixels)):
        for j in range(len(pixels[0])):
            if len(pixels[i][j]) > 0:
                for k in range(len(pixels[i][j])):
                    image[i][j] += (pixels[i][j][k][0])
                image[i][j] = image[i][j] / len(pixels[i][j])
            else:
                image[i][j] = 0
    return image


def display_image(image, rectangle=None, imageFileName = "./"):
    # display the image
    # Create the colormap
    depthmap = mcolors.LinearSegmentedColormap.from_list('depth_cmap', colors, N=256)
    cmap = depthmap
    cmap.set_bad(color='black')
    # show image using matplotlib

    show_image = np.ma.masked_where((image == 0), image)

    
    if rectangle is not None:
        fig, ax = plt.subplots()
        rect = mpatches.Rectangle((rectangle[0], rectangle[1]), rectangle[2], rectangle[3], fill=False, edgecolor='red', linewidth=1, facecolor='none')
        ax.add_patch(rect)

    # Ensure the directory exists
    os.makedirs(os.path.dirname(imageFileName), exist_ok=True)
    plt.imshow(show_image,cmap=cmap ,vmin = 1050*2,vmax=1500*2 ,interpolation='nearest')
    plt.colorbar()
    plt.savefig(imageFileName, format='png', dpi=600, transparent=True)
    plt.show()

def rotate_90_counterclockwise(matrix):
    return [list(row) for row in zip(*matrix)][::-1]


def save_histogram_to_matlab(combined_array, filename="./test"):


    histogram = [[ [pair[0] for pair in entry] for entry in row] for row in combined_array]
    collosion = [[ [pair[1] for pair in entry] for entry in row] for row in combined_array]
    histogram_file_name = filename + '_histogram.mat'
    collosion_file_name = filename + '_collosion.mat'

    os.makedirs(os.path.dirname(histogram_file_name), exist_ok=True)
    os.makedirs(os.path.dirname(collosion_file_name), exist_ok=True)
    # Save to MATLAB .mat file
    scipy.io.savemat(histogram_file_name, {'histogram': histogram})
    scipy.io.savemat(collosion_file_name, {'collosion': collosion})
  

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

    output_file_name =  '_'.join(input_file_path.rsplit('_', 1)[:-1]) + '_'
     




    pixels, image_width, image_heigh = decode_file(input_file_path)
    pixels_rot = rotate_90_counterclockwise(pixels)
    # rectangular = [310,345,20,20]
    # image = form_image(pixels, image_width, image_heigh)

    save_histogram_to_matlab(pixels_rot,output_file_name)
    # print(pixels_rot[299][299])
    image = form_image(pixels_rot, image_heigh,image_width)
    display_image(image,None,output_image_name)




# histogram = get_histogram(pixels, rectangular)
# print(len(histogram))
#display the histogram
# plt.hist(histogram,40)
# plt.yscale('log')
# plt.savefig('noiseHistogram.png', format='png', dpi=600, transparent=True)
# plt.show()





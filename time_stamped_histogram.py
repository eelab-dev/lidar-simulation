import scipy.io as sio
import numpy as np
import sys
import h5py
import cv2
import matplotlib.pyplot as plt
import matplotlib.colors as m
import os
import matplotlib.colors as mcolors

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

def load_and_process_matrices(file1, file2):

    range_min = 1000 * 2
    range_max = 1600 * 2
    bin_size = 20
    bin_width = (range_max - range_min) / bin_size
    # Load the .mat files
    data1 = sio.loadmat(file1)
    data2 = sio.loadmat(file2)

    # Assuming the matrices are stored with a known variable name, e.g., 'matrix'
    distance_matrix = data1.get('histogram')
    collosion_matrix = data2.get('collosion')
    print(distance_matrix[87][80])

    # Ensure the matrices have the same shape
    if distance_matrix.shape != collosion_matrix.shape:
        print("Error: Matrices 'histogram' and 'collosion' must have the same shape.")
        return None
    
    height,width = distance_matrix.shape[0],distance_matrix.shape[1]
    stamped_histogram = np.zeros((height, width, bin_size), dtype=int)
    stamped_collosion = np.zeros((height, width, bin_size), dtype=float)
    for i in range(height):
        for j in range(width):
            if isinstance(distance_matrix[i][j],np.ndarray):
                list_data_distance = distance_matrix[i,j].flatten().tolist()
                list_data_collosion = collosion_matrix[i,j].flatten().tolist()
                photon_number = len(list_data_distance)
                # print(list_data_distance)
                # print(photon_number)
                for k in range(photon_number):
                    # print(k)
                    # print(list_data_distance[k])
                    distance = list_data_distance[k]
                    collosioin = list_data_collosion[k]
                    if range_min <= distance <= range_max:
                        bin_index = min(int((distance - range_min) / bin_width), bin_size - 1)
                        stamped_histogram[i, j, bin_index] += 1
                        stamped_collosion[i,j, bin_index] += collosioin
                for k in range(bin_size):
                    if stamped_histogram[i,j,k] > 0:
                        stamped_collosion[i,j,k] = stamped_collosion[i,j,k]/stamped_histogram[i,j,k]
    return stamped_histogram, stamped_collosion


def plot_distance_image(stamped_histogram, range_min, bin_width):
    height, width, bin_size = stamped_histogram.shape
    distance_image = np.zeros((height, width), dtype=np.float32)

    for i in range(height):
        for j in range(width):
            max_bin_index = np.argmax(stamped_histogram[i, j])
            if stamped_histogram[i, j, max_bin_index] > 0:
                distance_image[i, j] = range_min + (max_bin_index + 0.5) * bin_width

    return distance_image
                 
                
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
    plt.imshow(show_image,cmap=cmap ,vmin = 1000*2,vmax=1600*2 ,interpolation='nearest')
    plt.colorbar()
    plt.savefig(imageFileName, format='png', dpi=600, transparent=True)
    plt.show()   



if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Usage: python process_mat_files.py <file1.mat> <file2.mat> <output_file.h5>")
        sys.exit(1)
    mat_file1 = sys.argv[1]
    mat_file2 = sys.argv[2]
    stamped_histogram, stamped_collosion = load_and_process_matrices(mat_file1, mat_file2)
    distance_image = plot_distance_image(stamped_histogram, 1000 * 2, (1600 * 2 - 1000 * 2) / 20 )
    display_image(distance_image,None,'./distance_image.png')

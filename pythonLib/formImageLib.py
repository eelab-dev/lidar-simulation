import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import matplotlib.patches as mpatches
import sys
import os
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
    with h5py.File(file_path, "r") as h5file:
        dataset = h5file["photon_data"]
        resolution_width = int(h5file.attrs["width"])
        resolution_height = int(h5file.attrs["height"])

        pixel_output_array = [[[] for _ in range(resolution_height)] for _ in range(resolution_width)]

        for i in range(resolution_width):
            for j in range(resolution_height):
                try:
                    photon_array = dataset[i, j]
                    if photon_array is not None:
                        pixel_output_array[i][j] = [
                            (float(p["distance"]), int(p["collision_count"]))
                            for p in photon_array
                        ]
                except Exception as e:
                    # NULL pointer or unreadable entry — treat as empty
                    pixel_output_array[i][j] = []
                    # Optional: print(f"Warning: skipping NULL data at ({i}, {j}): {e}")

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


def form_histogram_image(pixels, image_width, image_height,bin_number = 25, range_distance = [2000,3500]):
    illegal_photon = np.empty((image_height, image_width), dtype=object)
    for i, j in np.ndindex(image_height, image_width):
        illegal_photon[i, j] = []
    stamped_histogram = np.zeros((image_height, image_width, bin_number), dtype=int)
    stamped_collosion = np.zeros((image_height, image_width, bin_number), dtype=float)

    distance_image = np.zeros((image_height, image_width), dtype=np.float32)
    range_min = range_distance[0]
    range_max = range_distance[1]
    bin_width = (range_max - range_min)/bin_number
    for i in range(image_height):
        for j in range(image_width): 
            photon_number = len(pixels[i][j])
            for k in range(photon_number):
                distance = pixels[i][j][k][0]
                collosion = pixels[i][j][k][1]
                if range_distance[0] > distance or range_distance[1] < distance:
                    illegal_photon[i][j].append((distance,collosion))
                else:
                    bin_index = min(int((distance - range_min) / bin_number), bin_number - 1)
                    stamped_histogram[i, j, bin_index] += 1
                    stamped_collosion[i,j, bin_index] += collosion
            for k in range(bin_number):
                if stamped_histogram[i,j,k] > 0:
                    stamped_collosion[i,j,k] = stamped_collosion[i,j,k]/stamped_histogram[i,j,k]
                
            max_bin_index = np.argmax(stamped_histogram[i, j])
            if stamped_histogram[i, j, max_bin_index] > 0:
                distance_image[i, j] = range_min + (max_bin_index + 0.5) * bin_width
    return distance_image, illegal_photon, stamped_histogram, stamped_collosion

def display_image(image, rectangle=None, imageFileName = "./", distance_range = [2100,3000]):
    # display the image
    # Create the colormap
    depthmap = mcolors.LinearSegmentedColormap.from_list('depth_cmap', colors, N=256)
    cmap = depthmap
    cmap.set_bad(color='black')
    # show image using matplotlib
    range_min = distance_range[0]
    range_max = distance_range[1]
    show_image = np.ma.masked_where((image == 0), image)

    
    if rectangle is not None:
        fig, ax = plt.subplots()
        rect = mpatches.Rectangle((rectangle[0], rectangle[1]), rectangle[2], rectangle[3], fill=False, edgecolor='red', linewidth=1, facecolor='none')
        ax.add_patch(rect)

    # Ensure the directory exists
    os.makedirs(os.path.dirname(imageFileName), exist_ok=True)
    plt.imshow(show_image,cmap=cmap ,vmin = range_min,vmax=range_max ,interpolation='nearest')
    plt.colorbar()
    plt.savefig(imageFileName, format='png', dpi=600, transparent=True)
    plt.show()
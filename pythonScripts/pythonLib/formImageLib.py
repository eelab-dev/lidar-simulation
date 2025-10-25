import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import matplotlib.patches as mpatches
import sys
import os
import h5py
import random
import cv2
from concurrent.futures import ThreadPoolExecutor


depthmap_colors = np.array([
    [0.55, 0.0, 0.0, 1.0],  # Dark Red
    [0.85, 0.0, 0.0, 1.0],  # Crimson
    [1.0, 0.0, 0.0, 1.0],   # Red
    [1.0, 0.3, 0.0, 1.0],   # Vermilion
    [1.0, 0.55, 0.0, 1.0],  # Dark Orange
    [1.0, 0.65, 0.0, 1.0],  # Orange
    [1.0, 0.84, 0.0, 1.0],  # Gold
    [1.0, 1.0, 0.0, 1.0],   # Yellow
    [0.8, 1.0, 0.2, 1.0],   # Yellow-Green
    [0.5, 1.0, 0.0, 1.0],   # Light Green
    [0.0, 1.0, 0.0, 1.0],   # Green
    [0.0, 1.0, 0.5, 1.0],   # Spring Green
    [0.0, 0.75, 0.5, 1.0],  # Turquoise
    [0.0, 1.0, 1.0, 1.0],   # Cyan
    [0.0, 0.5, 1.0, 1.0],   # Sky Blue
    [0.0, 0.0, 1.0, 1.0],   # Blue
    [0.0, 0.0, 0.5, 1.0],   # Dark Blue
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

def crop_image(left_index,right_index,bottom_index,top_index,input_histogarm,input_collosion):
    crop_histogram = input_histogarm[left_index:right_index, bottom_index:top_index, :]
    crop_collosion = input_collosion[left_index:right_index, bottom_index:top_index, :]
    width,height,bin_width = crop_histogram.shape
    return width, height, bin_width, crop_histogram, crop_collosion

def form_average_image(pixels, image_width, image_heigh):
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


def form_histogram_image(pixels, image_width, image_height,bin_number = 25, range_distance = [1000,2500]):
    illegal_photon = np.empty((image_width,image_height), dtype=object)
    for i, j in np.ndindex(image_width,image_height):
        illegal_photon[i, j] = []
    stamped_histogram = np.zeros((image_width,image_height,bin_number), dtype=int)
    stamped_collosion = np.zeros((image_width,image_height,bin_number), dtype=float)

    distance_image = np.zeros((image_width,image_height), dtype=np.float32)
    range_min = range_distance[0]
    range_max = range_distance[1]
    bin_width = (range_max - range_min)/bin_number
    for i in range(image_width):
        for j in range(image_height): 
            photon_number = len(pixels[i][j])
            for k in range(photon_number):
                distance = pixels[i][j][k][0]
                collosion = pixels[i][j][k][1]
                if range_distance[0] > distance or range_distance[1] < distance:
                    illegal_photon[i][j].append((distance,collosion))
                else:
                    bin_index = min(int((distance - range_min) / bin_width), bin_number - 1)
                    stamped_histogram[i, j, bin_index] += 1
                    stamped_collosion[i,j, bin_index] += collosion
            for k in range(bin_number):
                if stamped_histogram[i,j,k] > 0:
                    stamped_collosion[i,j,k] = stamped_collosion[i,j,k]/stamped_histogram[i,j,k]
                
            max_bin_index = np.argmax(stamped_histogram[i, j])
            if stamped_histogram[i, j, max_bin_index] > 0:
                distance_image[i, j] = range_min + (max_bin_index + 0.5) * bin_width
    return distance_image, illegal_photon, stamped_histogram, stamped_collosion

def save_image(image, rectangle=None,imageFileName = "./",distance_range=[2100,3000]):
    depthmap = mcolors.LinearSegmentedColormap.from_list('depth_cmap', colors, N=256)
    cmap = depthmap
    cmap.set_bad(color='black')
    # show image using matplotlib
    range_min = distance_range[0]
    range_max = distance_range[1]
    show_image = np.ma.masked_where((image == 0), image)

    # Ensure the directory exists
    os.makedirs(os.path.dirname(imageFileName), exist_ok=True)
    plt.imshow(show_image.T,cmap=cmap ,origin='lower',vmin = range_min,vmax=range_max ,interpolation='nearest')
    plt.colorbar()
    plt.savefig(imageFileName, format='png', dpi=600, transparent=True)
  
    # plt.show()






def display_combined_normalized_histogram(histogram_matrix):


    total_photons = np.sum(histogram_matrix)
    if total_photons > 0:
        scale = 10000.0 / total_photons
        histogram = histogram_matrix * scale
    combined_histogram = np.sum(histogram, axis=(0, 1))  # shape will be (depth,)



    bin_centers = np.arange(len(combined_histogram))  # or use bin_width and range_min if needed
    plt.figure()
    plt.bar( bin_centers, combined_histogram, width=1)
    plt.xlabel("Bin Index")
    plt.ylabel("Count")
    plt.grid(True)
    plt.show()



def display_image(image, rectangle=None,distance_range = [2100,3000]):
    # display the image
    # Create the colormap

    depthmap = mcolors.LinearSegmentedColormap.from_list('depth_cmap', colors, N=256)
    cmap = depthmap
    cmap.set_bad(color='black')
    # show image using matplotlib
    range_min = distance_range[0]
    range_max = distance_range[1]
    show_image = np.ma.masked_where((image == 0), image)
    # bin_num = 15
    # range_min = 0
    # range_max = bin_num 

    # Ensure the directory exists
    # os.makedirs(os.path.dirname(imageFileName), exist_ok=True)
    plt.imshow(show_image.T,origin = 'lower' ,cmap=cmap,vmin = range_min,vmax=range_max ,interpolation='nearest')
    plt.colorbar()
    # plt.savefig(imageFileName, format='png', dpi=600, transparent=True)
    plt.show()
# depthmap = mcolors.LinearSegmentedColormap.from_list('depth_cmap', colors, N=256)
# def display_image(image, distance_range=[2100, 3000], window_name="LiDAR Display"):
#     range_min, range_max = distance_range

#     # mask zeros
#     show_image = np.ma.masked_where((image == 0), image)

#     # normalize to 0–255 for OpenCV
#     norm = np.clip((show_image - range_min) / (range_max - range_min), 0, 1)
#     norm = (norm.filled(0) * 255).astype(np.uint8)

#     # convert matplotlib colormap to OpenCV color image
#     cmap_vals = (depthmap(norm / 255.0)[:, :, :3] * 255).astype(np.uint8)

#     # OpenCV wants BGR
#     cmap_bgr = cv2.cvtColor(cmap_vals, cv2.COLOR_RGB2BGR)

#     cv2.imshow(window_name, cmap_bgr)
#     cv2.waitKey(1)   # 1 ms delay → updates window in real-time

def display_pixel_histogram_from_matrix(matrix, x_index, y_index, max_bin = None, display = False):

    pixel_hist = matrix[x_index,y_index,:]
    if max_bin is not None:
        pixel_hist = pixel_hist[10:max_bin]

    bin_centers = np.arange(len(pixel_hist))  # or use bin_width and range_min if needed
    if display == True:
        plt.figure()
        plt.bar( bin_centers, pixel_hist, width=1)
        plt.xlabel("Bin Index")
        plt.ylabel("Count")
        plt.grid(True)
        plt.show()
    return pixel_hist

def display_image_fromH5(filename):


    with h5py.File(filename, 'r') as h5f:
        stamped_histogram = h5f["stamped_histogram"][:]
        range_max = h5f.attrs["range_max"]
        range_min = h5f.attrs["range_min"]
        height, width,bin_num = stamped_histogram.shape
        print(stamped_histogram.shape)
        bin_width= (range_max - range_min) / bin_num
        bin_width = 1
        range_min = 0
        range_max = bin_num
        depthImage= np.zeros((height, width), dtype=np.float32)
        for i in range(height):
            for j in range(width):
                max_bin_index = np.argmax(stamped_histogram[i, j])
                if stamped_histogram[i, j, max_bin_index] > 0:
                    depthImage[i, j] = range_min + (max_bin_index + 0.5) * bin_width
        depthmap = mcolors.LinearSegmentedColormap.from_list('depth_cmap', colors, N=256)
        cmap = depthmap
        cmap.set_bad(color='black')
        
        displayImage = np.ma.masked_where((depthImage == 0), depthImage)
        plt.imshow(displayImage.T,cmap=cmap ,interpolation='nearest',origin='lower',vmin=range_min, vmax=range_max)
        plt.colorbar()
        plt.show()
        return stamped_histogram



def save_histogram_to_h5(filename, stamped_histogram, stamped_collosion,
                          range_min, range_max, image_width, image_height, bin_number):
    """
    Save histogram and collision data to an HDF5 file with metadata.

    Parameters:
    - filename (str): Output HDF5 file path
    - stamped_histogram (np.ndarray): Histogram data [H, W, B]
    - stamped_collosion (np.ndarray): Average collision data [H, W, B]
    - range_min (float): Minimum distance
    - range_max (float): Maximum distance
    - image_width (int): Width of the image
    - image_height (int): Height of the image
    - bin_number (int): Number of bins
    """
    with h5py.File(filename, 'w') as h5f:
        h5f.create_dataset("stamped_histogram", data=stamped_histogram, compression="gzip")
        h5f.create_dataset("stamped_collosion", data=stamped_collosion, compression="gzip")

        # Add metadata
        h5f.attrs["range_min"] = range_min
        h5f.attrs["range_max"] = range_max
        h5f.attrs["image_width"] = image_width
        h5f.attrs["image_height"] = image_height
        h5f.attrs["bin_number"] = bin_number

def read_histogram_from_h5(filename):
    """
    Reads histogram data and metadata from an HDF5 file.

    Parameters:
    - filename (str): Path to the .h5 file

    Returns:
    - stamped_histogram (np.ndarray)
    - stamped_collosion (np.ndarray)
    - metadata (dict): Includes range_min, range_max, image_width, image_height, bin_number
    """
    with h5py.File(filename, 'r') as h5f:
        stamped_histogram = h5f["stamped_histogram"][:]
        stamped_collosion = h5f["stamped_collosion"][:]

        metadata = {
            "range_min": h5f.attrs["range_min"],
            "range_max": h5f.attrs["range_max"],
            "image_width": h5f.attrs["image_width"],
            "image_height": h5f.attrs["image_height"],
            "bin_number": h5f.attrs["bin_number"],
        }

    return stamped_histogram, stamped_collosion, metadata


# def dead_time_simulation(real_photon_num=5000*2.37*10e5 * 5000 * 0.005, pluse_train_num = 5000, simulated_photon= None, dead_time = 35,imageWidth = 50,imageHeight = 50):
#     # simulated_photon_num = sum(len(row) for layer in simulated_photon for row in layer)
#     simulated_photon_num = 30000 * 200000 * 1
#     simulated_pluse_train = int(simulated_photon_num / real_photon_num * pluse_train_num)
#     speed_of_light = 3e8
#     discard_count = 0
#     accept_count = 0

#     # print(simulated_pluse_train)
#     pixel_output_array = [[[] for _ in range(imageHeight)] for _ in range(imageWidth)]

#     # Traverse the 3D list
#     for y, row in enumerate(simulated_photon):         # loop over height
#         for x, entries in enumerate(row):              # loop over width
#             if entries:
#                 last_photon_time_array = [None] * simulated_pluse_train
#                 for entrie in entries:
#                     arrive_time = entrie[0]/1000 / speed_of_light * 1e12
#                     pluse_index = random.randrange(simulated_pluse_train)
#                     if last_photon_time_array[pluse_index] == None:
#                         last_photon_time_array[pluse_index] = arrive_time  
#                         pixel_output_array[y][x].append(entrie)
#                         accept_count = accept_count + 1
#                     else:
#                         last_time = last_photon_time_array[pluse_index] 
#                         # first_time = True
#                         # if first_time:
#                         #     print(arrive_time)
#                         #     first_time = False
#                         if last_time + dead_time > arrive_time:
#                             discard_count = discard_count + 1
#                         else:
#                             accept_count = accept_count + 1
#                             pixel_output_array[y][x].append(entrie)
#                             last_photon_time_array[pluse_index] = arrive_time
#     return pixel_output_array,discard_count,accept_count

def dead_time_simulation(
    real_photon_num = 5000 * 2.37 * 10e5 * 5000 * 0.005,
    pluse_train_num = 50000,
    simulated_photon=None,
    dead_time=13000,
    imageWidth=50,
    imageHeight=50,
    max_workers=None
):
    if simulated_photon is None:
        raise ValueError("simulated_photon must be provided")

    # If you intended 1e5, note 10e5 == 1e6
    simulated_photon_num = 30000 * 2000000 * 1
    simulated_pluse_train = max(
        1, int(simulated_photon_num / real_photon_num * pluse_train_num)
    )

    speed_of_light = 3e8

    def process_pixel(y, x, entries):
        rng = random.Random(hash((y, x)))  # per-pixel rng
        local_out = []
        discard = 0
        accept = 0

        if entries:
            last_photon_time_array = [None] * simulated_pluse_train
            entries.sort(key=lambda e: e[0])
            for entrie in entries:
                arrive_time = entrie[0] / 1000 / speed_of_light * 1e12
                pluse_index = rng.randrange(simulated_pluse_train)

                last_time = last_photon_time_array[pluse_index]
                if last_time is None:
                    last_photon_time_array[pluse_index] = arrive_time
                    local_out.append(entrie)
                    accept += 1
                elif last_time + dead_time > arrive_time:
                    discard += 1
                else:
                    last_photon_time_array[pluse_index] = arrive_time
                    local_out.append(entrie)
                    accept += 1

        return (y, x, local_out, discard, accept)

    # Launch all pixels in parallel
    tasks = []
    for y, row in enumerate(simulated_photon):
        for x, entries in enumerate(row):
            tasks.append((y, x, entries))

    if max_workers is None:
        max_workers = min(32, os.cpu_count() or 1)

    pixel_output_array = [[[] for _ in range(imageWidth)] for _ in range(imageHeight)]
    discard_count = 0
    accept_count = 0

    with ThreadPoolExecutor(max_workers=max_workers) as ex:
        for y, x, out, d, a in ex.map(lambda args: process_pixel(*args), tasks):
            pixel_output_array[y][x] = out
            discard_count += d
            accept_count += a

    return pixel_output_array, discard_count, accept_count




                
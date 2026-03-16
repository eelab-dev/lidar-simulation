
import numpy as np
import matplotlib.pyplot as plt




def from_distance_from_histogram(
    histogram,
    image_height=None,
    image_width=None,
    range_max=None,
    range_min=None,
    bin_number=None,
    peak_find=np.argmax
):
    if histogram.ndim != 3:
        raise ValueError("histogram must be a 3D array with shape (image_height, image_width, bin_number).")
    if range_max is None or range_min is None:
        raise ValueError("range_max and range_min are required.")

    # Compute image size/bin count from histogram automatically.
    image_height, image_width, inferred_bin_number = histogram.shape
    bin_number = inferred_bin_number
    distance_image = np.zeros((image_height, image_width), dtype=np.float32)

    bin_width = (range_max - range_min)/bin_number
    for i in range(image_height):
        for j in range(image_width): 

            max_bin_index = peak_find(histogram[i, j])
            if  max_bin_index >= 0:
                distance_image[i, j] = range_min + (max_bin_index + 0.5) * bin_width
    return distance_image


def form_intensity_from_histogram(histogram, image_height=None, image_width=None, mode="sum", peak_find=np.argmax):
    """
    Form a 2D intensity image from a 3D histogram matrix.

    histogram shape is expected to be (image_height, image_width, bin_number).
    mode:
    - "sum": intensity = total histogram counts per pixel
    - "peak": intensity = value at the detected peak bin per pixel
    """
    if histogram.ndim != 3:
        raise ValueError("histogram must be a 3D array with shape (image_height, image_width, bin_number).")

    # Compute image size from histogram automatically.
    image_height, image_width, _ = histogram.shape
    intensity_image = np.zeros((image_height, image_width), dtype=np.float32)

    if mode == "sum":
        intensity_image = np.sum(histogram, axis=2, dtype=np.float32)
    elif mode == "peak":
        for i in range(image_height):
            for j in range(image_width):
                max_bin_index = peak_find(histogram[i, j])
                if max_bin_index >= 0:
                    intensity_image[i, j] = histogram[i, j, max_bin_index]
    else:
        raise ValueError("mode must be either 'sum' or 'peak'.")

    return intensity_image


def display_intensity(
    intensity_image,

    mask_zero=True
):
    """
    Display a 2D intensity image.
    """
    if intensity_image.ndim != 2:
        raise ValueError("intensity_image must be a 2D array.")

    show_image = np.ma.masked_where((intensity_image == 0), intensity_image) if mask_zero else intensity_image

    if mask_zero:
        valid_values = intensity_image[intensity_image != 0]
    else:
        valid_values = intensity_image.reshape(-1)

    if valid_values.size > 0:
        vmin = float(np.min(valid_values))
        vmax = float(np.max(valid_values))
        if vmax <= vmin:
            vmax = vmin + 1.0
    else:
        vmin, vmax = 0.0, 1.0

    plt.figure()
    plt.imshow(show_image.T, origin="lower", cmap="viridis", interpolation="nearest", vmin=vmin, vmax=vmax)
    plt.colorbar(label="Intensity")
    plt.title("Intensity Image")
    plt.show()

    return intensity_image





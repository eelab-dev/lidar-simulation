import h5py
import matplotlib.pyplot as plt
import numpy as np


def read_diffuseMap_file(file_path):
    """
    Read diffuse map output HDF5 file and return a 2D float array (width x height).
    """
    with h5py.File(file_path, "r") as h5file:
        if "diffuse_data" not in h5file:
            raise KeyError("Dataset 'diffuse_data' not found.")

        dataset = h5file["diffuse_data"][()]
        if "diffuse_avg" not in dataset.dtype.names:
            raise KeyError("Field 'diffuse_avg' not found in 'diffuse_data'.")

        diffuse_map = dataset["diffuse_avg"].astype(np.float32)

    return diffuse_map


def display_diffuseMap(diffuse_map, value_range=(0.0, 1.0), title="Diffuse Map"):
    """
    Display diffuse map. Values < 0 are treated as invalid and shown in black.
    """
    if not isinstance(diffuse_map, np.ndarray):
        diffuse_map = np.asarray(diffuse_map, dtype=np.float32)

    show_map = np.ma.masked_where(diffuse_map < 0.0, diffuse_map)
    cmap = plt.cm.viridis.copy()
    cmap.set_bad(color="black")

    plt.figure()
    plt.axis('off')
    plt.imshow(
        show_map.T,
        origin="lower",
        cmap=cmap,
        vmin=value_range[0],
        vmax=value_range[1],
        interpolation="nearest",
    )
    # plt.title(title)
    # plt.colorbar(label="Diffuse Average")
    plt.tight_layout()
    plt.show()

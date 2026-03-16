import numpy as np


def generate_ambient_noise(dnl_matrix, histogram_matrixc, diffuse_map, noise_factor, seed=None):
    """
    Generate indirect ambient white-noise photons for a histogram cube.

    Parameters
    ----------
    dnl_matrix : ndarray or None
        3D DNL matrix with shape (W, H, B). Bin widths are treated as
        proportional to (1 + dnl). If None, bins are assumed evenly spaced.
    histogram_matrixc : ndarray
        Input photon histogram matrix with shape (W, H, B).
    diffuse_map : ndarray
        2D diffuse map with shape (W, H). Ambient-noise strength per pixel is
        proportional to this map.
    noise_factor : float
        Noise strength in percent of total photon count in `histogram_matrixc`.
        Example: noise_factor=5 means noise photons ~= 5% of total photons.
    seed : int or None
        Optional RNG seed.

    Returns
    -------
    ambient_noise_matrix : ndarray
        Integer noise histogram with shape (W, H, B).
    """

    histogram_matrixc = np.asarray(histogram_matrixc)
    diffuse_map = np.asarray(diffuse_map)

    if histogram_matrixc.ndim != 3:
        raise ValueError("histogram_matrixc must be 3D with shape (W, H, B).")
    if diffuse_map.ndim != 2:
        raise ValueError("diffuse_map must be 2D with shape (W, H).")

    width, height, bin_number = histogram_matrixc.shape
    if diffuse_map.shape != (width, height):
        raise ValueError(
            f"Shape mismatch: diffuse_map{diffuse_map.shape} must match (W, H)=({width}, {height})."
        )
    if dnl_matrix is not None:
        dnl_matrix = np.asarray(dnl_matrix)
        if dnl_matrix.ndim != 3:
            raise ValueError("dnl_matrix must be 3D with shape (W, H, B), or None.")
        if dnl_matrix.shape != histogram_matrixc.shape:
            raise ValueError(
                f"Shape mismatch: dnl_matrix{dnl_matrix.shape} != histogram_matrixc{histogram_matrixc.shape}."
            )

    if float(noise_factor) < 0:
        raise ValueError("noise_factor must be >= 0.")

    total_photons = float(np.sum(histogram_matrixc))
    total_noise_photons = int(round(total_photons * (float(noise_factor) / 100.0)))

    ambient_noise_matrix = np.zeros((width, height, bin_number), dtype=np.int32)
    if total_noise_photons == 0:
        return ambient_noise_matrix

    rng = np.random.default_rng(seed)

    # Indirect ambient noise strength is proportional to diffuse value.
    pixel_diffuse = np.clip(diffuse_map.astype(np.float64), a_min=0.0, a_max=None)
    diffuse_sum = float(np.sum(pixel_diffuse))

    if diffuse_sum <= 0.0:
        pixel_prob = np.full(width * height, 1.0 / (width * height), dtype=np.float64)
    else:
        pixel_prob = (pixel_diffuse / diffuse_sum).reshape(-1)

    # Allocate total noise photons across pixels based on diffuse map.
    pixel_noise_counts = rng.multinomial(total_noise_photons, pixel_prob)

    # Bin probabilities from bin-width ratios:
    # p_k = width_k / sum(width), with width_k proportional to (1 + dnl_k).
    if dnl_matrix is None:
        bin_prob_matrix = np.full((width, height, bin_number), 1.0 / bin_number, dtype=np.float64)
    else:
        widths = 1.0 + dnl_matrix.astype(np.float64)
        widths = np.clip(widths, a_min=1e-12, a_max=None)
        widths_sum = np.sum(widths, axis=2, keepdims=True)
        invalid = widths_sum <= 0.0
        widths_sum[invalid] = float(bin_number)
        bin_prob_matrix = widths / widths_sum

    for pixel_idx, n_noise in enumerate(pixel_noise_counts):
        if n_noise == 0:
            continue

        i = pixel_idx // height
        j = pixel_idx % height
        ambient_noise_matrix[i, j, :] = rng.multinomial(n_noise, bin_prob_matrix[i, j, :])


    return ambient_noise_matrix

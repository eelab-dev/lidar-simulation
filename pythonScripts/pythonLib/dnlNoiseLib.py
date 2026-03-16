import numpy as np
import matplotlib.pyplot as plt

def generate_dnl_matrix(image_width, image_height, bin_number, dnl_value, seed=None):
    """
    Generate a randomized 3D DNL matrix.

    Parameters
    ----------
    image_width : int
    image_height : int
    bin_number : int
    dnl_value : float
        Standard deviation of DNL (LSB fraction).
    seed : int or None
        Random seed for reproducibility.

    Returns
    -------
    dnl_matrix : ndarray
        Shape (image_width, image_height, bin_number)
    """

    rng = np.random.default_rng(seed)

    dnl_matrix = rng.normal(
        loc=0.0,
        scale=dnl_value,
        size=(image_width, image_height, bin_number)
    )
    dnl_matrix = np.clip(dnl_matrix, -1.0, 1.0)

    return dnl_matrix


def _normalize_dnl_rows(dnl_rows):
    """
    Normalize input to a list of 1D numpy arrays.

    Accepts:
    - 1D array-like: one row
    - 2D array-like: each row is a DNL row
    - list/tuple of 1D array-like rows
    """
    arr = np.asarray(dnl_rows, dtype=float)
    if arr.ndim == 1:
        return [arr.reshape(-1)]
    if arr.ndim == 2:
        return [arr[i].reshape(-1) for i in range(arr.shape[0])]
    raise ValueError("dnl_rows must be a 1D row or a 2D list/array of rows.")


def displayDNL(dnl_rows):
    """
    Plot one or multiple DNL rows together.

    Parameters
    ----------
    dnl_rows : array-like
        1D DNL row, or 2D/list of DNL rows.
    """
    rows = _normalize_dnl_rows(dnl_rows)

    fig, ax = plt.subplots(figsize=(8, 4))
    for idx, row in enumerate(rows):
        x = np.arange(row.size)
        ax.plot(x, row, linewidth=1.2, label=f"DNL row {idx}")
    ax.axhline(0.0, color="black", linestyle="--", linewidth=0.8)
    ax.set_xlabel("Bin Index")
    ax.set_ylabel("DNL (LSB)")
    ax.set_title("Differential Nonlinearity (DNL)")
    ax.grid(True, alpha=0.3)
    ax.legend()
    plt.tight_layout()
    plt.show()
    return fig, ax


def displayINL(dnl_rows):
    """
    Plot INL curves computed from one or multiple DNL rows.

    Parameters
    ----------
    dnl_rows : array-like
        1D DNL row, or 2D/list of DNL rows.
    """
    rows = _normalize_dnl_rows(dnl_rows)

    fig, ax = plt.subplots(figsize=(8, 4))
    for idx, row in enumerate(rows):
        inl_row = np.cumsum(row)
        x = np.arange(inl_row.size)
        ax.plot(x, inl_row, linewidth=1.2, label=f"INL row {idx}")
    ax.axhline(0.0, color="black", linestyle="--", linewidth=0.8)
    ax.set_xlabel("Bin Index")
    ax.set_ylabel("INL (LSB)")
    ax.set_title("Integral Nonlinearity (INL)")
    ax.grid(True, alpha=0.3)
    ax.legend()
    plt.tight_layout()
    plt.show()
    return fig, ax

#include <cmath>
#include <utility>

std::pair<int, int> computeAdjustedSize(int input_width, int input_height, int target_area = 250000) {
    if (input_width <= 0 || input_height <= 0) {
        throw std::invalid_argument("Width and height must be positive integers.");
    }

    // Calculate the aspect ratio
    double aspect_ratio = static_cast<double>(input_width) / input_height;

    // Calculate height from area and aspect ratio: h = sqrt(area / aspect_ratio)
    int new_height = static_cast<int>(std::ceil(std::sqrt(target_area / aspect_ratio)));
    int new_width = static_cast<int>(std::ceil(new_height * aspect_ratio));

    // Ensure area is not less than target
    while (new_width * new_height < target_area) {
        new_height++;
        new_width = static_cast<int>(std::ceil(new_height * aspect_ratio));
    }

    return {new_width, new_height};
}
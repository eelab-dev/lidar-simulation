#pragma once

#include <H5Cpp.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

inline std::unordered_map<std::string, std::vector<std::string>> parseFlags(int argc, char* argv[]) {
    std::unordered_map<std::string, std::vector<std::string>> args;
    std::string current_key;

    for (int i = 1; i < argc; ++i) {
        std::string token = argv[i];
        if (token.rfind("--", 0) == 0) {
            current_key = token;
            args[current_key] = {};
        } else if (!current_key.empty()) {
            args[current_key].push_back(token);
        }
    }
    return args;
}

struct DiffuseGridCell {
    float diffuse_avg = -1.0f;
    int32_t valid_count = 0;
};

inline std::vector<DiffuseGridCell> buildDiffuseGrid(
    const std::vector<float>& diffuse_sum,
    const std::vector<int>& diffuse_count,
    int image_width,
    int image_height
) {
    const size_t pixel_num = static_cast<size_t>(image_width) * static_cast<size_t>(image_height);
    std::vector<DiffuseGridCell> grid(pixel_num);

    for (size_t idx = 0; idx < pixel_num; ++idx) {
        const int count = diffuse_count[idx];
        grid[idx].valid_count = static_cast<int32_t>(count);
        if (count > 0) {
            grid[idx].diffuse_avg = diffuse_sum[idx] / static_cast<float>(count);
        } else {
            grid[idx].diffuse_avg = -1.0f;
        }
    }
    return grid;
}

class diffuseHDF5Writer {
private:
    std::string filename;
    H5::H5File file;
    H5::DataSet datasetDiffuse;
    H5::CompType diffuseCellType;

public:
    explicit diffuseHDF5Writer(
        const std::string& outputFilename,
        float fov_x,
        float fov_y,
        int height,
        int width
    );

    void writeGrid(const std::vector<float>& diffuse_sum, const std::vector<int>& diffuse_count, int image_width, int image_height);
    void finalizeFile();

private:
    void initializeFile(float fov_x, float fov_y, int image_height, int image_width);
};

inline diffuseHDF5Writer::diffuseHDF5Writer(
    const std::string& outputFilename,
    float fov_x = 50.0f,
    float fov_y = 50.0f,
    int height = 500,
    int width = 500
) : filename(outputFilename),
    file(H5::H5File(outputFilename, H5F_ACC_TRUNC)),
    diffuseCellType(sizeof(DiffuseGridCell)) {
    initializeFile(fov_x, fov_y, height, width);
}

inline void diffuseHDF5Writer::initializeFile(float fov_x, float fov_y, int image_height, int image_width) {
    hsize_t dims[2] = {
        static_cast<hsize_t>(image_width),
        static_cast<hsize_t>(image_height)
    };
    H5::DataSpace dataspace(2, dims);

    diffuseCellType.insertMember("diffuse_avg", HOFFSET(DiffuseGridCell, diffuse_avg), H5::PredType::NATIVE_FLOAT);
    diffuseCellType.insertMember("valid_count", HOFFSET(DiffuseGridCell, valid_count), H5::PredType::NATIVE_INT32);

    datasetDiffuse = file.createDataSet("diffuse_data", diffuseCellType, dataspace);

    H5::DataSpace scalarSpace(H5S_SCALAR);
    int width_attr = image_width;
    int height_attr = image_height;
    file.createAttribute("FOV_X", H5::PredType::NATIVE_FLOAT, scalarSpace).write(H5::PredType::NATIVE_FLOAT, &fov_x);
    file.createAttribute("FOV_Y", H5::PredType::NATIVE_FLOAT, scalarSpace).write(H5::PredType::NATIVE_FLOAT, &fov_y);
    file.createAttribute("width", H5::PredType::NATIVE_INT, scalarSpace).write(H5::PredType::NATIVE_INT, &width_attr);
    file.createAttribute("height", H5::PredType::NATIVE_INT, scalarSpace).write(H5::PredType::NATIVE_INT, &height_attr);
}

inline void diffuseHDF5Writer::writeGrid(
    const std::vector<float>& diffuse_sum,
    const std::vector<int>& diffuse_count,
    int image_width,
    int image_height
) {
    if (diffuse_sum.empty() || diffuse_count.empty()) {
        return;
    }

    std::vector<DiffuseGridCell> grid = buildDiffuseGrid(diffuse_sum, diffuse_count, image_width, image_height);
    datasetDiffuse.write(grid.data(), diffuseCellType);
}

inline void diffuseHDF5Writer::finalizeFile() {
    file.close();
}

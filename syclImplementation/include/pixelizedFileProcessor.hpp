#pragma once

#include <H5Cpp.h>
#include "Vec.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <sycl/sycl.hpp>
// #include <sycl/ext/oneapi/algorithm.hpp>
#include <oneapi/dpl/algorithm>
#include <oneapi/dpl/execution>
#include <iostream>

// // Extracts --key value pairs from command-line arguments
// std::unordered_map<std::string, std::string> parseFlags(int argc, char* argv[]) {
//     std::unordered_map<std::string, std::string> args;

//     for (int i = 1; i < argc - 1; ++i) {
//         std::string key = argv[i];
//         if (key.rfind("--", 0) == 0) { // starts with "--"
//             std::string value = argv[i + 1];
//             args[key] = value;
//             ++i; // skip next since it's a value
//         }
//     }

//     return args;
// }



std::unordered_map<std::string, std::vector<std::string>> parseFlags(int argc, char* argv[]) {
    std::unordered_map<std::string, std::vector<std::string>> args;
    std::string current_key;

    for (int i = 1; i < argc; ++i) {
        std::string token = argv[i];
        if (token.rfind("--", 0) == 0) { // Token starts with "--"
            current_key = token;
            // Initialize with an empty vector in case it's a flag with no value
            args[current_key] = {}; 
        } else if (!current_key.empty()) {
            // If we have a key, this token is a value for it
            args[current_key].push_back(token);
        }
    }

    return args;
}

struct CollisionRecord {
    int collisionCount;
    float distance;
    Vec3 collisionLocation;
    Vec3 collisionDirection;
    int camera_x = -1;
    int camera_y = -1;
    float emission_delay = -1;
};


struct pixelPhotonRecord {
    float   distance;        // np.float32
    int32_t collision_count; // np.int32
};

using PixelPhotonList = std::vector<pixelPhotonRecord>;
using PhotonGrid = std::vector<std::vector<PixelPhotonList>>;

void host_exclusive_scan(const std::vector<int>& in, std::vector<int>& out) {
    std::exclusive_scan(in.begin(), in.end(), out.begin(), 0);
}

class pixelizedHDF5Writer {
private:
    std::string filename;
    size_t current_index;
    H5::H5File file;
    H5::DataSet datasetCollision;
    H5::VarLenType vlenPixelType;
    



public:
    explicit pixelizedHDF5Writer(const std::string& outputFilename,float fov_x, float fov_y, int height, int width);
    void finalizeFile();
    void writeRecord(int collisionCount, float distance, Vec3 collisionLocation, Vec3 collisionDirection, int camera_x, int camera_y, float emission_delay);
    
    void writeToFile(
        const std::vector<int>& collisionCount,
        const std::vector<float>& distance,
        const std::vector<Vec3>& collisionLocation,
        const std::vector<Vec3>& collisionDirection,
        sycl::queue& myQueue
    );

    void writeBatch(const std::vector<CollisionRecord>& records, int image_width, int image_height);
private:
    void initializeFile(float fov_x, float fov_y, int height,int width);
    

};

// Constructor
pixelizedHDF5Writer::pixelizedHDF5Writer(const std::string& outputFilename, float fov_x = 50, float fov_y = 50, int height = 500, int width = 500)
    : filename(outputFilename), current_index(0),
      file(H5::H5File(outputFilename, H5F_ACC_TRUNC)) {
    initializeFile(fov_x,fov_y, height, width);
}


void pixelizedHDF5Writer::initializeFile(float fov_x = 50, float fov_y = 50, int image_height = 500, int image_width = 500) {

    // hsize_t init_size[1] = {0};  // Start with 0 records
    // hsize_t max_size[1] = {H5S_UNLIMITED};  // Allow unlimited records
    
    hsize_t dims[2] = {
        static_cast<hsize_t>(image_width),
        static_cast<hsize_t>(image_height)
    };

    H5::DataSpace dataspace(2, dims); // <--- 2D array

    // H5::DataSpace space(1, init_size, max_size);

    // Define compound datatype
    H5::CompType pixelType(sizeof(pixelPhotonRecord));

    // compType.insertMember("CollisionCount", HOFFSET(CollisionRecord, collisionCount), H5::PredType::NATIVE_INT);
    // compType.insertMember("Distance", HOFFSET(CollisionRecord, distance), H5::PredType::NATIVE_FLOAT);
    pixelType.insertMember("distance",
                            HOFFSET(pixelPhotonRecord, distance),
                            H5::PredType::NATIVE_FLOAT);
    pixelType.insertMember("collision_count",
                            HOFFSET(pixelPhotonRecord, collision_count),
                            H5::PredType::NATIVE_INT32);

    // H5::VarLenType vlenPixelType(pixelType);

    vlenPixelType = H5::VarLenType(pixelType);
   
    datasetCollision = file.createDataSet(
        "photon_data",       // same name as in Python
        vlenPixelType,      // type: vlen compound
        dataspace            // space: (width, height)
    );
    // hsize_t vec3_dims[1] = {3};
    // H5::ArrayType vec3Type(H5::PredType::NATIVE_FLOAT, 1, vec3_dims);
    
    // compType.insertMember("CollisionLocation", HOFFSET(CollisionRecord, collisionLocation), vec3Type);
    // compType.insertMember("CollisionDirection", HOFFSET(CollisionRecord, collisionDirection), vec3Type);


    // compType.insertMember("Camera_x", HOFFSET(CollisionRecord, camera_x), H5::PredType::NATIVE_INT);
    // compType.insertMember("Camera_y", HOFFSET(CollisionRecord, camera_y), H5::PredType::NATIVE_INT);
    // compType.insertMember("emission_delay",HOFFSET(CollisionRecord, emission_delay), H5::PredType::NATIVE_FLOAT); 
    // // Enable chunking (for extendability)
    // H5::DSetCreatPropList prop;
    // hsize_t chunk_dims[1] = {100}; // Chunk size (can adjust based on expected record rate)
    // prop.setChunk(1, chunk_dims);

    // // Create dataset with unlimited size
    // datasetCollision = file.createDataSet("CollisionData", compType, space, prop);

    H5::DataSpace scalarSpace(H5S_SCALAR);

    int width_attr  = image_width;
    int height_attr = image_height;

    file.createAttribute("FOV_X",H5::PredType::NATIVE_FLOAT,scalarSpace).write(H5::PredType::NATIVE_FLOAT, &fov_x);

    file.createAttribute("FOV_Y",H5::PredType::NATIVE_FLOAT,scalarSpace).write(H5::PredType::NATIVE_FLOAT, &fov_y);

    file.createAttribute("width",H5::PredType::NATIVE_INT,scalarSpace).write(H5::PredType::NATIVE_INT, &width_attr);

    file.createAttribute("height",H5::PredType::NATIVE_INT,scalarSpace).write(H5::PredType::NATIVE_INT, &height_attr);
    // // Create scalar dataspace for single values
    // H5::DataSpace scalar_space = H5::DataSpace(H5S_SCALAR);

    // file.createAttribute("FOV", H5::PredType::NATIVE_FLOAT, scalar_space).write(H5::PredType::NATIVE_FLOAT, &fov);
    // file.createAttribute("ImageHeight", H5::PredType::NATIVE_INT, scalar_space).write(H5::PredType::NATIVE_INT, &image_height);
    // file.createAttribute("ImageWidth", H5::PredType::NATIVE_INT, scalar_space).write(H5::PredType::NATIVE_INT, &image_width);
}



void pixelizedHDF5Writer::writeRecord(int collisionCount, float distance, Vec3 collisionLocation, Vec3 collisionDirection, int camera_x, int camera_y, float emission_delay) {

    CollisionRecord record;
    record.collisionCount = collisionCount;
    record.distance = distance;
    record.collisionLocation.x = collisionLocation.x;
    record.collisionLocation.y = collisionLocation.y;
    record.collisionLocation.z = collisionLocation.z;
    record.collisionDirection.x = collisionDirection.x;
    record.collisionDirection.y = collisionDirection.y;
    record.collisionDirection.z = collisionDirection.z;
    record.camera_x = camera_x;
    record.camera_y = camera_y;
    record.emission_delay = emission_delay;
    

    // Extend dataset to accommodate new record
    hsize_t new_size[1] = {current_index + 1};
    datasetCollision.extend(new_size);

    // Select location for writing new data
    hsize_t offset[1] = {static_cast<hsize_t>(current_index)};
    hsize_t dims[1] = {1}; // Writing one record
    H5::DataSpace memspace(1, dims);

    H5::DataSpace dataspace = datasetCollision.getSpace();
    dataspace.selectHyperslab(H5S_SELECT_SET, dims, offset);

    // Write the record
    datasetCollision.write(&record, datasetCollision.getCompType(), memspace, dataspace);

    // Move to the next index
    current_index++;
}

PhotonGrid buildPhotonGrid(const std::vector<CollisionRecord>& records,
                           int image_width,
                           int image_height)
{
    // Allocate 2D grid: width × height, each cell starts as empty vector<pixelPhotonRecord>
    PhotonGrid grid;
    grid.resize(image_width);
    for (int x = 0; x < image_width; ++x) {
        grid[x].resize(image_height);
    }

    // Fill from flat list of collisions
    for (const auto& rec : records) {
        int x = rec.camera_x;
        int y = rec.camera_y;

        // Safety: skip out-of-bounds pixels
        if (x < 0 || x >= image_width || y < 0 || y >= image_height) {
            continue;
        }

        pixelPhotonRecord p{};
        p.distance        = rec.distance;
        p.collision_count = rec.collisionCount;

        grid[x][y].push_back(p);
    }

    return grid;
}


void pixelizedHDF5Writer::writeBatch(const std::vector<CollisionRecord>& records, int image_width, int image_height) {
    if (records.empty()) return;


    PhotonGrid pixel_output_array = buildPhotonGrid(records,image_width,image_height);

    std::vector<hvl_t> vlenBuffer(image_width * image_height);

    for (int x = 0; x < image_width; ++x) {
        for (int y = 0; y < image_height; ++y) {
            const auto& photons = pixel_output_array[x][y];

            hvl_t& cell = vlenBuffer[x * image_height + y];

            cell.len = photons.size();
            cell.p   = photons.empty()
                ? nullptr
                : const_cast<pixelPhotonRecord*>(photons.data());
        }
    }

    // Write the entire 2D dataset in one call, like h5file.create_dataset(..., data=data)
    // vlenPixelType is the H5::VarLenType you created in initializeFile
    datasetCollision.write(vlenBuffer.data(), vlenPixelType);
}


void pixelizedHDF5Writer::finalizeFile() {
    file.close();
}





std::vector<CollisionRecord> filterCollisionRecordsSYCL(
    const std::vector<CollisionRecord>& inputRecords,
    sycl::queue& myQueue
)
{
    size_t recordNum = inputRecords.size();
    std::vector<int> mask(recordNum);
    std::vector<int> positions(recordNum);
    std::vector<CollisionRecord> filtered(recordNum); // max possible size

    // Step 1: Build mask where collisionCount > 0
    {
        sycl::buffer<CollisionRecord> in_buf(inputRecords);
        sycl::buffer<int> mask_buf(mask);

        myQueue.submit([&](sycl::handler& h) {
            auto in = in_buf.get_access<sycl::access::mode::read>(h);
            auto m = mask_buf.get_access<sycl::access::mode::write>(h);
            h.parallel_for(recordNum, [=](sycl::id<1> i) {
                m[i] = in[i].collisionCount > 0 ? 1 : 0;
            });
        });
    }

    // Step 2: Exclusive scan on CPU
    host_exclusive_scan(mask, positions);

    // Step 3: Compact valid records into `filtered`
    {
        sycl::buffer<CollisionRecord> in_buf(inputRecords);
        sycl::buffer<int> mask_buf(mask);
        sycl::buffer<int> pos_buf(positions);
        sycl::buffer<CollisionRecord> out_buf(filtered);

        myQueue.submit([&](sycl::handler& h) {
            auto in = in_buf.get_access<sycl::access::mode::read>(h);
            auto m = mask_buf.get_access<sycl::access::mode::read>(h);
            auto p = pos_buf.get_access<sycl::access::mode::read>(h);
            auto out = out_buf.get_access<sycl::access::mode::write>(h);

            h.parallel_for(recordNum, [=](sycl::id<1> i) {
                if (m[i]) {
                    int idx = p[i];
                    out[idx] = in[i];
                }
            });
        });
        myQueue.wait_and_throw();
    }

    // Step 4: Trim to valid size
    int validCount = 0;
    if (recordNum > 0) {
        validCount = positions[recordNum - 1] + mask[recordNum - 1];
    }
    std::cout << "Invalid records: " << recordNum - validCount << std::endl;
    filtered.resize(validCount);

    return filtered;
}






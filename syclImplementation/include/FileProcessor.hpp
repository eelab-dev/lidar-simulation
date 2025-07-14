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
};


void host_exclusive_scan(const std::vector<int>& in, std::vector<int>& out) {
    std::exclusive_scan(in.begin(), in.end(), out.begin(), 0);
}

class HDF5Writer {
private:
    std::string filename;
    size_t current_index;
    H5::H5File file;
    H5::DataSet datasetCollision;
    



public:
    explicit HDF5Writer(const std::string& outputFilename,float fov, int height, int width);
    void finalizeFile();
    void writeRecord(int collisionCount, float distance, Vec3 collisionLocation, Vec3 collisionDirection, int camera_x, int camera_y);
    
    void writeToFile(
        const std::vector<int>& collisionCount,
        const std::vector<float>& distance,
        const std::vector<Vec3>& collisionLocation,
        const std::vector<Vec3>& collisionDirection,
        sycl::queue& myQueue
    );

    void writeBatch(const std::vector<CollisionRecord>& records);
private:
    void initializeFile(float fov,int height,int width);
    

};

// Constructor
HDF5Writer::HDF5Writer(const std::string& outputFilename, float fov = 50, int height = 500, int width = 500)
    : filename(outputFilename), current_index(0),
      file(H5::H5File(outputFilename, H5F_ACC_TRUNC)) {
    initializeFile(fov, height, width);
}


void HDF5Writer::initializeFile(float fov = 50, int image_height = 500, int image_width = 500) {

    hsize_t init_size[1] = {0};  // Start with 0 records
    hsize_t max_size[1] = {H5S_UNLIMITED};  // Allow unlimited records

    H5::DataSpace space(1, init_size, max_size);

    // Define compound datatype
    H5::CompType compType(sizeof(CollisionRecord));

    compType.insertMember("CollisionCount", HOFFSET(CollisionRecord, collisionCount), H5::PredType::NATIVE_INT);
    compType.insertMember("Distance", HOFFSET(CollisionRecord, distance), H5::PredType::NATIVE_FLOAT);


    hsize_t vec3_dims[1] = {3};
    H5::ArrayType vec3Type(H5::PredType::NATIVE_FLOAT, 1, vec3_dims);
    
    compType.insertMember("CollisionLocation", HOFFSET(CollisionRecord, collisionLocation), vec3Type);
    compType.insertMember("CollisionDirection", HOFFSET(CollisionRecord, collisionDirection), vec3Type);


    compType.insertMember("Camera_x", HOFFSET(CollisionRecord, camera_x), H5::PredType::NATIVE_INT);
    compType.insertMember("Camera_y", HOFFSET(CollisionRecord, camera_y), H5::PredType::NATIVE_INT);
    // Enable chunking (for extendability)
    H5::DSetCreatPropList prop;
    hsize_t chunk_dims[1] = {100}; // Chunk size (can adjust based on expected record rate)
    prop.setChunk(1, chunk_dims);

    // Create dataset with unlimited size
    datasetCollision = file.createDataSet("CollisionData", compType, space, prop);

    // Create scalar dataspace for single values
    H5::DataSpace scalar_space = H5::DataSpace(H5S_SCALAR);

    file.createAttribute("FOV", H5::PredType::NATIVE_FLOAT, scalar_space).write(H5::PredType::NATIVE_FLOAT, &fov);
    file.createAttribute("ImageHeight", H5::PredType::NATIVE_INT, scalar_space).write(H5::PredType::NATIVE_INT, &image_height);
    file.createAttribute("ImageWidth", H5::PredType::NATIVE_INT, scalar_space).write(H5::PredType::NATIVE_INT, &image_width);
}



void HDF5Writer::writeRecord(int collisionCount, float distance, Vec3 collisionLocation, Vec3 collisionDirection, int camera_x, int camera_y) {

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


void HDF5Writer::writeBatch(const std::vector<CollisionRecord>& records) {
    if (records.empty()) return;

    hsize_t new_size[1] = { current_index + records.size() };
    datasetCollision.extend(new_size);

    hsize_t offset[1] = { current_index };
    hsize_t dims[1] = { records.size() };

    H5::DataSpace memspace(1, dims);
    H5::DataSpace dataspace = datasetCollision.getSpace();
    dataspace.selectHyperslab(H5S_SELECT_SET, dims, offset);

    datasetCollision.write(records.data(), datasetCollision.getCompType(), memspace, dataspace);
    current_index += records.size();
}


void HDF5Writer::finalizeFile() {
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






#pragma once

#include <H5Cpp.h>
#include "Vec.hpp"
#include <string>
#include <vector>
#include <unordered_map>

// Extracts --key value pairs from command-line arguments
std::unordered_map<std::string, std::string> parseFlags(int argc, char* argv[]) {
    std::unordered_map<std::string, std::string> args;

    for (int i = 1; i < argc - 1; ++i) {
        std::string key = argv[i];
        if (key.rfind("--", 0) == 0) { // starts with "--"
            std::string value = argv[i + 1];
            args[key] = value;
            ++i; // skip next since it's a value
        }
    }

    return args;
}




class HDF5Writer {
private:
    std::string filename;
    size_t current_index;
    H5::H5File file;
    H5::DataSet datasetCollision;
    
    struct CollisionRecord {
        int collisionCount;
        float distance;
        float collisionLocation[3];
        float collisionDirection[3];
    };

public:
    explicit HDF5Writer(const std::string& outputFilename);
    void finalizeFile();
    void writeRecord(int collisionCount, float distance, Vec3 collisionLocation, Vec3 collisionDirection);
private:
    void initializeFile();
};

// Constructor
HDF5Writer::HDF5Writer(const std::string& outputFilename)
    : filename(outputFilename), current_index(0),
      file(H5::H5File(outputFilename, H5F_ACC_TRUNC)) {
    initializeFile();
}


void HDF5Writer::initializeFile() {

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

    // Enable chunking (for extendability)
    H5::DSetCreatPropList prop;
    hsize_t chunk_dims[1] = {100}; // Chunk size (can adjust based on expected record rate)
    prop.setChunk(1, chunk_dims);

    // Create dataset with unlimited size
    datasetCollision = file.createDataSet("CollisionData", compType, space, prop);

}



void HDF5Writer::writeRecord(int collisionCount, float distance, Vec3 collisionLocation, Vec3 collisionDirection) {

    CollisionRecord record;
    record.collisionCount = collisionCount;
    record.distance = distance;
    record.collisionLocation[0] = collisionLocation.x;
    record.collisionLocation[1] = collisionLocation.y;
    record.collisionLocation[2] = collisionLocation.z;
    record.collisionDirection[0] = collisionDirection.x;
    record.collisionDirection[1] = collisionDirection.y;
    record.collisionDirection[2] = collisionDirection.z;

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


void HDF5Writer::finalizeFile() {
    file.close();
}

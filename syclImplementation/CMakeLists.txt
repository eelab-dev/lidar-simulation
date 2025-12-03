cmake_minimum_required(VERSION 3.22.1)


option(ENABLE_DEBUG "Enable Valgrind-based tests" OFF)
option(ENABLE_GPGPU "Enable GPGPU-based tests" ON)

project(LiDARSimulation)


if(ENABLE_DEBUG)
   message(STATUS "DEBUG FLAG IS ON")
   add_compile_options(-Wall -g -O0 -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fno-sanitize-recover=all  -static-libsan)
else()
   message(STATUS "DEBUG FLAG IS OFF")
   add_compile_options(-Wall) 
endif()



   # add_compile_options(-fsycl)
FIND_PACKAGE(IntelSYCL REQUIRED)
FIND_PACKAGE(HDF5 REQUIRED COMPONENTS C CXX)

if(ENABLE_GPGPU)
   set(SYCL_FLAGS "-fsycl"
         "-fsycl-targets=nvptx64-nvidia-cuda,spir64_x86_64"
         "-fsycl-unnamed-lambda"
         "-Wno-linker-warnings")
else()       
   set(SYCL_FLAGS "-fsycl"
      "-fsycl-targets=spir64_x86_64"
      "-fsycl-unnamed-lambda"
      "-Wno-linker-warnings")
endif()


GET_FILENAME_COMPONENT(PARENT_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)


ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/external/tinyusdz)



function(add_sycl_excutable target_name source_file)

ADD_EXECUTABLE(${target_name} ${source_file}) #${CMAKE_SOURCE_DIR}/build/external/tinyusdz/src external/tinyobjloader/tiny_obj_loader.cc)

TARGET_INCLUDE_DIRECTORIES(${target_name} PUBLIC 
   ${CMAKE_SOURCE_DIR}/include 
   ${CMAKE_SOURCE_DIR}/external/tinyusdz/src 
   ~/intel/oneapi/compiler/latest/linux/include 
   ${HDF5_INCLUDE_DIRS})

   if(ENABLE_DEBUG)
      TARGET_LINK_LIBRARIES(${target_name} PUBLIC tinyusdz::tinyusdz_static sycl ${SYCL_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fno-sanitize-recover=all  -static-libsan ${HDF5_CXX_LIBRARIES} ${HDF5_LIBRARIES})
   else()
      TARGET_LINK_LIBRARIES(${target_name} PUBLIC tinyusdz::tinyusdz_static sycl ${SYCL_FLAGS} ${HDF5_CXX_LIBRARIES} ${HDF5_LIBRARIES})
   endif()

   TARGET_COMPILE_OPTIONS(${target_name} PUBLIC ${SYCL_FLAGS})
endfunction()

set(LIDAR_GROUNDTRUTH_TARGET "LiDARGroundtruth")


add_sycl_excutable(${PROJECT_NAME}
   ${CMAKE_SOURCE_DIR}/lidarSimulation.cpp
)

add_sycl_excutable(${LIDAR_GROUNDTRUTH_TARGET}
   ${CMAKE_SOURCE_DIR}/lidarGroundTruth.cpp
)


message(STATUS "SYCL FLAG IS: ${SYCL_FLAGS}")
message(STATUS "ENABLE_GPGPU FLAG IS: ${ENABLE_GPGPU}")
message(STATUS "MAIN TARGET: ${PROJECT_NAME}")
message(STATUS "OTHER TARGET: ${LIDAR_GROUNDTRUTH_TARGET}")
# ADD_SYCL_TO_TARGET(TARGET ${PROJECT_NAME} SOURCES ${CMAKE_SOURCE_DIR} Test.cpp)


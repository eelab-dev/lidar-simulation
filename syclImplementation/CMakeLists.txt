cmake_minimum_required(VERSION 3.22.1)


option(ENABLE_DEBUG "Enable Valgrind-based tests" OFF)
option(ENABLE_GPGPU "Enable GPGPU-based tests" ON)


set(CMAKE_C_COMPILER /opt/intel/oneapi/compiler/latest/bin/icx)
set(CMAKE_CXX_COMPILER /opt/intel/oneapi/compiler/latest/bin/icpx)


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


if(ENABLE_DEBUG)
   ADD_SUBDIRECTORY(${PARENT_DIR}/external/tinyobjloader ${CMAKE_SOURCE_DIR}/debugBuild/external/tinyobjloader)
else()
   ADD_SUBDIRECTORY(${PARENT_DIR}/external/tinyobjloader ${CMAKE_SOURCE_DIR}/build/external/tinyobjloader)
endif()


ADD_EXECUTABLE(${PROJECT_NAME} ${CMAKE_SOURCE_DIR} Test.cpp) #external/tinyobjloader/tiny_obj_loader.cc)

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/build/external/tinyobjloader /opt/intel/oneapi/compiler/latest/linux/include ${HDF5_INCLUDE_DIRS})

if(ENABLE_DEBUG)
   TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC tinyobjloader sycl ${SYCL_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fno-sanitize-recover=all  -static-libsan ${HDF5_CXX_LIBRARIES} ${HDF5_LIBRARIES})
else()
   TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC tinyobjloader sycl ${SYCL_FLAGS} ${HDF5_CXX_LIBRARIES} ${HDF5_LIBRARIES})
endif()

TARGET_COMPILE_OPTIONS(${PROJECT_NAME} PUBLIC ${SYCL_FLAGS})
message(STATUS "SYCL FLAG IS: ${SYCL_FLAGS}")
message(STATUS "ENABLE_GPGPU FLAG IS: ${ENABLE_GPGPU}")
ADD_SYCL_TO_TARGET(TARGET ${PROJECT_NAME} SOURCES ${CMAKE_SOURCE_DIR} Test.cpp)


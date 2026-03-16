#include <stdio.h>
#include <math.h>
#include <limits>
#include <stdio.h>
#include <iostream>
#include "Vec.hpp"
#include "Camera.hpp"
#include <fstream>
#include "common.hpp"
#include "Utility.hpp"
#include "diffuseFileProcessor.hpp"
#include <chrono>
#include <sycl/sycl.hpp>
#include "syclScene.hpp"
#include <filesystem>

int main(int argc, char* argv[])
{
  std::string inputFile = "./Model/cornell_box.obj";
  std::string outputFile = "./diffuse_map.h5";
  int inputWidth = 100;
  int inputHeight = 100;
  float fov_x = 40.0f;
  float fov_y = 40.0f;
  int ssp = 1;
  unsigned int seed = 123;

  auto args = parseFlags(argc, argv);
  if (args.count("--model") && !args["--model"].empty()) inputFile = args["--model"][0];
  if (args.count("--output") && !args["--output"].empty()) outputFile = args["--output"][0];
  if (args.count("--width") && !args["--width"].empty()) inputWidth = std::stoi(args["--width"][0]);
  if (args.count("--height") && !args["--height"].empty()) inputHeight = std::stoi(args["--height"][0]);
  if (args.count("--fov_x") && !args["--fov_x"].empty()) fov_x = std::stof(args["--fov_x"][0]);
  if (args.count("--fov_y") && !args["--fov_y"].empty()) fov_y = std::stof(args["--fov_y"][0]);
  if (args.count("--ssp") && !args["--ssp"].empty()) ssp = std::stoi(args["--ssp"][0]);
  if (args.count("--seed") && !args["--seed"].empty()) seed = std::stoi(args["--seed"][0]);

  Vec3 cameraPosition(0.0f, 330.0f, 250 + 500 + 10);
  Vec3 lookAt(0.0f, 274.0f, 0.0f);
  Vec3 up(0.0f, 1.0f, 0.0f);

  try {
    if (args.count("--cameraPosition")) {
      const auto& pos_vals = args["--cameraPosition"];
      if (pos_vals.size() != 3) {
        throw std::runtime_error("--cameraPosition requires 3 float values (x y z)");
      }
      cameraPosition = Vec3(std::stof(pos_vals[0]), std::stof(pos_vals[1]), std::stof(pos_vals[2]));
    }

    if (args.count("--lookAt")) {
      const auto& look_vals = args["--lookAt"];
      if (look_vals.size() != 3) {
        throw std::runtime_error("--lookAt requires 3 float values (x y z)");
      }
      lookAt = Vec3(std::stof(look_vals[0]), std::stof(look_vals[1]), std::stof(look_vals[2]));
    }

    if (args.count("--up")) {
      const auto& up_vals = args["--up"];
      if (up_vals.size() != 3) {
        throw std::runtime_error("--up requires 3 float values (x y z)");
      }
      up = Vec3(std::stof(up_vals[0]), std::stof(up_vals[1]), std::stof(up_vals[2]));
    }
  } catch (const std::exception& e) {
    std::cerr << "Error parsing arguments: " << e.what() << std::endl;
  }

  size_t pos = inputFile.find_last_of('/');
  std::string ModelDir = inputFile.substr(0, pos);
  std::string ModelName = inputFile.substr(pos);

  float detectorWidth = 20;
  float detectorHeight = 20;

  auto iterationSize = computeAdjustedSize(inputWidth, inputHeight);
  int imageWidth = iterationSize.first;
  int imageHeight = iterationSize.second;

  int widthUnit = imageWidth / inputWidth;
  int heightUnit = imageHeight / inputHeight;

  OBJ_Loader loader;
  loader.addTriangleUSDFile(ModelDir, ModelName);
  Camera camera(imageWidth, imageHeight, fov_x, fov_y, cameraPosition, lookAt, up, detectorWidth, detectorHeight);
  loader.addCamera(&camera);

  Triangle_OBJ_result TriangleResult = loader.outputTrangleResult();
  sycl::queue myQueue(sycl::gpu_selector_v);

  ObjectListContent sceneObjListContent(myQueue);
  sceneObjListContent.addObject(TriangleResult);
  ObjectList sceneObject;
  sceneObject.setObjects(sceneObjListContent);

  syclScene scene(sceneObject);
  scene.commit();
  sycl::buffer<syclScene, 1> scenebuf(&scene, sycl::range<1>(1));

  const size_t pixelCount = static_cast<size_t>(inputWidth) * static_cast<size_t>(inputHeight);
  std::vector<float> diffuseSum(pixelCount, 0.0f);
  std::vector<int> diffuseCount(pixelCount, 0);

  sycl::buffer<float> diffuse_sum_buf(diffuseSum);
  sycl::buffer<int> diffuse_count_buf(diffuseCount);
  sycl::buffer<Camera, 1> camerabuf(&camera, sycl::range<1>(1));

  myQueue.wait_and_throw();

  auto startTime = std::chrono::high_resolution_clock::now();
  std::cout << "Running on " << myQueue.get_device().get_info<sycl::info::device::name>() << std::endl;

  myQueue.submit([&](sycl::handler& cgh) {
    auto sceneAcc = scenebuf.template get_access<sycl::access::mode::read>(cgh);
    auto cameraAcc = camerabuf.template get_access<sycl::access::mode::read>(cgh);

    sycl::accessor diffuse_sum_acc(diffuse_sum_buf, cgh, sycl::read_write);
    sycl::accessor diffuse_count_acc(diffuse_count_buf, cgh, sycl::read_write);

    cgh.parallel_for(sycl::range<2>(imageWidth, imageHeight), [=](sycl::id<2> index) {
      int i = index[0];
      int j = index[1];

      for (int s = 0; s < ssp; ++s)
      {
        RNG rng(seed + i + j * imageWidth + s * ssp);
        Vec3 rayDir = cameraAcc[0].getRayDirection(i, j, rng);
        Ray ray(cameraAcc[0].getPosition(), rayDir);
        auto tem = sceneAcc[0].doDiffuseMap(ray, rng);

        if (tem._hit && tem._diffuseValue >= 0.0f)
        {
          const int camera_x = (inputWidth - 1) - (i / widthUnit);
          const int camera_y = (inputHeight - 1) - (j / heightUnit);
          if (camera_x < 0 || camera_x >= inputWidth || camera_y < 0 || camera_y >= inputHeight) {
            continue;
          }
          const int pixel_idx = camera_x * inputHeight + camera_y;

          auto sum_atomic = sycl::atomic_ref<
            float,
            sycl::ext::oneapi::detail::memory_order::relaxed,
            sycl::ext::oneapi::detail::memory_scope::device,
            sycl::access::address_space::global_space>(diffuse_sum_acc[pixel_idx]);
          auto count_atomic = sycl::atomic_ref<
            int,
            sycl::ext::oneapi::detail::memory_order::relaxed,
            sycl::ext::oneapi::detail::memory_scope::device,
            sycl::access::address_space::global_space>(diffuse_count_acc[pixel_idx]);
          sum_atomic.fetch_add(tem._diffuseValue);
          count_atomic.fetch_add(1);
        }
      }
    });
  });

  myQueue.wait_and_throw();
  myQueue.update_host(diffuse_sum_buf.get_access());
  myQueue.update_host(diffuse_count_buf.get_access());

  diffuseHDF5Writer writer(outputFile, fov_x, fov_y, inputWidth, inputHeight);
  writer.writeGrid(diffuseSum, diffuseCount, inputWidth, inputHeight);
  writer.finalizeFile();

  auto endTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> executionTime = endTime - startTime;
  std::cout << "Rendering time = "
            << (std::chrono::duration_cast<std::chrono::milliseconds>(executionTime).count()) / 1000.0f
            << "s" << std::endl;
  size_t validPixelNum = 0;
  for (size_t idx = 0; idx < pixelCount; ++idx) {
    if (diffuseCount[idx] > 0) {
      validPixelNum++;
    }
  }
  std::cout << "Valid diffuse pixels: " << validPixelNum << "/" << pixelCount << std::endl;

  return 0;
}

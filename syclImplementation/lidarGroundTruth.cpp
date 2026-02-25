#include <stdio.h>
#include <math.h>
#include <limits>
#include <stdio.h>
// #include <tiny_obj_loader.h>
#include <iostream>
#include "Vec.hpp"
#include "Camera.hpp" 
#include <fstream>
#include "common.hpp"
#include "Utility.hpp"
#include "pixelizedFileProcessor.hpp"
#include <chrono>
#include <sycl/sycl.hpp>
#include "syclScene.hpp" 
#include <filesystem>



int main(int argc, char* argv[])
{
  std::string inputFile = "./Model/cornell_box.obj";
  std::string outputFile = "./outputtest.h5";
  int inputWidth = 100;
  int inputHeight = 100;
  float fov_x = 40.0f; // Field of view in degrees
  float fov_y = 40.0f; // Field of view in degrees
  int ssp = 500*1000*25;
  unsigned int seed = 123; 



  auto args = parseFlags(argc, argv);
  // Handle single-value flags
  if (args.count("--model") && !args["--model"].empty()) inputFile = args["--model"][0];
  if (args.count("--output") && !args["--output"].empty()) outputFile = args["--output"][0];
  if (args.count("--width") && !args["--width"].empty()) inputWidth = std::stoi(args["--width"][0]);
  if (args.count("--height") && !args["--height"].empty()) inputHeight = std::stoi(args["--height"][0]);
  if (args.count("--fov_x") && !args["--fov_x"].empty()) fov_x = std::stof(args["--fov_x"][0]);
  if (args.count("--fov_y") && !args["--fov_y"].empty()) fov_y = std::stof(args["--fov_y"][0]);
  if (args.count("--ssp") && !args["--ssp"].empty()) ssp = std::stoi(args["--ssp"][0]);
  if (args.count("--seed") && !args["--seed"].empty()) seed = std::stoi(args["--seed"][0]);

  Vec3 cameraPosition(0.0f, 330.0f, 250 + 500 + 10); // Example camera position
  Vec3 lookAt(0.0f, 274.0f, 0.0f); // Look at the center of the Cornell Box
  Vec3 up(0.0f, 1.0f, 0.0f); // Up direction


  // --- Handle Multi-Value Flags for Camera ---
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

      if (args.count("--up"))
      {
        const auto& up_vals = args["--up"];
        
        if (up_vals.size() != 3) {
            throw std::runtime_error("--up requires 3 float values (x y z)");
        }
        up = Vec3(std::stof(up_vals[0]), std::stof(up_vals[1]), std::stof(up_vals[2]));
      }
  } catch (const std::exception& e) {
      std::cerr << "Error parsing arguments: " << e.what() << std::endl;
  }

  // Extract directory and file name
  size_t pos = inputFile.find_last_of('/');
  std::string ModelDir = inputFile.substr(0, pos);
  std::string ModelName = inputFile.substr(pos);
  
  float detectorWidth = 20;
  float detectorHeight = 20;

  auto iterationSize = computeAdjustedSize(inputWidth,inputHeight);
  int imageWidth = iterationSize.first;
  int imageHeight = iterationSize.second;

  int widthUnit = imageWidth/inputWidth;
  int heightUnit = imageHeight/inputHeight;

  std::cout << widthUnit << " " << heightUnit << std::endl;

  OBJ_Loader loader;
  loader.addTriangleUSDFile(ModelDir, ModelName);  
  Camera camera(imageWidth, imageHeight, fov_x, fov_y, cameraPosition, lookAt, up, detectorWidth, detectorHeight);
  loader.addCamera(&camera);


  Triangle_OBJ_result TriangleResult = loader.outputTrangleResult();
  std::cout << "hello from GPGPU\n" <<std::endl;
  sycl::queue myQueue(sycl::gpu_selector_v);

  ObjectListContent sceneObjListContent(myQueue);
  sceneObjListContent.addObject(TriangleResult);
  ObjectList sceneObject;
  sceneObject.setObjects(sceneObjListContent);

  syclScene scene(sceneObject);
  scene.commit();
  sycl::buffer<syclScene, 1> scenebuf(&scene, sycl::range<1>(1));


  constexpr size_t recordSize = 640*120*800;


  std::vector<CollisionRecord> collision(recordSize);
  int recordNum = 0;
  std::cout << "Running on " << myQueue.get_device().get_info<sycl::info::device::name>() << std::endl;

  sycl::buffer<CollisionRecord> collision_buf(collision);
  sycl::buffer<int, 1> counter_buf(&recordNum, sycl::range<1>(1));
  sycl::buffer<Camera, 1> camerabuf(&camera, sycl::range<1>(1));

  myQueue.wait_and_throw();

  auto startTime = std::chrono::high_resolution_clock::now();
  std::cout << "submitting kernel\n";

  myQueue.submit([&](sycl::handler& cgh) {
  sycl::stream out(1024, 256, cgh);
  auto sceneAcc = scenebuf.template get_access<sycl::access::mode::read>(cgh);
  auto cameraAcc = camerabuf.template get_access<sycl::access::mode::read>(cgh);

  sycl::accessor counter_acc(counter_buf, cgh, sycl::write_only);
  sycl::accessor collision_acc(collision_buf, cgh, sycl::write_only);

  cgh.parallel_for(sycl::range<2>(imageWidth, imageHeight), [=](sycl::id<2> index) 
  {
    int i = index[0];
    int j = index[1];

    for (int s = 0; s < ssp; ++s) 
    {
      RNG rng(seed + i + j * imageWidth + s *ssp);
      Vec3 rayDir = cameraAcc[0].getRayDirection(i, j, rng); 
      Ray ray(cameraAcc[0].getPosition(), rayDir); 
      auto tem = sceneAcc[0].doGroundTruth(ray, rng);
      // out << ray.direction.x << " " << ray.direction.y << " " << ray.direction.z << sycl::endl;
      // if (tem._collisionCount !=0){
      //   out << tem._collisionCount<< sycl::endl;
      // } 
      if(tem._hit)
      {
        
          auto v_counter = sycl::atomic_ref<
              int,
              sycl::ext::oneapi::detail::memory_order::relaxed,
              sycl::ext::oneapi::detail::memory_scope::device,
              sycl::access::address_space::global_space>(counter_acc[0]);

        int idx = v_counter.fetch_add(1);  
        if (idx < static_cast<int>(recordSize))
        {
          collision_acc[idx].collisionCount = tem._collisionCount;
          collision_acc[idx].distance = tem._travelDistance*2; 
          collision_acc[idx].collisionLocation = tem._position;
          collision_acc[idx].collisionDirection = cameraAcc[0].toCameraBase(tem._direction);
          collision_acc[idx].camera_x = (inputWidth-1) - (i/widthUnit);
          collision_acc[idx].camera_y = (inputHeight -1 ) - (j/heightUnit);

          collision_acc[idx].emission_delay = tem._emission_delay;
        }
      }
    }

    });
  });


myQueue.wait_and_throw();
myQueue.update_host(counter_buf.get_access());
myQueue.update_host(collision_buf.get_access());
std::cout << "finished rendering" << std::endl;

pixelizedHDF5Writer writer(outputFile, fov_x, fov_y, inputWidth, inputHeight);


if (collision.size() > recordNum) {
    collision.resize(recordNum);
}
std::cout << "Final value of the shared counter: " << recordNum<< std::endl;
// sycl::queue HDF5WriterQueue(sycl::cpu_selector_v);
// HDF5WriterQueue.wait_and_throw();
writer.writeBatch(collision, inputWidth, inputHeight);
writer.finalizeFile();



auto endTime = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> executionTime = endTime - startTime;
std::cout << "Rendering time = " << (std::chrono::duration_cast<std::chrono::milliseconds>(executionTime).count())/1000.0f << "s" << std::endl;



return 0;



}

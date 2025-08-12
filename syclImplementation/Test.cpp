#include <stdio.h>
#include <math.h>
#include <limits>
#include <stdio.h>
#include <tiny_obj_loader.h>
#include <iostream>
#include "Vec.hpp"
#include "Camera.hpp" 
#include <fstream>
#include "common.hpp"
#include "Utility.hpp"
#include "FileProcessor.hpp"
#include <chrono>
#include <sycl/sycl.hpp>
#include "syclScene.hpp" 
#include <filesystem>


int main(int argc, char* argv[]){
  
  std::string inputFile = "./Model/cornell_box.obj";
  std::string outputFile = "./outputtest.h5";
  int inputWidth = 100;
  int inputHeight = 100;
  float fov = 40.0f; // Field of view in degrees
  int ssp = 500*1000*25;
  unsigned int seed = 123;  
  float detectorDistance = 560;
  float detectorWidth = 20;
  float detectorHeight = 20;
  myComputeType delay_mean = 44.52;
  myComputeType delay_std = 195;

  delay_std = 395;
  // // Parse flags
  // std::unordered_map<std::string, std::string> args = parseFlags(argc, argv);

  // // Handle known flags
  // if (args.count("--model")) inputFile = args["--model"];
  // if (args.count("--output")) outputFile = args["--output"];
  // if (args.count("--width")) inputWidth = std::stoi(args["--width"]);
  // if (args.count("--height")) inputHeight = std::stoi(args["--height"]);
  // if (args.count("--fov")) fov = std::stof(args["--fov"]);
  // if (args.count("--ssp")) ssp = std::stoi(args["--ssp"]);
  // if (args.count("--seed")) seed = std::stoi(args["--seed"]);
  // if (args.count("--detectorDistance")) detectorDistance = std::stof(args["--detectorDistance"]);  

  // --- Parse Command-Line Arguments ---
  auto args = parseFlags(argc, argv);

  // Handle single-value flags
  if (args.count("--model") && !args["--model"].empty()) inputFile = args["--model"][0];
  if (args.count("--output") && !args["--output"].empty()) outputFile = args["--output"][0];
  if (args.count("--width") && !args["--width"].empty()) inputWidth = std::stoi(args["--width"][0]);
  if (args.count("--height") && !args["--height"].empty()) inputHeight = std::stoi(args["--height"][0]);
  if (args.count("--fov") && !args["--fov"].empty()) fov = std::stof(args["--fov"][0]);
  if (args.count("--ssp") && !args["--ssp"].empty()) ssp = std::stoi(args["--ssp"][0]);
  if (args.count("--seed") && !args["--seed"].empty()) seed = std::stoi(args["--seed"][0]);
  if (args.count("--detectorDistance") && !args["--detectorDistance"].empty()) detectorDistance = std::stof(args["--detectorDistance"][0]);

  if (args.count("--detectorWidth") && !args["--detectorWidth"].empty()) detectorWidth= std::stof(args["--detectorWidth"][0]);
  if (args.count("--detectorHeight") && !args["--detectorHeight"].empty()) detectorHeight= std::stof(args["--detectorHeight"][0]);
  
  Vec3 cameraPosition(0.0f, 330.0f, 250 + detectorDistance + 10); // Example camera position
  Vec3 lookAt(0.0f, 274.0f, 0.0f); // Look at the center of the Cornell Box

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
    } catch (const std::exception& e) {
        std::cerr << "Error parsing arguments: " << e.what() << std::endl;
    }

  // Extract directory and file name
  size_t pos = inputFile.find_last_of('/');
  std::string ModelDir = inputFile.substr(0, pos);
  std::string ModelName = inputFile.substr(pos);


  Vec3 up(0.0f, 1.0f, 0.0f); // Up direction

  auto iterationSize = computeAdjustedSize(inputWidth,inputHeight);
  int imageWidth = iterationSize.first;
  int imageHeight = iterationSize.second;

  int widthUnit = imageWidth/inputWidth;
  int heightUnit = imageHeight/inputHeight;

  std::cout << widthUnit << " " << heightUnit << std::endl;

  OBJ_Loader loader;
  loader.addTriangleObjectFile(ModelDir, ModelName);  
  Camera camera(imageWidth, imageHeight, fov, cameraPosition, lookAt, up, detectorWidth, detectorHeight);
  loader.addCamera(&camera);
 
Triangle_OBJ_result TriangleResult = loader.outputTrangleResult();
std::cout << "hello from GPGPU\n" <<std::endl;
sycl::queue myQueue(sycl::gpu_selector_v);

ObjectListContent sceneObjListContent(myQueue);
sceneObjListContent.addObject(TriangleResult.Triangles, TriangleResult.MaterialsInfoList, TriangleResult.materialIDs);
ObjectList sceneObject;
sceneObject.setObjects(sceneObjListContent);

syclScene scene(sceneObject);
scene.commit();
sycl::buffer<syclScene, 1> scenebuf(&scene, sycl::range<1>(1));

constexpr size_t recordSize = 640*120*8000;


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
    myComputeType delay_distance = sample_delay_distance(delay_mean,delay_std,rng);
    auto tem = sceneAcc[0].doRendering(ray, rng, delay_distance);
    
    // out << ray.direction.x << " " << ray.direction.y << " " << ray.direction.z << sycl::endl;
    // if (tem._collisionCount !=0){
    //   out << tem._collisionCount<< sycl::endl;
    // } 
    if(tem._hit)
    {
      
      // auto v_counter = sycl::atomic_ref<int, sycl::ext::oneapi::detail::memory_order::relaxed,sycl::ext::oneapi::detail::memory_scope::device, sycl::access::address_space::global_space>(counter_acc[0]);
      // int index = v_counter;
        auto v_counter = sycl::atomic_ref<
            int,
            sycl::ext::oneapi::detail::memory_order::relaxed,
            sycl::ext::oneapi::detail::memory_scope::device,
            sycl::access::address_space::global_space>(counter_acc[0]);

      int idx = v_counter.fetch_add(1);  
      collision_acc[idx].collisionCount = tem._collisionCount;
      collision_acc[idx].distance = tem._travelDistance;
      collision_acc[idx].collisionLocation = tem._position;
      collision_acc[idx].collisionDirection = cameraAcc[0].toCameraBase(tem._direction);
      collision_acc[idx].camera_x = i/widthUnit;
      collision_acc[idx].camera_y = j/heightUnit;
    }
  }

  });
});
myQueue.wait_and_throw();
myQueue.update_host(counter_buf.get_access());
myQueue.update_host(collision_buf.get_access());
std::cout << "finished rendering" << std::endl;


HDF5Writer writer(outputFile, fov, imageHeight, imageWidth);


if (collision.size() > recordNum) {
    collision.resize(recordNum);
}

sycl::queue HDF5WriterQueue(sycl::cpu_selector_v);
HDF5WriterQueue.wait_and_throw();
// auto filterRecord = filterCollisionRecordsSYCL(collision,HDF5WriterQueue);
// writer.writeBatch(filterRecord);
writer.writeBatch(collision);
writer.finalizeFile();



auto endTime = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> executionTime = endTime - startTime;
std::cout << "Rendering time = " << (std::chrono::duration_cast<std::chrono::milliseconds>(executionTime).count())/1000.0f << "s" << std::endl;
std::cout << "Final value of the shared counter: " << recordNum<< std::endl;


return 0;
}

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
#include "FileProcessor.hpp"

#include <chrono>
#include <sycl/sycl.hpp>

#include "syclScene.hpp" 
#include <filesystem>






int compoentToint(myComputeType x){
return (int)(255*(std::pow(clamp(x, 0.0, 1.0), 1/2.2)));
//return (int)(x);  
}


int main(int argc, char* argv[]){
  
  std::string inputFile = "./Model/cornell_box.obj";
  std::string outputFile = "./outputtest.txt";
  int imageWidth = 500;
  int imageHeight = 500;
  float fov = 40.0f; // Field of view in degrees
  int ssp = 500*1000;
  unsigned int seed = 123;  

  // Parse flags
  std::unordered_map<std::string, std::string> args = parseFlags(argc, argv);

  // Handle known flags
  if (args.count("--model")) inputFile = args["--model"];
  if (args.count("--output")) outputFile = args["--output"];
  if (args.count("--width")) imageWidth = std::stoi(args["--width"]);
  if (args.count("--height")) imageHeight = std::stoi(args["--height"]);
  if (args.count("--fov")) fov = std::stof(args["--fov"]);
  if (args.count("--ssp")) ssp = std::stoi(args["--ssp"]);
  if (args.count("--seed")) seed = std::stoi(args["--seed"]);


  // Extract directory and file name
  size_t pos = inputFile.find_last_of('/');
  std::string ModelDir = inputFile.substr(0, pos);
  std::string ModelName = inputFile.substr(pos);



  OBJ_Loader loader;
  loader.addTriangleObjectFile(ModelDir, ModelName);

  Triangle_OBJ_result TriangleResult = loader.outputTrangleResult();

  Vec3 cameraPosition(20.0f, 274.0f, 1280.0f); // Example camera position
  Vec3 lookAt(0.0f, 274.0f, 0.0f); // Look at the center of the Cornell Box
  Vec3 up(0.0f, 1.0f, 0.0f); // Up direction


  Camera camera(imageWidth, imageHeight, fov, cameraPosition, lookAt, up);

 





std::cout << "hello from GPGPU\n" <<std::endl;
sycl::queue myQueue(sycl::gpu_selector_v);
//auto sceneObject = loader.outputSyclObj(myQueue);

ObjectListContent sceneObjListContent(myQueue);
sceneObjListContent.addObject(TriangleResult.Triangles, TriangleResult.MaterialsInfoList, TriangleResult.materialIDs);
ObjectList sceneObject;
sceneObject.setObjects(sceneObjListContent);



syclScene scene(sceneObject);
scene.commit();
sycl::buffer<syclScene, 1> scenebuf(&scene, sycl::range<1>(1));


// constexpr size_t vector_size = 1000;
// std::vector<Vec3>
constexpr size_t recordSize = 640*120*8000;



std::vector<Vec3> directionRecord(recordSize);
std::vector<Vec3> positionRecord(recordSize);
std::vector<int> collisionRecord(recordSize);
std::vector<float> travelDistanceRecord(recordSize);
int recordNum = 0;



sycl::buffer<Vec3, 1> direction_buf(directionRecord.data(), sycl::range<1>(recordSize));
sycl::buffer<Vec3, 1> position_buf(positionRecord.data(), sycl::range<1>(recordSize));
sycl::buffer<int, 1> counter_buf(&recordNum, sycl::range<1>(1));
sycl::buffer<int, 1> collision_buf(collisionRecord.data(), sycl::range<1>(recordSize));
sycl::buffer<float,1> travelDistance_buf(travelDistanceRecord.data(), sycl::range<1>(recordSize));


std::cout << "Running on " << myQueue.get_device().get_info<sycl::info::device::name>() << std::endl;

std::vector<Vec3> image(imageWidth * imageHeight);
sycl::buffer<Vec3, 1> imagebuf(image.data(), sycl::range<1>(image.size()));
sycl::buffer<Camera, 1> camerabuf(&camera, sycl::range<1>(1));

myQueue.wait_and_throw();

auto startTime = std::chrono::high_resolution_clock::now();
std::cout << "submitting kernel\n";

myQueue.submit([&](sycl::handler& cgh) {
  //sycl::stream out(imageWidth, imageHeight, cgh);  
sycl::stream out(1024, 256, cgh);
auto sceneAcc = scenebuf.template get_access<sycl::access::mode::read>(cgh);
auto imageAcc = imagebuf.template get_access<sycl::access::mode::write>(cgh);
auto cameraAcc = camerabuf.template get_access<sycl::access::mode::read>(cgh);

sycl::accessor counter_acc(counter_buf, cgh, sycl::write_only);
sycl::accessor direction_acc(direction_buf, cgh, sycl::write_only);
sycl::accessor position_acc(position_buf, cgh, sycl::write_only);
sycl::accessor collision_acc(collision_buf, cgh, sycl::write_only);
sycl::accessor travelDistance_acc(travelDistance_buf, cgh, sycl::write_only);


cgh.parallel_for(sycl::range<2>(imageWidth, imageHeight), [=](sycl::id<2> index) 
{
  int i = index[0];
  int j = index[1];
  Vec3 pixelColor(0.0f, 0.0f, 0.0f);


  
  //sycl::atomic_ref<int, sycl::ext::oneapi::detail::memory_order::relaxed, sycl::ext::oneapi::detail::memory_scope::device, sycl::access::address_space::global_space> atomic_counter(counter_acc[0]);
  
  for (int s = 0; s < ssp; ++s) 
  {
    RNG rng(seed + i + j * imageWidth + s *ssp);
    Vec3 rayDir = cameraAcc[0].getRayDirection(i, j, rng); 
    Ray ray(cameraAcc[0].getPosition(), rayDir); 
    auto tem = sceneAcc[0].doRendering(ray, rng);
    //pixelColor = pixelColor + tem;
    if(tem._hit)
    {
      
      // auto v_counter = sycl::atomic_ref<int, sycl::ext::oneapi::detail::memory_order::relaxed,sycl::ext::oneapi::detail::memory_scope::device, sycl::access::address_space::global_space>(counter_acc[0]);
      // int index = v_counter;
        auto v_counter = sycl::atomic_ref<
            int,
            sycl::ext::oneapi::detail::memory_order::relaxed,
            sycl::ext::oneapi::detail::memory_scope::device,
            sycl::access::address_space::global_space>(counter_acc[0]);

      int index = v_counter.fetch_add(1);  
      auto v_positionX = sycl::atomic_ref<float, sycl::ext::oneapi::detail::memory_order::relaxed,sycl::ext::oneapi::detail::memory_scope::device, sycl::access::address_space::global_space>(position_acc[index].x);
      auto v_positionY = sycl::atomic_ref<float, sycl::ext::oneapi::detail::memory_order::relaxed,sycl::ext::oneapi::detail::memory_scope::device, sycl::access::address_space::global_space>(position_acc[index].y);
      auto v_positionZ = sycl::atomic_ref<float, sycl::ext::oneapi::detail::memory_order::relaxed,sycl::ext::oneapi::detail::memory_scope::device, sycl::access::address_space::global_space>(position_acc[index].z);

      auto v_directionX = sycl::atomic_ref<float, sycl::ext::oneapi::detail::memory_order::relaxed,sycl::ext::oneapi::detail::memory_scope::device, sycl::access::address_space::global_space>(direction_acc[index].x);
      auto v_directionY = sycl::atomic_ref<float, sycl::ext::oneapi::detail::memory_order::relaxed,sycl::ext::oneapi::detail::memory_scope::device, sycl::access::address_space::global_space>(direction_acc[index].y);
      auto v_directionZ = sycl::atomic_ref<float, sycl::ext::oneapi::detail::memory_order::relaxed,sycl::ext::oneapi::detail::memory_scope::device, sycl::access::address_space::global_space>(direction_acc[index].z);

      auto v_distance = sycl::atomic_ref<float, sycl::ext::oneapi::detail::memory_order::relaxed,sycl::ext::oneapi::detail::memory_scope::device, sycl::access::address_space::global_space>(travelDistance_acc[index]);
      auto v_collision = sycl::atomic_ref<int, sycl::ext::oneapi::detail::memory_order::relaxed,sycl::ext::oneapi::detail::memory_scope::device, sycl::access::address_space::global_space>(collision_acc[index]);

      Vec3 position = tem._position;
      Vec3 direction = tem._direction;

      v_positionX = position.x;
      v_positionY = position.y;
      v_positionZ = position.z;

      v_directionX = direction.x;
      v_directionY = direction.y;
      v_directionZ = direction.z;

      v_collision = tem._collisionCount;  
      v_distance = tem._travelDistance;
    }
  }

  //imageAcc[i + j * imageWidth] = pixelColor/ssp;

  });
});
myQueue.wait_and_throw();
std::cout << "done\n";
myQueue.update_host(imagebuf.get_access());
myQueue.update_host(counter_buf.get_access());
myQueue.update_host(direction_buf.get_access());
myQueue.update_host(position_buf.get_access());
myQueue.update_host(collision_buf.get_access());
myQueue.update_host(travelDistance_buf.get_access());
std::cout << "finished rendering" << std::endl;


HDF5Writer writer(outputFile, fov, imageHeight, imageWidth);

for(int i = 0;i<recordNum;i++){


  if(collisionRecord[i]>0){
    writer.writeRecord(collisionRecord[i],travelDistanceRecord[i],positionRecord[i],directionRecord[i]);
  }

}

writer.finalizeFile();



auto endTime = std::chrono::high_resolution_clock::now();
std::chrono::duration<double> executionTime = endTime - startTime;
std::cout << "Rendering time = " << (std::chrono::duration_cast<std::chrono::milliseconds>(executionTime).count())/1000.0f << "s" << std::endl;
std::cout << "Final value of the shared counter: " << recordNum<< std::endl;


return 0;
}

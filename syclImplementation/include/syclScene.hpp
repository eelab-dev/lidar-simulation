#pragma once

#include "Vec.hpp"
#include "Ray.hpp"
#include "Intersection.hpp"
#include "Material.hpp"
#include <vector>   
#include <string>
#include <cmath>
#include <iostream>
// #include "BVHArray.hpp"
// #include "sycl_obj_loader.hpp"
#include "ObjectList.hpp"
#include "usd_obj_loader.hpp"


class syclScene
{

    private:

        ObjectList _sceneObject; 
        //BVHArray* _bvh = nullptr;

    public:
        
        ~syclScene()
        {
            //delete _bvh;
        }

        syclScene(ObjectList sceneObject): _sceneObject(sceneObject)
        {
        }


 

        syclScene(const syclScene& scene) = delete;

        
//        BVHArray *_bvh = nullptr;
        void buildBVH();

//         SamplingRecord sampleLight(RNG &rng) const
//         {

//             size_t objectsListSize = _sceneObject.getObjectsListSize();   
                      
//             float emitArea = 0;
//             for (size_t i = 0; i < objectsListSize; i++)
//             {
//                 const Material* curMaterial = _sceneObject.getMaterial(i);
//                 if (curMaterial->getEmission())
//                 {
// //                    emitArea += _objectsList[i]->getArea();
//                       emitArea = _sceneObject.getArea(i);
//                 }
//             }

//             float p = std::abs(get_random_float(rng)) * emitArea;
//             float area = 0;

//             for (size_t i = 0; i < objectsListSize; i++)
//             {
//                 const Material* curMaterial = _sceneObject.getMaterial(i);
//                 if (curMaterial->getEmission())
//                 {
//                     area = area + _sceneObject.getArea(i);
//                     if (area >= p){
//                     return _sceneObject.Sample(rng,i);
//                         //pdf /= emitArea;
                        
//                     }
//                 }
//             }

//             return SamplingRecord();


//         }

        
        resultRecordStructure doGroundTruth(const Ray &initialRay, RNG &rng) const
        {

            resultRecordStructure result;
            result._collisionCount = 0;
            result._hit = false;
            result._travelDistance = 0;


            Intersection intersection = castRay(initialRay);
            if (!intersection._hit)
            {
                result._hit = false;
                return result;
            }


            result._hit = true;
            result._position = intersection._position;
            result._direction = initialRay.direction;
            result._travelDistance = result._travelDistance + (intersection._position - initialRay.origin).length();
            result._collisionCount++;
            return result;


        }

        diffuseRecordStructure doDiffuseMap(const Ray &initialRay, RNG &rng) const
        {
            diffuseRecordStructure result;
            result._hit = false;
            result._diffuseValue = -1.0f;

            Intersection intersection = castRay(initialRay);
            if (!intersection._hit)
            {
                return result;
            }

            float diffuseValue = _sceneObject.getMaterialDiffuse(intersection._objectIndex);
            if (diffuseValue < 0.0f)
            {
                return result;
            }

            result._hit = true;
            result._diffuseValue = diffuseValue;
            return result;
        }

        
        resultRecordStructure doSimulation(const Ray &initialRay, RNG &rng) const
        {

            int maxDepth = 10;
            int depth = 0;
            resultRecordStructure result;
            Ray temRay = initialRay;
            Ray &currentRay = temRay;
            result._collisionCount = 0;
            result._hit = false;
            result._travelDistance = 0;
    
            
            for (depth = 0; depth < maxDepth; ++depth)
            {

                
                Intersection intersection = castRay(currentRay);

                if (!intersection._hit)
                {
                    result._hit = false;
                    return result;
                }
                auto intersectionID = intersection._objectIndex;
                const Material* intersectionMaterial = _sceneObject.getMaterial(intersectionID);
                if (intersectionMaterial == nullptr) {
                    result._hit = false;
                    return result;
                }


                result._travelDistance = result._travelDistance + (intersection._position - currentRay.origin).length();
                result._collisionCount++;
                


                if (intersectionMaterial->isDetector())
                {

                    result._hit = true;
                    result._position = intersection._position;
                    result._direction = currentRay.direction;
                    
                    return result;
                }

                float diffuse_factor = clamp(intersectionMaterial->_reflectivity, 0.0f, 1.0f);
                if(get_random_float(rng) > diffuse_factor)
                {
                    result._hit = false;
                    return result;
                }
     

                const float EPSILON = 1e-5f;
                Vec3 normal = intersection._normal.normalized();
                Vec3 offset = normal * EPSILON;
                if (dotProduct(currentRay.direction, normal) > 0.0f) {
                    offset = -offset;
                }
                Vec3 safeOrigin = intersection._position + offset;
                Vec3 newDirection = intersectionMaterial->sample(currentRay.direction, normal, rng);
                currentRay = Ray(safeOrigin, newDirection);           
            }

            return result;
        } 


        void commit()
        {
            std::cout << "building tree " << " object size " << _sceneObject.getObjectsListSize() <<std::endl;
            //this->_bvh = new BVHAccel(_sceneObject, _sceneObject->getObjectsListSize());
            //std::cout << "The Tree size is  " << countTreeNodeSize(_bvh->root) <<std::endl;
            //this->_bvh = new BVHArray(_sceneObject, _myQueue);
        }

        // Intersection castRay(Ray inputRay) const
        // {
        //     Intersection result;
        //     float t;
        //     float t_min = INFINITY;
        //     size_t objectsListSize = _sceneObject.getObjectsListSize();
        //     for (size_t i = 0; i < objectsListSize; i++)
        //     {
        //         auto intersection = _sceneObject.getIntersection(inputRay,i);
        //         if(intersection._hit)
        //         {
        //             t = intersection._distance;
        //             if(t<t_min)
        //             {
        //                 t_min = t;
        //                 result = intersection;
        //             }
        //         }
        //     }
        //     return result;   
        // }


        Intersection castRay(const Ray &ray) const 
        {
            return  _sceneObject.Intersect(ray);
        }



        int getObjectsListSize()
        {
            return _sceneObject.getObjectsListSize();
        }





};






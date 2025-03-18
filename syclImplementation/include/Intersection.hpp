#pragma once    

#include "Vec.hpp"
#include "Ray.hpp"
#include "Material.hpp"

struct Intersection{

    Vec3 _position = Vec3(0,0,0);
    Vec3 _normal = Vec3(0,0,0);  
    myComputeType _distance = 0;
//    Material* _material = nullptr;
    long _objectIndex = -1;
    bool _hit =false;
};

struct resultRecordStructure{
    Vec3 _position = Vec3(0,0,0);
    Vec3 _direction = Vec3(0,0,0);
    bool _hit = false;
    int _collisionCount = 0;
    myComputeType _travelDistance = 0;
};
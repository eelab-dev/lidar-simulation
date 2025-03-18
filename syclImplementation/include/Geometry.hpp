#pragma once

#include "Vec.hpp"
#include "Ray.hpp"
#include "Intersection.hpp"
#include "Bounds3.hpp"



enum class GeometryType
{
    TRIANGLE 
};

struct SamplingRecord
{
    Intersection pos;
    myComputeType pdf = 0.0f;
};

class Geometry
{
public:
    Geometry(GeometryType type) :_type(type) {}
    ~Geometry() = default;
    //bool intersect(const Ray& ray);
    Intersection getIntersection(const Ray& ray) ;
    myComputeType getArea() ;
    SamplingRecord Sample(RNG &rng);
    Bounds3 getBounds();
    GeometryType _type;
    //virtual bool hasEmit() = 0; 
};

#include "Triangle.hpp"

// write the destructor for objector 
// Geometry::~Geometry()
// {
//     switch (_type)
//     {
//     case GeometryType::TRIANGLE:
//         delete static_cast<Triangle*>(this);
//         break;
//     default:
//         break;
//     }
// }


Intersection Geometry::getIntersection(const Ray& ray)
{
    switch (_type)
    {
    case GeometryType::TRIANGLE:
        return static_cast<Triangle*>(this)->getIntersection_virtual(ray);
    default:
        return Intersection();
    }
}

SamplingRecord Geometry::Sample(RNG &rng)
{

    switch (_type)
    {
    case GeometryType::TRIANGLE:
        return static_cast<Triangle*>(this)->Sample_virtual(rng);
    default:
        return SamplingRecord();
    }
}

myComputeType Geometry::getArea()
{
    switch (_type)
    {
    case GeometryType::TRIANGLE:
        return static_cast<Triangle*>(this)->getArea_virtual();
    default:
        return 0;
    }
}

Bounds3 Geometry::getBounds() 
{
    switch (_type)
    {
    case GeometryType::TRIANGLE:
        return static_cast<Triangle*>(this)->getBounds_virtual();
    default:
        return Bounds3();
    }
}







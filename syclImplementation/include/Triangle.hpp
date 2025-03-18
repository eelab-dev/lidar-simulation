#pragma once
#include "Geometry.hpp"
#include "common.hpp"

class Geometry;
class Triangle : public Geometry
{
    public:
         
        Vec3 _v1,_v2,_v3;
        Triangle()
            : Geometry(GeometryType::TRIANGLE){}
        Triangle (const Vec3 &v1,const Vec3 &v2, const Vec3 &v3):Geometry(GeometryType::TRIANGLE),_v1(v1),_v2(v2),_v3(v3)
        {
            e1 = _v2 - _v1;
            e2 = _v3 - _v1;
            normal = crossProduct(e1,e2).normalized();
            area = (crossProduct(e1,e2)).length()*0.5f;
            Geometry::_type = GeometryType::TRIANGLE;
        }
        ~Triangle(){}

    //bool Intersect_virtual(const Ray& ray) const;
    Intersection getIntersection_virtual(const Ray& ray) const;
        
         
    myComputeType getArea_virtual()const{
        return this->area;
    }

    SamplingRecord Sample_virtual(RNG &rng)
    {
        SamplingRecord record;
        record.pdf = 1.0f / area;
        myComputeType x = get_random_float(rng);
        myComputeType y = get_random_float(rng);
        record.pos._position = _v1 * (1.0f - x) + _v2 * (x * (1.0f - y)) + _v3 * (x * y);
        record.pos._normal = this->normal;
        return record;
    }


    Bounds3 getBounds_virtual() const
    {
         return Union(Bounds3(_v1, _v2), _v3); 
    }
    

    private:

        myComputeType area;
        Vec3 normal;
        Vec3 e1,e2;

};


Intersection Triangle::getIntersection_virtual(const Ray& ray) const 
{
    Intersection intersection;

    // Handle floating-point precision errors in dot product check
    if(dotProduct(normal, ray.direction) > -MyEPSILON)  
    {
        intersection._hit = false;
        return intersection;
    }

    myComputeType u, v, t_tmp = 0;
    Vec3 pvec = crossProduct(ray.direction, e2);
    myComputeType det = dotProduct(e1, pvec);

    // Handle near-zero determinant issue with better precision handling
    if (sycl::fabs(det) < MyEPSILON * 10)  
    {
        intersection._hit = false;
        return intersection;
    }

    myComputeType inv_det = 1.0f / det;
    Vec3 tvec = ray.origin - _v1;
    u = dotProduct(tvec, pvec) * inv_det;

    // Adding numerical tolerance to avoid false rejections
    if (u < -MyEPSILON || u > 1 + MyEPSILON)  
    {
        intersection._hit = false;
        return intersection;
    }

    Vec3 qvec = crossProduct(tvec, e1);
    v = dotProduct(ray.direction, qvec) * inv_det;

    // Improved precision handling for u + v condition
    if (v < -MyEPSILON || u + v > 1 + MyEPSILON)  
    {
        intersection._hit = false;
        return intersection;
    }

    t_tmp = dotProduct(e2, qvec) * inv_det;

    // Prevent false rejection due to very small t_tmp
    if (t_tmp < -MyEPSILON)  
    {
        intersection._hit = false;
        return intersection;
    }

    // If all checks passed, register a valid intersection
    intersection._hit = true;
    intersection._position = ray.origin + ray.direction * t_tmp;
    intersection._normal = normal;
    intersection._distance = t_tmp;
    return intersection;
}




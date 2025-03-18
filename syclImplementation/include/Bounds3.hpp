#pragma once


#include "Ray.hpp"
#include "Vec.hpp"
#include <limits>
#include <array>
#include <cmath>
#include <float.h>

class Bounds3
{
    public:
        Vec3 pMin,pMax;
        Bounds3()
        {
            myComputeType minNum = std::numeric_limits<myComputeType>::lowest();
            myComputeType maxNum = std::numeric_limits<myComputeType>::max();
            pMax = Vec3(minNum,minNum,minNum);
            pMin = Vec3(maxNum,maxNum,maxNum);
        }

        Bounds3(const Vec3 p): pMin(p), pMax(p) {}
        Bounds3(const Vec3 p1, const Vec3 p2)
        {
            pMin = Vec3( std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z) );
            pMax = Vec3( std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z) );
        }

        Vec3 Diagonal () const {return pMax - pMin;}
        int maxExtent () const 
        {

            Vec3 d = Diagonal();
            if (d.x > d.y && d.x > d.z){return 0;}
            else if (d.y >d.z){return 1;}
            else {return 2;}
        }

        myComputeType SurfaceArea() const
        {
            Vec3 d = Diagonal();
            return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
        }

        Vec3 Centroid() {return 0.5 * pMin + 0.5 * pMax;}
        Bounds3 Intersect(const Bounds3& b)
        {
            return Bounds3(
                Vec3( std::max(pMin.x, b.pMin.x), std::max(pMin.y, b.pMin.y),std::max(pMin.z, b.pMin.z) ),
                Vec3( std::min(pMax.x, b.pMax.x), std::min(pMax.y, b.pMax.y),std::min(pMax.z, b.pMax.z) )
            );
        }

        bool Overlaps(const Bounds3& b1, const Bounds3& b2)
        {
            bool x = (b1.pMax.x >= b2.pMin.x) && (b1.pMin.x <= b2.pMax.x);
            bool y = (b1.pMax.y >= b2.pMin.y) && (b1.pMin.y <= b2.pMax.y);
            bool z = (b1.pMax.z >= b2.pMin.z) && (b1.pMin.z <= b2.pMax.z);
            return (x && y && z);
        }

        bool Inside(const Vec3& p, const Bounds3& b)
        {
            return (p.x >= b.pMin.x && p.x <= b.pMax.x && p.y >= b.pMin.y &&
                    p.y <= b.pMax.y && p.z >= b.pMin.z && p.z <= b.pMax.z);
        }

        // inline const Vector3f& operator[](int i) const{

        // }

        inline const Vec3& operator[](int i) const
        {
            return (i == 0) ? pMin : pMax;
        }


        inline bool IntersectP(const Ray& ray, const Vec3& invDir, const std::array<int, 3>& dirIsNeg) const;


};

inline bool Bounds3::IntersectP(const Ray& ray, const Vec3& invDir, const std::array<int, 3>& dirIsNeg) const
{
    myComputeType tEnter = std::numeric_limits<myComputeType>::lowest();
    myComputeType tExit = std::numeric_limits<myComputeType>::max();

    for (int i = 0; i < 3; ++i)
    {
        if (ray.direction[i] == 0) // Ray is parallel to slab
        {
            // If ray's origin is outside the slab, there's no intersection
            if (ray.origin[i] < pMin[i] || ray.origin[i] > pMax[i])
                return false;
            continue; // Otherwise, the ray stays within the bounds for this axis
        }

        myComputeType t_min = (pMin[i] - ray.origin[i]) * invDir[i];
        myComputeType t_max = (pMax[i] - ray.origin[i]) * invDir[i];

        if (dirIsNeg[i] == 0)
            std::swap(t_min, t_max); // Swap only when needed

        tEnter = std::max(t_min, tEnter);
        tExit = std::min(t_max, tExit);

        // Early exit: If at any point tEnter > tExit, no intersection
        if (tEnter > tExit)
            return false;
    }

    return tExit >= 0; // Ensure the intersection happens in front of the ray
}


inline Bounds3 Union(const Bounds3& b1, const Bounds3& b2)
{
    return Bounds3(
            Vec3( std::min(b1.pMin.x, b2.pMin.x), std::min(b1.pMin.y, b2.pMin.y),std::min(b1.pMin.z, b2.pMin.z) ),
            Vec3( std::max(b1.pMax.x, b2.pMax.x), std::max(b1.pMax.y, b2.pMax.y),std::max(b1.pMax.z, b2.pMax.z) )
        );
}

inline Bounds3 Union(const Bounds3& b, const Vec3& p)
{
        return Bounds3(
            Vec3( std::min(b.pMin.x, p.x), std::min(b.pMin.y, p.y),std::min(b.pMin.z, p.z) ),
            Vec3( std::max(b.pMax.x, p.x), std::max(b.pMax.y, p.y),std::max(b.pMax.z, p.z) )
        );
}
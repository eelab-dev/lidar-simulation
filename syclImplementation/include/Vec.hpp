#pragma once

#include <cmath>
#include <iostream>
#include <TypeDefine.hpp>


class Vec3 {
public:
    myComputeType x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(myComputeType xx) : x(xx), y(xx), z(xx) {}
    Vec3(myComputeType xx, myComputeType yy, myComputeType zz) : x(xx), y(yy), z(zz) {}
    Vec3 operator * (const myComputeType &r) const { return Vec3(x * r, y * r, z * r); }
    Vec3 operator / (const myComputeType &r) const { return Vec3(x / r, y / r, z / r); }

    myComputeType length() const {return std::sqrt(x * x + y * y + z * z);}
    Vec3 normalized() {
        myComputeType n = std::sqrt(x * x + y * y + z * z);
        return Vec3(x / n, y / n, z / n);
    }

    myComputeType operator[](int index) const {
        // Assuming index is 0, 1, or 2, corresponding to x, y, z
        switch (index) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        //default: throw std::out_of_range("Index out of range");
        }
        return -1;
    }

    // Vec3f normalize(const Vec3f &v) {
    //     float n = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    //     return Vec3f(v.x / n, v.y / n, v.z / n);
    // }

    // float length(const Vec3f &v) {return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);}

    Vec3 operator * (const Vec3 &v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
    Vec3 operator - (const Vec3 &v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator + (const Vec3 &v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator - () const { return Vec3(-x, -y, -z); }
    Vec3& operator += (const Vec3 &v) { x += v.x, y += v.y, z += v.z; return *this; }
    friend Vec3 operator * (const float &r, const Vec3 &v)
    { return Vec3(v.x * r, v.y * r, v.z * r); }
    friend std::ostream & operator << (std::ostream &os, const Vec3 &v)
    { return os << v.x << ", " << v.y << ", " << v.z; }
    //float       operator[](int index) const;
    //float&      operator[](int index);


    static Vec3 Min(const Vec3 &p1, const Vec3 &p2) {
        return Vec3(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
                       std::min(p1.z, p2.z));
    }

    static Vec3 Max(const Vec3 &p1, const Vec3 &p2) {
        return Vec3(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
                       std::max(p1.z, p2.z));
    }
};

inline myComputeType dotProduct(const Vec3 &a, const Vec3 &b)
{ return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vec3 crossProduct(const Vec3 &a, const Vec3 &b)
{
    return Vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
    );
}









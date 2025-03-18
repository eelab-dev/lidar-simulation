#pragma once 
#include <cmath>
#include <limits>
#include <TypeDefine.hpp>

#include <oneapi/dpl/random>
typedef oneapi::dpl::minstd_rand RNG;
#undef M_PI
#define M_PI 3.14159265358979323846f

const myComputeType MyEPSILON = std::numeric_limits<myComputeType>::epsilon();
const myComputeType kInfinity = std::numeric_limits<myComputeType>::max();

inline myComputeType Radians(myComputeType deg) { return (M_PI / 180.f) * deg; }
inline myComputeType Degrees(myComputeType rad) { return (180.f / M_PI) * rad; }

inline myComputeType clamp(myComputeType val, myComputeType low, myComputeType high) {
    if (val < low) return low;
    else if (val > high) return high;
    else return val;
}


myComputeType get_random_float(RNG &rng)
{
    oneapi::dpl::uniform_real_distribution<myComputeType> distribution(0.f, 1.f);
    return distribution(rng);
}

inline Vec3 toWorld(const Vec3 &a, const Vec3 &N){
    Vec3 B, C;
    if (sycl::fabs(N.x) > sycl::fabs(N.y)){
        myComputeType invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
        C = Vec3(N.z * invLen, 0.0f, -N.x *invLen);
    }
    else {
        myComputeType invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
        C = Vec3(0.0f, N.z * invLen, -N.y *invLen);
    }
    B = crossProduct(C, N);
    return a.x * B + a.y * C + a.z * N;
}
#pragma once 
#include <cmath>
#include <cstdint>
#include <limits>
#include <TypeDefine.hpp>
#include <random>

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

inline std::uint32_t mixSeed(std::uint64_t value)
{
    value += 0x9E3779B97F4A7C15ull;
    value = (value ^ (value >> 30)) * 0xBF58476D1CE4E5B9ull;
    value = (value ^ (value >> 27)) * 0x94D049BB133111EBull;
    value ^= value >> 31;
    return static_cast<std::uint32_t>(value) | 1u;
}

inline std::uint32_t makeSampleSeed(std::uint32_t baseSeed, int x, int y, int width, int sampleIndex)
{
    const std::uint64_t pixelIndex =
        static_cast<std::uint64_t>(y) * static_cast<std::uint64_t>(width) +
        static_cast<std::uint64_t>(x);
    const std::uint64_t sampleKey =
        (static_cast<std::uint64_t>(baseSeed) << 32) ^
        (pixelIndex << 1) ^
        static_cast<std::uint64_t>(sampleIndex);
    return mixSeed(sampleKey);
}

inline myComputeType maxAbsComponent(const Vec3& v)
{
    return std::max(std::max(sycl::fabs(v.x), sycl::fabs(v.y)), sycl::fabs(v.z));
}

inline myComputeType computeRayEpsilon(const Vec3& position)
{
    const myComputeType scale = 1.0f + maxAbsComponent(position);
    return std::max((myComputeType)1e-5f, 64.0f * MyEPSILON * scale);
}

inline Vec3 offsetRayOrigin(const Vec3& position, const Vec3& normal, const Vec3& direction)
{
    Vec3 orientedNormal = normal.normalized();
    if (dotProduct(direction, orientedNormal) < 0.0f) {
        orientedNormal = -orientedNormal;
    }
    return position + orientedNormal * computeRayEpsilon(position);
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

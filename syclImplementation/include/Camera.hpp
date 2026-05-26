#pragma once

#include "Vec.hpp"
#include "common.hpp"
#include <utility>
#include <cmath>
#include "Geometry.hpp"
#include <iostream>
class Camera {
public:
    float detectorWidth;
    float detectorHeight;
    Camera(int width, int height, myComputeType fov_x,myComputeType fov_y, const Vec3& position, const Vec3& lookAt, const Vec3& up, float detectorWidth = 20, float detectorHeight = 20)
        : detectorWidth(detectorWidth),detectorHeight(detectorHeight),width(width), height(height), fov_x(fov_x), fov_y(fov_y), position(position), lookAt(lookAt), up(up)  {
        updateBasis();
    }

    void setLookAt(const Vec3& newLookAt) {
        lookAt = newLookAt;
        updateBasis();
    }

    void setBasis(const Vec3& newRight, const Vec3& newUp, const Vec3& newForward) {
        right = newRight;
        up = newUp;
        forward = newForward;
    }

    Vec3 getPosition() const {
        return position;
    }

    Vec3 getDetectorPosition() const{
        return detectorCenter;
    }

    Vec3 getRayDirection(myComputeType x, myComputeType y, RNG &rng) const {
        // myComputeType aspectRatio = static_cast<myComputeType>(width) / static_cast<myComputeType>(height);
        myComputeType halfFovTanX = std::tan(Radians(fov_x) * 0.5f);
        myComputeType halfFovTanY = std::tan(Radians(fov_y) * 0.5f);
        myComputeType randomX = get_random_float(rng);
        myComputeType randomY = get_random_float(rng);

        //float viewX = (2.0f * (x + 0.5f) / width - 1.0f) * aspectRatio * halfFovTan;
        //float viewY = (2.0f * (y + 0.5f) / height - 1.0f) * halfFovTan;
        myComputeType viewX = (2.0f * (x + randomX) / width - 1.0f) *  halfFovTanX;
        myComputeType viewY = (1.0f- 2.0f * (y + randomY) / height ) * halfFovTanY;

        Vec3 rayDir = (viewX * right + viewY * up + forward).normalized();
        return rayDir;
    }

    Vec3 getRayDirection_gasussian(myComputeType x, myComputeType y, RNG &rng) const {
        const myComputeType halfFovTanX = std::tan(Radians(fov_x) * 0.5f);
        const myComputeType halfFovTanY = std::tan(Radians(fov_y) * 0.5f);

        // Start from the beam center for this channel instead of sampling inside a rectangle cell.
        const myComputeType centerViewX = (2.0f * (x + 0.5f) / width - 1.0f) * halfFovTanX;
        const myComputeType centerViewY = (1.0f - 2.0f * (y + 0.5f) / height) * halfFovTanY;

        // Model the emitted beam as a 2D Gaussian in angular space.
        const myComputeType pixelPitchX = (2.0f * halfFovTanX) / width;
        const myComputeType pixelPitchY = (2.0f * halfFovTanY) / height;
        const myComputeType sigmaScale = 0.35f;
        const myComputeType beamOffsetX = sampleGaussian(rng) * pixelPitchX * sigmaScale;
        const myComputeType beamOffsetY = sampleGaussian(rng) * pixelPitchY * sigmaScale;

        const myComputeType viewX = centerViewX + beamOffsetX;
        const myComputeType viewY = centerViewY + beamOffsetY;

        return (viewX * right + viewY * up + forward).normalized();
    }



    std::pair<Triangle, Triangle> generateDetector(const myComputeType detectorWidth, const myComputeType detectorHeight, Vec3 detectorOffset = Vec3(5,0,0)) {
        // Calculate the center of the detector plate in world space

        detectorCenter = position + (right * detectorOffset[0]) +( up * detectorOffset[1]) + (forward * detectorOffset[2]);

        // Get the scaled vectors for the corners of the plate using the user-provided dimensions
        Vec3 halfWidthVec = right * (detectorWidth / 2.0f);
        Vec3 halfHeightVec = up * (detectorHeight / 2.0f);

        // Calculate the four corner vertices of the detector plate
        Vec3 topLeft     = detectorCenter - halfWidthVec + halfHeightVec;
        Vec3 topRight    = detectorCenter + halfWidthVec + halfHeightVec;
        Vec3 bottomLeft  = detectorCenter - halfWidthVec - halfHeightVec;
        Vec3 bottomRight = detectorCenter + halfWidthVec - halfHeightVec;

        // Create the two triangles that form the rectangular plate.
        // The winding order is counter-clockwise when viewed from the camera.
        // Triangle tri1(bottomLeft,topRight, bottomRight);
        // Triangle tri2(bottomLeft, topLeft,topRight );

        Triangle tri1(bottomLeft, bottomRight, topRight);
        Triangle tri2(bottomLeft, topRight, topLeft);


        std::cout << detectorCenter << std::endl;
        std::cout << "forward direction : " << forward << std::endl;
        std::cout << "right direction : " << right << std::endl;
        std::cout << "up direction : " << up << std::endl;
        std::cout << tri1 << std::endl;
        return { tri1, tri2 };
    }


    Vec3 toCameraBase(const Vec3 &incomeDirection) const {
    // Convert world-space direction to camera local basis
    return Vec3(
        dotProduct(incomeDirection, right),
        dotProduct(incomeDirection, up),
        dotProduct(incomeDirection, forward)
    );
}


private:
    int width, height;
    myComputeType fov_x,fov_y;
    Vec3 position, lookAt, up, right, forward;
    Vec3 detectorCenter;
    myComputeType sampleGaussian(RNG &rng) const {
        myComputeType u1 = get_random_float(rng);
        myComputeType u2 = get_random_float(rng);
        u1 = sycl::fmax(u1, static_cast<myComputeType>(1e-6f));

        const myComputeType radius = sycl::sqrt(-2.0f * sycl::log(u1));
        const myComputeType angle = 2.0f * static_cast<myComputeType>(M_PI) * u2;
        return radius * sycl::cos(angle);
    }

    // void updateBasis() {
    //     forward = (lookAt - position).normalized();
    //     right = crossProduct(forward, up).normalized();
    //     up = crossProduct(right, forward).normalized();
    // }
    
    void updateBasis() {
    Vec3 lookDir = lookAt - position;
    if (lookDir.length() < 1e-8) {
        // Avoid NaN
        return;
    }
    forward = lookDir.normalized();

    // Project up onto plane orthogonal to forward
    Vec3 upProjected = up - forward * dotProduct(up, forward);
    if (upProjected.length() < 1e-8) {
        // Avoid NaN if up is parallel to forward
        upProjected = Vec3(0, 1, 0); // or another fallback
    }

    right = crossProduct(upProjected, forward).normalized();
    up = crossProduct(forward, right).normalized();
}
};

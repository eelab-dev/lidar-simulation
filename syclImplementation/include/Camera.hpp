#pragma once

#include "Vec.hpp"
#include "common.hpp"
#include <utility>
#include <cmath>
#include "Geometry.hpp"
#include <iostream>
class Camera {
public:
    Camera(int width, int height, myComputeType fov, const Vec3& position, const Vec3& lookAt, const Vec3& up)
        : width(width), height(height), fov(fov), position(position), lookAt(lookAt), up(up) {
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

    Vec3 getRayDirection(myComputeType x, myComputeType y, RNG &rng) const {
        myComputeType aspectRatio = static_cast<myComputeType>(width) / static_cast<myComputeType>(height);
        myComputeType halfFovTan = std::tan(Radians(fov) * 0.5f);
        myComputeType randomX = get_random_float(rng);
        myComputeType randomY = get_random_float(rng);

        //float viewX = (2.0f * (x + 0.5f) / width - 1.0f) * aspectRatio * halfFovTan;
        //float viewY = (2.0f * (y + 0.5f) / height - 1.0f) * halfFovTan;
        myComputeType viewX = (2.0f * (x + randomX) / width - 1.0f) * aspectRatio * halfFovTan;
        myComputeType viewY = (1.0f- 2.0f * (y + randomY) / height ) * halfFovTan;

        Vec3 rayDir = (viewX * right + viewY * up + forward).normalized();
        return rayDir;
    }

    Camera(const Camera& other){
        width = other.width;
        height = other.height;
        fov = other.fov;
        position = other.position;
        lookAt = other.lookAt;
        up = other.up;
        right = other.right;
        forward = other.forward;
    }

    std::pair<Triangle, Triangle> generateDetector(const myComputeType detectorWidth, const myComputeType detectorHeight, Vec3 detectorOffset = Vec3(30,0,0)) const {
        // Calculate the center of the detector plate in world space

        Vec3 detectorCenter = position + (right * detectorOffset[0]) +( up * detectorOffset[1]) + (forward * detectorOffset[2]);

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
    myComputeType fov;
    Vec3 position, lookAt, up, right, forward;

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


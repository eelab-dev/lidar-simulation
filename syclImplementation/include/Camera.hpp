#pragma once

#include "Vec.hpp"
#include "common.hpp"


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




private:
    int width, height;
    myComputeType fov;
    Vec3 position, lookAt, up, right, forward;

    void updateBasis() {
        forward = (lookAt - position).normalized();
        right = crossProduct(forward, up).normalized();
        up = crossProduct(right, forward).normalized();
    }
};

#pragma once
#include "Material.hpp"


class diffuseMaterial: public Material{
    public:


        diffuseMaterial(): Material() {
            
            _type = DIFFUSE;
        }

        diffuseMaterial(Vec3 emission, Vec3 specular, Vec3 diffuse): Material(emission, specular, diffuse) {
            _type = DIFFUSE;
        }

        Vec3 sample_virtual(const Vec3 &wi, const Vec3 &N, RNG &rng) const{
            // uniform sample on the hemisphere
            myComputeType x_1 = get_random_float(rng), x_2 = get_random_float(rng);
            myComputeType z = sycl::fabs(1.0f - 2.0f * x_1);
            myComputeType r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
            Vec3 localRay(r*std::cos(phi), r*std::sin(phi), z);
            return toWorld(localRay, N);
        }

        Vec3 eval_virtual(const Vec3 &wi, const Vec3 &wo, const Vec3 &N) const{

            float cosTheta = dotProduct(N, wo);
            if (cosTheta <= -MyEPSILON){
                return Vec3(0.0f, 0.0f, 0.0f);
            }
            return _diffuse / M_PI;
        }

        myComputeType pdf_virtual(const Vec3 &wi, const Vec3 &wo, const Vec3 &N) const{
            return 0.5/M_PI;
        }
};

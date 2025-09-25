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

            // Cosine-weighted hemisphere sampling
            myComputeType u1 = get_random_float(rng);  // [0,1)
            myComputeType u2 = get_random_float(rng);  // [0,1)

            myComputeType z   = sycl::sqrt(u1);                          // cosθ
            myComputeType r   = sycl::sqrt((myComputeType)1.0 - z * z);
            myComputeType phi = (myComputeType)(2.0 * M_PI) * u2;

            Vec3 localRay(r * sycl::cos(phi), 
                        r * sycl::sin(phi), 
                        z);

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

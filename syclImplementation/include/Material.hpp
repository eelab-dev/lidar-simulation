#pragma once

#include "Vec.hpp"
#include "common.hpp"



enum MaterialType {DIFFUSE, EMISSION, DETECTOR};

struct MaterialInfo{
    Vec3 _emission;
    Vec3 _specular;
    Vec3 _diffuse;
    bool _isDetector = false;
    MaterialInfo(Vec3 emission, Vec3 specular, Vec3 diffuse): _emission(emission), _specular(specular), _diffuse(diffuse) {}
};


class Material
{

    public:

        Material() {
        }
        MaterialType _type;
        myComputeType pdf(const Vec3 &wi, const Vec3 &wo, const Vec3 &N) const;
        Vec3 sample(const Vec3 &wi, const Vec3 &N, RNG &rng) const;
        Material(Vec3 emission, Vec3 specular, Vec3 diffuse): _emission(emission), _specular(specular), _diffuse(diffuse) {} 
        Vec3 eval(const Vec3 &wi, const Vec3 &wo, const Vec3 &N)const;               

        Vec3 _emission;
        Vec3 _specular;
        Vec3 _diffuse;

        bool getEmission() const
        {
            return  _emission.length() > MyEPSILON;
        }

    
};

#include "DiffuseMaterial.hpp"

myComputeType Material::pdf(const Vec3 &wi, const Vec3 &wo, const Vec3 &N)const{
    switch (_type)
    {
    case DIFFUSE:
        return static_cast<const diffuseMaterial*>(this)->pdf_virtual(wi, wo, N);
    default:
        return 0.0f;
    }
}

Vec3 Material::sample(const Vec3 &wi, const Vec3 &N, RNG &rng)const{
    switch (_type)
    {
    case DIFFUSE:
        return static_cast<const diffuseMaterial*>(this)->sample_virtual(wi, N, rng);
    default:
        return Vec3(0.0f, 0.0f, 0.0f);
    }
}

Vec3 Material::eval(const Vec3 &wi, const Vec3 &wo, const Vec3 &N)const{
    switch (_type)
    {
    case DIFFUSE:
        return static_cast<const diffuseMaterial*>(this)->eval_virtual(wi, wo, N);
    default:
        return Vec3(0.0f, 0.0f, 0.0f);
    }
}




class MaterialFactory{
    public:
    static Material* createMaterial(Vec3 emission, Vec3 specular, Vec3 diffuse){
        return new diffuseMaterial(emission, specular, diffuse);
    } 
};














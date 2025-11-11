#pragma once

#include <tinyusdz.hh>
#include <string>
#include <vector>
#include "Material.hpp"
#include "Vec.hpp"
#include <iostream>
#include <memory>
// #include "ObjectList.hpp"
#include <filesystem>

namespace fs = std::filesystem;

// struct Material_information
// {
//     size_t diffuseMaterialNum = 0;
//     size_t cameraMaterialNum = 0;
// };
struct Material_information
{
    size_t diffuseMaterialNum = 0;
    size_t detectorMaterialNum = 0;
};

struct Triangle_OBJ_result
{
    std::vector<Triangle> Triangles;
    std::vector<MaterialInfo> MaterialsInfoList;
    std::vector<int> materialIDs;
    Material_information materialInfo;
    
};

class OBJ_Loader
{

    std::vector<Triangle> _gloabalTranglesResult;
    std::vector<MaterialInfo> _globalMaterialsInfoList;
    std::vector<int> _globalMaterialIDs;

public:
    std::unordered_map<std::string, unsigned int> materialIndexMap;
    // size_t globalMaterialIndex = 0;
    size_t globalDiffuseMaterialNum = 0;
    size_t globalDetectorNum = 0;

    Triangle_OBJ_result outputTrangleResult()
    {
        Triangle_OBJ_result result;
        result.Triangles = _gloabalTranglesResult;
        result.MaterialsInfoList = _globalMaterialsInfoList;
        result.materialIDs = _globalMaterialIDs;
        result.materialInfo.diffuseMaterialNum = globalDiffuseMaterialNum;
        result.materialInfo.detectorMaterialNum = globalDetectorNum;
        std::cout << result.materialInfo.diffuseMaterialNum << std::endl;
        return result;
    }

    bool addTriangleUSDFile(std::string objFilePath, std::string objFile)
    {
        tinyusdz::Stage stage;
        std::string warn, err;

        std::filesystem::path usdPath = std::filesystem::path(objFilePath + objFile) ;
        tinyusdz::USDLoadOptions opts; // explicit 5th arg
        if (!tinyusdz::LoadUSDFromFile(usdPath.string(), &stage, &warn, &err, opts))
        {
            std::cerr << "USD load failed: " << err << usdPath <<"\n"
                      << warn << std::endl;
            return false;
        }

        tinyusdz::Path p("/materials", "");

        auto r = stage.GetPrimAtPath(p);

        if (!r)
        {
            // r holds an error string
            std::cerr << "\x1b[31m[ERROR]m  GetMaterialRoot failed: \x1b[0" << r.error() << '\n';
            return false;
        }
        else
        {
            // r holds a Prim*; there's no string to print here
            const tinyusdz::Prim *materialPrim = *r;
            std::cout << "Found material prim.\n"; // print whatever you want about the prim
            bool l = loadMaterial(materialPrim);
        }

        tinyusdz::Path g("/geometries", "");

        r = stage.GetPrimAtPath(g);

        if (!r)
        {
            // r holds an error string
            std::cerr << "GetMeshRoot failed: " << r.error() << '\n';
            return false;
        }
        else
        {
            // r holds a Prim*; there's no string to print here
            const tinyusdz::Prim *materialPrim = *r;
            std::cout << "Found mesh prim.\n"; // print whatever you want about the prim
            bool l = loadMesh(materialPrim);
        }

        return true;
    }

    bool loadMesh(const tinyusdz::Prim *root_mesh)
    {

        std::vector<const tinyusdz::Prim *> stack;
        stack.push_back(root_mesh);

        while (!stack.empty())
        {
            auto p = stack.back();
            stack.pop_back();
            for (const auto &c : p->children())
                stack.push_back(&c);

            if (p->type_name() == "Mesh")
            {
                const auto *mesh = p->as<tinyusdz::GeomMesh>();
                if (!mesh)
                    continue;

                auto P = mesh->get_points();                // std::vector<value::point3f>
                auto counts = mesh->get_faceVertexCounts(); // std::vector<int32_t>
                auto fvi = mesh->get_faceVertexIndices();   // std::vector<int32_t>
                std::vector<Vec3> pts;
                pts.reserve(P.size());

                std::string meshMaterialName;
                unsigned int meshMaterialIndex;

                if (!resolve_material_binding_path(mesh, meshMaterialName, meshMaterialIndex))
                {
                    continue;
                };

                // std::cout << "Mesh prim: " << p->element_name() << "  Material: " << meshMaterialName << "  material index: " << materialIndexMap[meshMaterialName] << "\n";

                unsigned int materialIndex = materialIndexMap[meshMaterialName];

                for (const auto &v : P)
                {
                    pts.emplace_back(static_cast<float>(v[0]),
                                     static_cast<float>(v[1]),
                                     static_cast<float>(v[2]));
                }

                size_t idx = 0;
                for (size_t f = 0; f < counts.size(); ++f)
                {
                    int n = counts[f];
                    if (n < 3)
                    {
                        idx += n;
                        continue;
                    }
                    int v0 = fvi[idx++];
                    int vPrev = fvi[idx++];
                    for (int k = 2; k < n; ++k)
                    {
                        int v2 = fvi[idx++];
                        _gloabalTranglesResult.emplace_back(pts[v0], pts[vPrev], pts[v2]);
                        _globalMaterialIDs.push_back(materialIndex); // if you have one
                        vPrev = v2;
                    }
                }
            }
        }

        return true;
    }

    bool loadMaterial(const tinyusdz::Prim* root_material)
    {

        std::vector<const tinyusdz::Prim*> stack;
        stack.push_back(root_material);

        while (!stack.empty())
        {
            auto p = stack.back(); stack.pop_back();
            for (const auto& c : p->children()) stack.push_back(&c);

            if (p->type_name() == "Material")
            {
                const auto* mat =  p->as<tinyusdz::Material>();

                auto it = mat->props.find("inputs:materialType");
                if (it == mat->props.end())
                {
                    continue;  // ← end this iteration
                }

                const tinyusdz::Attribute& attr = it->second.get_attribute();

                // Try token form first (most common for enums like "lambert", "mirror", …)
                tinyusdz::value::token tok;
                if (attr.get_value<tinyusdz::value::token>(&tok))
                {
                    std::cout << "Material " << p->element_name()
                              << "  materialType=" << tok.str() << "\n";                              

                    auto matName = p->element_name();
                    MaterialInfo mat;
                    
                    if (!materialIndexMap.count(matName))
                    {
                        size_t preMaterialList = _globalMaterialsInfoList.size();
                        materialIndexMap[matName] = preMaterialList;
                    }
                    else
                    {
                        std::cerr << "\x1b[31m[ERROR]\x1b[0m "  << "\x1b[31m redfine material \x1b[0m" << std::endl; 
                        std::abort();
                    } 
                    
                    
                    if(tok.str() == "lambert") 
                    {
                        mat = MaterialInfo(MaterialType::DIFFUSE);
                        float mat_diffuse;
                        if(getDiffuseMaterialproperty(p,mat_diffuse))
                        {
                            mat.addReflectivity(mat_diffuse);
                            globalDiffuseMaterialNum++; 
                        }
                        else{

                            std::cerr << "\x1b[31m[ERROR]\x1b[0m "  << "\x1b[31m fail to load diffuse material \x1b[0m" << std::endl; 
                            std::abort();
                        }
                    }

                    _globalMaterialsInfoList.push_back(mat);
                }
            }
        }
        
        return true;
    }
    
    bool resolve_material_binding_path(const tinyusdz::GeomMesh *mesh, std::string &meshMaterialName, unsigned int meshMaterialIndex)
    {

        if (mesh && mesh->has_materialBinding())
        {

            const tinyusdz::Relationship &rel = mesh->materialBinding.value(); // "material:binding"

            if (rel.is_path())
            {
                const tinyusdz::Path &p = rel.targetPath;      // the bound material’s prim path
                const std::string &matName = p.element_name(); // last path element = prim name
                // std::cout << matName << std::endl;

                meshMaterialName = matName;
                if (materialIndexMap.count(matName))
                {
                    meshMaterialIndex = materialIndexMap[matName];
                    return true;
                }
                else
                {
                    std::cerr << "\x1b[31m[ERROR]\x1b[0m " << "\x1b[31m can not find the material \x1b[0m" << std::endl;
                }
            }
        }
        return false;
    }


    bool getDiffuseMaterialproperty(const tinyusdz::Prim* inputDiffusematerial, float& reflectivity)
    {
        const auto* mat =  inputDiffusematerial->as<tinyusdz::Material>();
        auto material_type_prop= mat->props.find("inputs:reflectivity");
        if (material_type_prop == mat->props.end())
        {

            std::cerr << "\x1b[31m[ERROR]\x1b[0m "  << "\x1b[31m can't find diffuse material property \x1b[0m" << std::endl; 
            return false;
        }

        const tinyusdz::Attribute& reflectivity_attr = material_type_prop->second.get_attribute();
        // Try token form first (most common for enums like "lambert", "mirror", …)
        // tinyusdz::value::kFloat tok;
        float tok;

        if (reflectivity_attr.get_value<float>(&tok))
        {
            std::cout << " reflectivity=" << tok << "\n";
            reflectivity = tok;                              
        }

        return true;
    }



    void addCamera(Camera* camera = nullptr)
    {
        if(camera)
        {

            int camearMaterailIndex = _globalMaterialsInfoList.size();
            // Vec3 diffuseVec(47.7688, 38.5664, 31.0928);
            // Vec3 specularVec(47.7688, 38.5664, 31.0928);
            // Vec3 emissionVec(47.7688, 38.5664, 31.0928);
            MaterialInfo camereMat = MaterialInfo(MaterialType::DETECTOR);
            _globalMaterialsInfoList.push_back(camereMat);
            auto camTri = camera->generateDetector(camera->detectorWidth,camera->detectorHeight);
            _gloabalTranglesResult.push_back(camTri.first);
            _gloabalTranglesResult.push_back(camTri.second);
            _globalMaterialIDs.push_back(camearMaterailIndex);
            _globalMaterialIDs.push_back(camearMaterailIndex);
            globalDetectorNum++;
            // std::cout << camearMaterailIndex << std::endl;
        }
    }
};
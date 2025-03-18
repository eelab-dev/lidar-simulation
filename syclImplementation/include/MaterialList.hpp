#pragma once

#include "Material.hpp"
#include <vector>

class materialList
{        
    Material** _materialList = nullptr;
    size_t _materialListSize = 0;
    diffuseMaterial* _diffuseList = nullptr;
    size_t _diffuseListSize = 0;

    public:

        materialList() = default;


        void setDiffuseList(diffuseMaterial* diffuseList, size_t diffuseListSize)
        {
            _diffuseList = diffuseList;
            _diffuseListSize = diffuseListSize;
        }

        void setMaterialList(Material** materialList, size_t materialListSize)
        {
            _materialList = materialList;
            _materialListSize = materialListSize;
        }
        
        materialList(const materialList& other) 
        {
            _materialList = other._materialList;
            _materialListSize = other._materialListSize;
            _diffuseList = other._diffuseList;
            _diffuseListSize = other._diffuseListSize;
        }

        materialList& operator=(const materialList& other)
        {
            _materialList = other._materialList;
            _materialListSize = other._materialListSize;
            _diffuseList = other._diffuseList;
            _diffuseListSize = other._diffuseListSize;
            return *this;
        }


        inline Material* getMaterial(long index) const
        {
            if(index >= _materialListSize || index < 0)
            {
                return nullptr;
            }

            return _materialList[index];
        }

        ~materialList()
        {

        }



};
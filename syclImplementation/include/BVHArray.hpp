#pragma once

#include "Bounds3.hpp"

class ObjectList;
struct BVHNode
{
    long _rightIndex = -1;
    long _leftIndex = -1;
    long _objectIndex = -1;
    Bounds3 _bounds;
};

class BVHArray
{

    long _arraySize = 0;
    BVHNode* _array = nullptr;
    
    

    Intersection getIntersection(const long index, const Ray& ray, const ObjectList* objects) const;
    
    bool haveNode(const long index) const
    {
        if (index >= _arraySize || index < 0){

            //std::cout << "The index is out of range" << std::endl;
            return false;
        } 

        return true;
    }   

    public:

    BVHArray()
    {
        _arraySize = 0;
        _array = nullptr;
    }

    BVHArray(const BVHArray& other)
    {
        _arraySize = other._arraySize;
        _array = other._array;
    }

    BVHArray& operator=(const BVHArray& other)
    {
        _arraySize = other._arraySize;
        _array = other._array;
        return *this;
    }


    void setBVHArray(size_t objectSize, BVHNode* array);
    Intersection Intersect(const Ray& ray, const ObjectList* objects) const;

    const BVHNode* getNode(long index) const
    {
        if (haveNode(index))
        {
            return &_array[index];
        }
        return nullptr;
    }

    long getArraySize() const
    {
        return _arraySize;
    }

    long buildTree(ObjectList* sceneObject, int left, int right);

    ~BVHArray()
    {

    }
};

size_t caculateArraySize(int numObjects)
{
    return 2 * numObjects - 1;
}

bool testIntersection(const BVHNode* node, const Ray& ray)
{
    Vec3 indiv(1.0f / ray.direction.x, 1.0f / ray.direction.y, 1.0f / ray.direction.z);
    std::array<int, 3> dirIsNeg;
    dirIsNeg[0] = ray.direction.x >0; 
    dirIsNeg[1] = ray.direction.y >0; 
    dirIsNeg[2] = ray.direction.z >0; 

    return node->_bounds.IntersectP(ray, indiv, dirIsNeg);
}

void BVHArray::setBVHArray(size_t arraySize, BVHNode* array)
{

    _arraySize = arraySize;
    _array = array;
    std::cout << "The array size is " << _arraySize << std::endl;
    
}







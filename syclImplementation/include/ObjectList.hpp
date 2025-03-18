#pragma once

#include "GeometryList.hpp"
#include "MaterialList.hpp"
#include "BVHArray.hpp"

struct Object
{
    long _geometryIndex = -1;
    long _materialIndex = -1;
};


struct ObjectListContent
{
    sycl::queue& _myQueue;
    ObjectListContent(sycl::queue& myQueue) : _myQueue(myQueue) {}

    Object* _objectList = nullptr;
    size_t _objectListSize = 0;

    Geometry** _geometryList = nullptr;
    size_t _geometryListSize = 0;
    Triangle* _triangleList = nullptr;
    size_t _triangleListSize = 0;
    
    Material** _materialList = nullptr;
    size_t _materialListSize = 0;
    diffuseMaterial* _diffuseMaterialList = nullptr;
    size_t _diffuseMaterialListSize = 0;

    size_t _globalGeometryIndex = 0;
    size_t _gloablMaterialIndex = 0;

    BVHNode* _bvhResource = nullptr;
    size_t _bvhSize = 0;    


    void addTriangleGeometry(std::vector<Triangle> &tris)
    {
        _triangleList = sycl::malloc_shared<Triangle>(tris.size(), _myQueue);
        _geometryList = sycl::malloc_shared<Geometry*>(tris.size(), _myQueue);

            for (size_t i = 0; i < tris.size(); i++)
            {
                //_triangles[i] = tris[i];
                _myQueue.memcpy(_triangleList+i, &tris[i], sizeof(Triangle)).wait();
                _geometryListSize++;
                _geometryList[_globalGeometryIndex] = &_triangleList[i];
                _globalGeometryIndex++;
            }
            _triangleListSize = tris.size();

    }


    void addMaterial(std::vector<MaterialInfo>& materialInfoList)
    {
            _materialList = sycl::malloc_shared<Material*>(materialInfoList.size(), _myQueue);
            _diffuseMaterialList = sycl::malloc_shared<diffuseMaterial>(materialInfoList.size(), _myQueue);
            for (size_t i = 0; i < materialInfoList.size(); i++)
            {
                //_diffuseList[i] = diffuseMaterial(materialInfoList[i]._emission, materialInfoList[i]._specular, materialInfoList[i]._diffuse);
                diffuseMaterial material(materialInfoList[i]._emission, materialInfoList[i]._specular, materialInfoList[i]._diffuse);
                _myQueue.memcpy(_diffuseMaterialList+i, &material, sizeof(diffuseMaterial)).wait();
                _materialList[_gloablMaterialIndex] = &_diffuseMaterialList[i];
                _materialListSize++;
                _diffuseMaterialListSize++;
                _gloablMaterialIndex++;
            }
    }

    void addObject(std::vector<Triangle> &tris, std::vector<MaterialInfo>& materialInfoList, std::vector<int>& geomIDs)
    {
        addTriangleGeometry(tris);
        addMaterial(materialInfoList);
        size_t GeometryListSize = tris.size();
        _objectList = sycl::malloc_shared<Object>(tris.size(), _myQueue);

        for (size_t i = 0; i < GeometryListSize; i++)
        {
            _objectList[_objectListSize]._materialIndex = static_cast<long>(geomIDs[i]);
            _objectList[_objectListSize]._geometryIndex = static_cast<long>(i);
            _objectListSize++;  
        }

        _bvhSize = caculateArraySize(_objectListSize);
        _bvhResource = sycl::malloc_shared<BVHNode>(_bvhSize, _myQueue);
        for (size_t i = 0; i < _bvhSize; i++)
        {
            _bvhResource[i]._objectIndex = -1;
            _bvhResource[i]._leftIndex = -1;
            _bvhResource[i]._rightIndex = -1;
        }

    }

    ~ObjectListContent()
    {
        if (_objectList != nullptr)
        {
            sycl::free(_objectList, _myQueue);
        }

        if (_geometryList != nullptr)
        {
            sycl::free(_geometryList, _myQueue);
        }

        if (_triangleList != nullptr)
        {
            sycl::free(_triangleList, _myQueue);
        }

        if (_materialList != nullptr)
        {
            sycl::free(_materialList, _myQueue);
        }

        if (_diffuseMaterialList != nullptr)
        {
            sycl::free(_diffuseMaterialList, _myQueue);
        }

        if (_bvhResource != nullptr)
        {
            sycl::free(_bvhResource, _myQueue);
        }
    }


    

};


class ObjectList
{
    GeometryList _geometryList;
    materialList _materialList;
    Object* _objectList = nullptr;
    size_t _objectListSize = 0;
    BVHArray _bvh;

    public:
        inline size_t getObjectsListSize() const{return _objectListSize;}

        ObjectList()
        {
        } 
            
        void setObjects(ObjectListContent& content)
        {
            _objectList = content._objectList;
            _objectListSize = content._objectListSize;
            _geometryList.setTriangles(content._triangleList, content._triangleListSize);
            _geometryList.setGeometryList(content._geometryList, content._geometryListSize);
            _materialList.setDiffuseList(content._diffuseMaterialList, content._diffuseMaterialListSize);            
            _materialList.setMaterialList(content._materialList, content._materialListSize);
            _bvh.setBVHArray(content._bvhSize, content._bvhResource);
            _bvh.buildTree(this, 0, _objectListSize - 1);
        }

        ObjectList(const ObjectList& other)
        {
            _objectList = other._objectList;
            _objectListSize = other._objectListSize;
            _geometryList = other._geometryList;
            _materialList = other._materialList;
            _bvh = other._bvh;
        }

        ObjectList& operator=(const ObjectList& other)
        {
            _objectList = other._objectList;
            _objectListSize = other._objectListSize;
            _geometryList = other._geometryList;
            _materialList = other._materialList;
            _bvh = other._bvh;
            return *this;
        }

        ~ObjectList()
        {

        }

        Bounds3 getBounds(size_t index)
        {
            Object _object = _objectList[index];
            Geometry* _geometry = _geometryList.getGeometry(_object._geometryIndex);
            return _geometry->getBounds();
        }

        //bool intersect(const Ray& ray){return _geometry->intersect(ray);}
        Intersection getIntersection(const Ray& ray, long index) const
        {
            if (index < 0)
            {
                return Intersection();
            }
            Object _object = _objectList[index];
            auto _geometry = _geometryList.getGeometry(_object._geometryIndex);
            auto intersection = _geometry->getIntersection(ray);
            //intersection._material = _material;
            intersection._objectIndex = index;
            return intersection;
        }

        myComputeType getArea(long index) const
        {
            if (index < 0)
            {
                return 0;
            }
            Object _object = _objectList[index];
            auto _geometry = _geometryList.getGeometry(_object._geometryIndex);
            return _geometry->getArea();
        }



        SamplingRecord Sample(RNG &rng, size_t index) const
        {
            Object _object = _objectList[index];
            Geometry* _geometry = _geometryList.getGeometry(_object._geometryIndex);
            SamplingRecord record = _geometry->Sample(rng);
            //auto _material = _materialList[_materialIndexArray[index]];
            // auto _material = _materialList.getMaterial(_object._materialIndex);
            //record.pos._material = _material;
            record.pos._objectIndex = index;
            return record;
        }


        const Material* getMaterial(long index) const
        {
            if(index < 0)
            {
                return nullptr;
            }
            Object _object = _objectList[index];
            return _materialList.getMaterial(_object._materialIndex);
            //return _materialList.getMaterial(_materialIndexArray[index]);
        }


        Intersection Intersect(const Ray &ray) const 
        {
            return _bvh.Intersect(ray, this);
        }


        void spacePartitioning(long left, long right)
        {

            //std::cout << left << "  " << right << std::endl;
            Bounds3 centroidBounds;
            for (int i = left; i <= right; i++)
            {
                auto _geometry = _geometryList.getGeometry(_objectList[i]._geometryIndex);
                centroidBounds = Union(centroidBounds, _geometry->getBounds().Centroid());
            }
            int dim = centroidBounds.maxExtent();
            // std::cout << dim << std::endl;
            // for (int i = left; i <= right; i++)
            // {
            //     auto _geometry = _geometryList.getGeometry(_objectList[i]._geometryIndex);
            //     std::cout << "ID number " << _objectList[i]._geometryIndex<< "  " <<_geometry->getBounds().Centroid()[dim] << std::endl;
            // }

            // std::cout << std::endl;
            if (dim == 0)
            {
                std::sort(_objectList + left, _objectList + right + 1 , [this](const Object& obja, const Object& objb) {
                    return spaceCompare(obja, objb, 0);
                });
            }
            else if (dim == 1)
            {
                std::sort(_objectList + left, _objectList + right + 1, [this](const Object& obja, const Object& objb) {
                    return spaceCompare(obja, objb, 1);
                });
            }
            else if (dim == 2)
            {
                std::sort(_objectList + left, _objectList + right + 1, [this](const Object& obja, const Object& objb) {
                    return spaceCompare(obja, objb, 2);
                });
            }
            // std::cout << "After sorting" << std::endl;
            // for (int i = left; i <= right; i++)
            // {
            //     auto _geometry = _geometryList.getGeometry(_objectList[i]._geometryIndex);
            //     std::cout << "ID number " << _objectList[i]._geometryIndex<< "  " <<_geometry->getBounds().Centroid()[dim] << std::endl;
            // }
        
        }

        bool spaceCompare(const Object& obja, const Object& objb, int dim)
        {

            auto a = _geometryList.getGeometry(obja._geometryIndex);
            auto b = _geometryList.getGeometry(objb._geometryIndex);


            myComputeType aCentroid = a->getBounds().Centroid()[dim];
            myComputeType bCentroid = b->getBounds().Centroid()[dim];

            return aCentroid < bCentroid;
        }

};



long BVHArray::buildTree(ObjectList* sceneObject, int left, int right)
{

    static long index = 0;

    if(!haveNode(index))
    {
        std::cout << index << std::endl;
        std::cout << left << "  " << right << std::endl;
        return -1;
    }
    
    long curIndex = index;
    index = index + 1;
    
    
    if (left == right)
    {
        _array[curIndex]._objectIndex = left;
        _array[curIndex]._bounds = sceneObject->getBounds(left);
        return curIndex;
    }
    else if (left + 1 == right)
    {
        long leftIndex = buildTree(sceneObject, left, left);
        long rightIndex = buildTree(sceneObject, right, right);
        _array[curIndex]._leftIndex = leftIndex; 
        _array[curIndex]._rightIndex = rightIndex;
        
        _array[curIndex]._bounds = Union(_array[leftIndex]._bounds, _array[rightIndex]._bounds);
        return curIndex;
    }
    sceneObject->spacePartitioning(left, right);
    long mid = (left + right) / 2;
    

    long leftIndex = buildTree(sceneObject,  left, mid);
    long rightIndex = buildTree(sceneObject, mid + 1, right); 
    _array[curIndex]._leftIndex = leftIndex;
    _array[curIndex]._rightIndex = rightIndex;

    //std::cout << "The left index is " << leftIndex << "  The right index is " << rightIndex << std::endl;
    if(haveNode(leftIndex) && haveNode(rightIndex))
    {
         _array[curIndex]._bounds = Union(_array[leftIndex]._bounds, _array[rightIndex]._bounds);
    }
   
    return curIndex;
}


Intersection BVHArray::Intersect(const Ray& ray, const ObjectList* objects) const
{
    Intersection inter;
    if (_array == nullptr) return inter;
    inter = getIntersection(0, ray, objects);
    return inter;
}

// Intersection BVHArray::getIntersection(const long index, const Ray& ray, const ObjectList* objects) const
// {
//     if (!haveNode(index)) return Intersection();

//     Intersection inter;
//     inter._hit = false;
//     inter._distance = INFINITY;


//     BVHNode* stack[64] = {nullptr};
//     BVHNode* curNode = &(_array[index]);
//     int stackCount = 0;

//     while((stackCount >= 0 && stackCount < 64) || curNode!=nullptr)
//     {
//         while(curNode != nullptr)
//         {
//             if(!testIntersection(curNode, ray))
//             {
//                 curNode = nullptr;
//                 break;
//             }
//             if (curNode->_objectIndex > 0)
//             {
//                 auto curObjectIndex = curNode->_objectIndex;
//                 Intersection tmp = objects->getIntersection(ray, curObjectIndex);
//                 if (tmp._hit && inter._distance > tmp._distance)
//                 {
//                     inter = tmp;
//                 }
//                 curNode = nullptr;
//                 break;
//             }
//             long curRight = curNode->_rightIndex;
//             if(haveNode(curRight))
//             {
//                 stackCount++;
//                 stack[stackCount] = &_array[curRight];
//             }
//             long curLeft = curNode->_leftIndex;
//             if (haveNode(curLeft))
//             {
//                 curNode = &_array[curLeft];
//             }
//             else
//             {
//                 curNode = nullptr;
//             }
//         }

//         if(stackCount >= 0 && stackCount < 64)
//         {
//             curNode = stack[stackCount];
//             stackCount--;
//         }
//     }

//     return inter;
 
// }


Intersection BVHArray::getIntersection(const long index, const Ray &ray,
                                       const ObjectList *objects) const {
  if (!haveNode(index))
    return Intersection();

  Intersection inter;
  inter._hit = false;
  inter._distance = INFINITY;

  BVHNode *stack[50] = {&(_array[index])};
  int stackCount = 1;

  while (stackCount != 0) {
    stackCount--;
    BVHNode *curNode = stack[stackCount];
    long curLeft = curNode->_leftIndex;
    long curRight = curNode->_rightIndex;

    if (!testIntersection(curNode, ray)) {
      continue;
    }

    // leaf node
    if (curNode->_objectIndex >= 0) {
      auto curObjectIndex = curNode->_objectIndex;
      Intersection tmp = objects->getIntersection(ray, curObjectIndex);
      if (tmp._hit && inter._distance > tmp._distance) {
        inter = tmp;
      }
      continue;
    }

    // add left
    if (haveNode(curLeft)) {
      stack[stackCount] = &_array[curLeft];
      stackCount++;
    }

    if (haveNode(curRight)) {
      stack[stackCount] = &_array[curRight];
      stackCount++;
    }
  }

  return inter;
}

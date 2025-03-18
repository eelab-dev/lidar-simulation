#pragma once

#include "Geometry.hpp"
#include <vector>
#include "algorithm"


class GeometryList{

    Geometry** _geometryList = nullptr;
    size_t _geometryListSize = 0;
    Triangle* _triangles = nullptr;
    size_t _trianglesSize = 0;


    public:

        //GeometryList() : _geometryList(nullptr), _geometryListSize(0), _triangles(nullptr), _trianglesSize(0), _globalIndex(0) {}

        GeometryList() = default;

        void setTriangles(Triangle* triangles, size_t trianglesSize)
        {
            _triangles = triangles;
            _trianglesSize = trianglesSize;
        }

        void setGeometryList(Geometry** geometryList, size_t geometryListSize)
        {
            _geometryList = geometryList;
            _geometryListSize = geometryListSize;
        }


        GeometryList(const GeometryList& other)
        {
            _geometryList = other._geometryList;
            _geometryListSize = other._geometryListSize;
            _triangles = other._triangles;
            _trianglesSize = other._trianglesSize;
        }



        GeometryList& operator=(const GeometryList& other)
        {
            _geometryList = other._geometryList;
            _geometryListSize = other._geometryListSize;
            _triangles = other._triangles;
            _trianglesSize = other._trianglesSize;
            return *this;
        }

        ~GeometryList()
        {
  
        }


        inline size_t getGeometryListSize()const{return _geometryListSize;}
        inline size_t getTrianglesSize()const{return _trianglesSize;} 


        inline Geometry* getGeometry(size_t index) const
        {
            return _geometryList[index];
        }







        

};




#pragma once

#include "Vec.hpp"


struct Ray{

    Vec3 origin;
    Vec3 direction;
    Ray(Vec3 o, Vec3 d):origin(o), direction(d){}



};
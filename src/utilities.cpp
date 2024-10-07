//
// Created by maja on 7.10.24..
//

#include "utilities.h"

#include <cmath>

double Utilities::constrainAngle(float x){
    x = fmod(x,360);
    if (x < 0)
        x += 360;
    return x;
}

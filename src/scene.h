//
// Created by maja on 7.10.24..
//

#ifndef SCENE_H
#define SCENE_H

#include <glm/vec3.hpp>

#include "programState.h"
#include "utilities.h"

class Character;

class Scene {
public:
    Scene();
    ~Scene() = default;

    // Light
    glm::vec3 lightPos;
    glm::vec3 roomLightPosition;
    glm::vec3 roomLightColor;
    bool spotlightOn;

    void setLights(Shader &shader, ProgramState *programState);
    void coinSetLights(Shader &shader, ProgramState *programState);


    // Is character in the hidden room
    void roomCheck(Character&, ProgramState*);
    bool inside;

    // Did character catch the star
    void starCheck(Character&);
    void yellowStarCheck();
    void redStarCheck(Character&);
    void blueStarCheck(Character&);
    bool starCatched;
    bool yellowStarCatched;
    bool redStarCatched;
    bool blueStarCatched;

    // Mushroom
    void mushroomCheck();
    bool mushroomVisible;
    float mushroomHeight;

    // Transparent box
    void boxCheck(Character&);
    bool boxRising;
    bool boxFalling;
    glm::vec3 transparentBoxPosition;
    Utilities::enumColor boxColor;
};



#endif //SCENE_H

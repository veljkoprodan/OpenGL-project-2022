//
// Created by maja on 7.10.24..
//

#ifndef CHARACTER_H
#define CHARACTER_H

#include <glm/vec3.hpp>

#include "utilities.h"

class Scene;

class Character {
public:
    Character();
    ~Character() = default;

    // Mario color
    void marioColorCheck();
    Utilities::enumColor marioColor;

    // Mario jump
    void jumpCheck(Scene &scene);
    bool jump;
    bool inAir;
    float jumpSpeed;
    float currentJumpHeight;
    float jumpLimit;

    // Mario fall
    void fallCheck();
    bool falling;
    bool rise;

    // Choose the character
    enum enumCharacter {mario, ghost};
    enumCharacter currentCharacter;

    // Character starting position
    glm::vec3 characterPosition{};

    // Character movement
    bool isOnPoint(float x, float z, float delta);
    float characterAngle;
    float characterSpeed;
};



#endif //CHARACTER_H

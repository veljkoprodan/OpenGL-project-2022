//
// Created by maja on 7.10.24..
//

#include "character.h"

#include "scene.h"

Character::Character()
{
    marioColor = Utilities::red;

    // Mario jump
    jump = false;
    inAir = false;
    jumpSpeed = 0.1;
    currentJumpHeight = 0;
    jumpLimit = 1.7f;

    // Mario fall
    falling = false;
    rise = false;

    // Choose the character
    currentCharacter = mario;

    // Character starting position
    characterPosition = glm::vec3(-5.0f, -3.0f, 0.2f);

    // Character movement
    characterAngle = 180.0f;
    characterSpeed = 0.07f;
}

void Character::marioColorCheck(){
    if(isOnPoint(-19.0f, 2.0f, 0.6f))
        marioColor = Utilities::red;
    else if(isOnPoint(-20.0f, 4.0f, 0.6f))
        marioColor = Utilities::blue;
    else if(isOnPoint(-19.0f, 6.0f, 0.6f))
        marioColor = Utilities::green;
    else if(isOnPoint(-17.0f, 6.0f, 0.6f))
        marioColor = Utilities::lightblue;
    else if(isOnPoint(-16.0f, 4.0f, 0.6f))
        marioColor = Utilities::yellow;
    else if(isOnPoint(-17.0f, 2.0f, 0.6f))
        marioColor = Utilities::pink;
}

void Character::jumpCheck(Scene &scene){
    if(jump){
        inAir = true;
        characterSpeed = 0.2f;
        currentJumpHeight += jumpSpeed;
        characterPosition.y += jumpSpeed;
    }

    if(currentJumpHeight >= jumpLimit){
        jump = false;
        characterSpeed = 0.07f;
        if(isOnPoint(-5.0f, 0.0f, 0.5f))
            scene.mushroomVisible = true;
    }

    if(jump == false && currentJumpHeight > 0){
        currentJumpHeight -= jumpSpeed;
        characterPosition.y -= jumpSpeed;
    }
    else{
        inAir = false;
    }
}

void Character::fallCheck()
{
    if(!inAir && !jump && characterPosition.x >= -11.567f && characterPosition.x <= -9.4321f
            && characterPosition.z >= -2.83704f && characterPosition.x <= 0.0236122f)
        falling = true;

    if(falling){
        if(characterPosition.y <= -20.0f){
            falling = false;
            rise = true;
            characterPosition = glm::vec3(-5.0f, -5.0f, 0.2f);
        }
        else
            characterPosition.y -= 0.2f;
    }
    if(rise){
        if(characterPosition.y >= -3.0f)
            rise = false;
        else
            characterPosition.y += 0.1f;
    }
}

bool Character::isOnPoint(float x, float z, float delta){
    return(characterPosition.x > x-delta && characterPosition.x < x+delta
           && characterPosition.z > z-delta && characterPosition.z < z + delta);
}
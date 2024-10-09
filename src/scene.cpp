//
// Created by maja on 7.10.24..
//

#include "scene.h"

#include <GLFW/glfw3.h>
#include <ctime>
#include <cstdlib>

#include "character.h"

Scene::Scene()
{
    // LightPos
    lightPos = glm::vec3(-5.0f, 5.3f, 0.0f);
    roomLightPosition = glm::vec3(20.3f, -4.3f, 0.0f);
    roomLightColor = glm::vec3(50.0f, 50.0f, 50.0f);
    spotlightOn = false;

    // Is character in the hidden room
    inside = false;

    // Did character catch the star
    starCatched = false;
    yellowStarCatched = false;
    redStarCatched = false;
    blueStarCatched = false;

    // Mushroom
    mushroomVisible = false;
    mushroomHeight = 0;

    // Transparent box
    boxRising = false;
    boxFalling = false;
    transparentBoxPosition = glm::vec3(-5.9f, -2.8f, -5.4f);

    std::srand(std::time(nullptr));
    boxColor = (Utilities::enumColor)(std::rand() % 5 + 1);
}

void Scene::setLights(Shader &shader, ProgramState *programState){
    shader.setVec3("light.position", lightPos);
    shader.setVec3("viewPos", programState->camera.Position);

    // directional light
    shader.setVec3("dirLight.direction", 1.0f, -1.0, 0.0f);
    shader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    // pointlight properties
    shader.setVec3("pointLights[0].position", lightPos);
    shader.setVec3("pointLights[0].ambient", 0.1f, 0.1f, 0.1f);
    shader.setVec3("pointLights[0].diffuse", 0.6f, 0.6f, 0.6f);
    shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);

    shader.setFloat("pointLights[0].constant", 1.0f);
    shader.setFloat("pointLights[0].linear", 0.09f);
    shader.setFloat("pointLights[0].quadratic", 0.032f);
    // spotLight
    shader.setVec3("spotLight.position", programState->camera.Position);
    shader.setVec3("spotLight.direction", programState->camera.Front);
    shader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    shader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    shader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("spotLight.constant", 1.0f);
    shader.setFloat("spotLight.linear", 0.09f);
    shader.setFloat("spotLight.quadratic", 0.032f);
    shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

    if(!spotlightOn){
        shader.setVec3("spotLight.diffuse", 0.0f, 0.0f, 0.0f);
        shader.setVec3("spotLight.specular", 0.0f, 0.0f, 0.0f);
    }
}

void Scene::coinSetLights(Shader &shader, ProgramState *programState){
    shader.setVec3("pointLight.position", lightPos);
    shader.setVec3("pointLight.ambient", 0.1f, 0.1f, 0.1f);
    shader.setVec3("pointLight.diffuse", 0.6f, 0.6f, 0.6f);
    shader.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("pointLight.constant", 1.0f);
    shader.setFloat("pointLight.linear", 0.09f);
    shader.setFloat("pointLight.quadratic", 0.032f);

    shader.setVec3("lightPos", lightPos);
    shader.setVec3("viewPos", programState->camera.Position);

    shader.setVec3("dirlight.direction", 1.0f, -1.0, 0.0f);
    shader.setVec3("dirlight.ambient", 0.05f, 0.05f, 0.05f);
    shader.setVec3("dirlight.diffuse", 0.4f, 0.4f, 0.4f);
    shader.setVec3("dirlight.specular", 0.5f, 0.5f, 0.5f);

    shader.setVec3("spotlight.ambient", 0.5f, 0.5f, 0.5f);
    shader.setVec3("spotlight.diffuse", 1.0f, 1.0f, 1.0f);
    shader.setVec3("spotlight.specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("spotlight.constant", 1.0f);
    shader.setFloat("spotlight.linear", 0.09f);
    shader.setFloat("spotlight.quadratic", 0.032f);
    shader.setVec3("spotlight.position", programState->camera.Position);
    shader.setVec3("spotlight.direction", programState->camera.Front);
    shader.setFloat("spotlight.cutOff", glm::cos(glm::radians(12.5f)));
    shader.setFloat("spotlight.outerCutOff", glm::cos(glm::radians(15.0f)));

}

void Scene::roomCheck(Character &character, ProgramState *programState){
    if(character.isOnPoint(-1.6f,-8.2f, 0.6f && character.currentCharacter == Character::mario)){
        character.characterPosition = glm::vec3 (14.3f, -4.5f, -4.77f);
        programState->camera.Position = glm::vec3(29.67f, -0.11f, -5.66f);
        programState->camera.Front = glm::vec3(-0.91f, -0.18f, 0.35f);
        inside = true;
    }

    if(character.isOnPoint(13.8f, -5.1f, 0.6f && character.currentCharacter == Character::mario) && starCatched){
        character.characterPosition = glm::vec3 (-3.57f, -3.0f, -7.71f);
        programState->camera.Position = glm::vec3(-12.17f, 0.5f, -17.0f);
        programState->camera.Front = glm::vec3(0.6f, -0.1f, 0.78f);
        starCatched = false;
        inside = false;
        roomLightPosition.y = -4.3f;
    }
}

void Scene::starCheck(Character &character){
    if(character.isOnPoint(20.0f, 0.0f, 0.5f)){
        starCatched = true;
        roomLightPosition.y = 100.0f;
    }
}

void Scene::yellowStarCheck(){
    float angle = Utilities::constrainAngle((float)glfwGetTime() * 40);

    if(angle >= 88.5f && angle <= 91.5f)
        yellowStarCatched = true;
}

void Scene::redStarCheck(Character &character){
    if(character.isOnPoint(-18.07f, -0.85f, 0.6f) && character.characterPosition.y >= 11.8f
        && character.characterPosition.y <= 13.3f)
    {
        redStarCatched = true;
    }
}

void Scene::blueStarCheck(Character &character){
    if(character.isOnPoint(2.68f, 3.12f, 0.6f) && character.characterPosition.y >= -1.7f &&
        character.characterPosition.y <= -1.2f)
    {
        blueStarCatched = true;
    }
}

void Scene::mushroomCheck(){
    if(mushroomVisible){
        if(mushroomHeight < 0.8f)
            mushroomHeight += 0.03;
    }
    else{
        if(mushroomHeight > 0)
            mushroomHeight -= 0.03;
    }
}

void Scene::boxCheck(Character &character){
    if(character.isOnPoint(-6.36f, -5.32f, 0.5f) && !boxRising && !boxFalling && !yellowStarCatched){
        boxRising = true;
    }

    if(boxRising){
        if(character.characterPosition.y >= 3.8f){
            boxRising = false;
            yellowStarCheck();
            if(yellowStarCatched){
                boxFalling = true;
            }
        }
        else{
            transparentBoxPosition.y += 0.1f;
            character.characterPosition.y += 0.1f;
        }
    }

    if(boxFalling){
        transparentBoxPosition.y -= 0.1f;
        character.characterPosition.y -= 0.1f;
        if(character.characterPosition.y <= -3.0f)
            boxFalling = false;
    }
}
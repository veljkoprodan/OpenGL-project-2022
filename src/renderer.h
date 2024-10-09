//
// Created by maja on 7.10.24..
//

#ifndef RENDERER_H
#define RENDERER_H

#include <learnopengl/shader.h>
#include <learnopengl/model.h>
#include <GLFW/glfw3.h>

class Renderer {

private:


public:
    Renderer();
    ~Renderer() = default;

    unsigned int quadVAO;
    unsigned int quadVBO;

    unsigned int cubeVAO;
    unsigned int cubeVBO;

    unsigned int static loadTexture(char const * path, bool gammaCorrection);

    void renderQuad();
    void renderCube();
    void renderMario(Shader &shader, Model &marioModel, glm::vec3 position, float angle);
    void renderGhost(Shader &shader, Model &ghostModel, glm::vec3 position, float angle);
    void renderRoomScene(Shader &shader);
    void renderRoomPipe(Shader &shader, Model &pipeModel);
    void renderPipe(Shader& shader, Model &pipeModel);
    void renderStar(Shader& shader, Model &starModel);
    void renderYellowStar(Shader &shader, Model &yellowStarModel);
    void renderBlueStar(Shader &shader, Model &blueStarModel);
    void renderRedStar(Shader &shader, Model &redStarModel);
    void renderIsland(Shader& shader, Model &islandModel);
    void renderShip(Shader& shader, Model &shipModel);
    void renderMushroom(Shader& shader, Model &mushroomModel, float height);
};



#endif //RENDERER_H

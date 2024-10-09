//
// Created by maja on 7.10.24..
//

#include "renderer.h"
#include "utilities.h"

Renderer::Renderer()
{
    quadVAO = 0;
    cubeVAO = 0;
    cubeVBO = 0;
}

unsigned int Renderer::loadTexture(char const * path, bool gammaCorrection)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void Renderer::renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Renderer::renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
                // back face
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                // right face
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
                // bottom face
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                // top face
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}


void Renderer::renderMario(Shader &shader, Model &marioModel, glm::vec3 position, float angle){
    glm::mat4 modelMario = glm::mat4(1.0f);
    modelMario = glm::translate(modelMario, position);
    modelMario = glm::rotate(modelMario, glm::radians(angle - 180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMario = glm::scale(modelMario, glm::vec3(0.4f));
    shader.setMat4("model", modelMario);
    marioModel.Draw(shader);
}

void Renderer::renderRoomPipe(Shader &shader, Model &pipeModel){
    glm::mat4 modelPipe = glm::mat4(1.0f);
    modelPipe = glm::translate(modelPipe, glm::vec3(9.0f, -5.0f, 0.0f));
    modelPipe = glm::scale(modelPipe, glm::vec3(0.5f, 0.5f, 0.5f));
    shader.setMat4("model", modelPipe);
    glDisable(GL_CULL_FACE);
    pipeModel.Draw(shader);
    glEnable(GL_CULL_FACE);
}

void Renderer::renderStar(Shader &shader, Model &starModel){
    glm::mat4 modelStar = glm::mat4(1.0f);
    modelStar = glm::translate(modelStar, glm::vec3(20.0f, -6.5f, 3.0f));
    //modelStar = glm::rotate(modelStar,(float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
    modelStar = glm::scale(modelStar, glm::vec3(5.0f, 5.0f, 5.0f));
    shader.setMat4("model", modelStar);
    starModel.Draw(shader);
}

void Renderer::renderMushroom(Shader& shader, Model &mushroomModel, float height){
    glm::mat4 modelMushroom = glm::mat4(1.0f);
    modelMushroom = glm::translate(modelMushroom, glm::vec3(-5.0f, -0.3f + height, 0.0f));
    modelMushroom = glm::rotate(modelMushroom, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelMushroom = glm::scale(modelMushroom, glm::vec3(0.3f));
    shader.setMat4("model", modelMushroom);
    mushroomModel.Draw(shader);
}

void Renderer::renderShip(Shader &shader, Model &shipModel){
    glm::mat4 modelShip = glm::mat4(1.0f);
    modelShip = glm::translate(modelShip, glm::vec3(-18.0f, 0.0f, 0.0f));
    modelShip = glm::scale(modelShip, glm::vec3(10.0f));
    shader.setMat4("model", modelShip);
    shipModel.Draw(shader);
}

void Renderer::renderPipe(Shader &shader, Model &pipeModel){
    glm::mat4 modelPipe = glm::mat4(1.0f);
    modelPipe = glm::translate(modelPipe, glm::vec3(-5.9f, -3.6f, -3.0f));
    modelPipe = glm::scale(modelPipe, glm::vec3(0.5f));
    shader.setMat4("model", modelPipe);
    pipeModel.Draw(shader);
}

void Renderer::renderIsland(Shader &shader, Model &islandModel){
    glm::mat4 modelIsland = glm::mat4(1.0f);
    modelIsland = glm::translate(modelIsland, glm::vec3 (0.0f, 0.0f, 0.0f));
    modelIsland = glm::scale(modelIsland, glm::vec3(10.0f));
    shader.setMat4("model", modelIsland);
    islandModel.Draw(shader);
}

void Renderer::renderGhost(Shader &shader, Model &ghostModel, glm::vec3 position, float angle){
    glm::mat4 modelGhost = glm::mat4(1.0f);
    modelGhost = glm::translate(modelGhost, position);
    modelGhost = glm::rotate(modelGhost, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelGhost = glm::scale(modelGhost, glm::vec3(0.003f));
    shader.setMat4("model", modelGhost);
    ghostModel.Draw(shader);
}

void Renderer::renderYellowStar(Shader &shader, Model &yellowStarModel){
    float angle = Utilities::constrainAngle((float)glfwGetTime() * 40);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-4.0f, 2.3f, -5.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(4.0f));
    shader.setMat4("model", model);
    yellowStarModel.Draw(shader);
}

void Renderer::renderBlueStar(Shader &shader, Model &blueStarModel){
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(5.0f, -3.0f, 3.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(4.0f));
    shader.setMat4("model", model);
    blueStarModel.Draw(shader);
}

void Renderer::renderRedStar(Shader &shader, Model &redStarModel){
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-14.5f, 10.0f, -0.8f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(6.0f));
    shader.setMat4("model", model);
    redStarModel.Draw(shader);
}

void Renderer::renderRoomScene(Shader &shader){
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(20.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(10.0f, 5.0f, 7.0f));
    shader.setMat4("model", model);
    glDisable(GL_CULL_FACE);
    shader.setInt("inverse_normals", 1);
    renderCube();
    shader.setInt("inverse_normals", 0);
    glEnable(GL_CULL_FACE);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(18.0f + 6*sin(glfwGetTime()), 0.0f, 1.0));
    model = glm::scale(model, glm::vec3(0.75f));
    shader.setMat4("model", model);
    renderCube();

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(18.0f - 6*sin(glfwGetTime()), 0.0f, 4.0));
    model = glm::scale(model, glm::vec3(0.75f));
    shader.setMat4("model", model);
    renderCube();
}
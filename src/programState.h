//
// Created by maja on 7.10.24..
//

#ifndef PROGRAMSTATE_H
#define PROGRAMSTATE_H
#include <glm/vec3.hpp>
#include <learnopengl/camera.h>
#include <learnopengl/shader.h>

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    PointLight pointLight;
    DirLight dirLight;
    SpotLight spotLight;
    ProgramState()
            : camera(glm::vec3(-15.0f, 0.0f, -3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

inline void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

inline void ProgramState::LoadFromFile(std::string filename) {}

#endif //PROGRAMSTATE_H

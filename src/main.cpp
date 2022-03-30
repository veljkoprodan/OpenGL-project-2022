#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void setLights(Shader shaderName);

void jumpCheck();

void mushroomCheck();

unsigned int loadCubemap(vector<std::string> faces);

unsigned int loadTexture(char const * path);

// Mario jump
bool jump = false;
float jumpSpeed = 0.1;
float currentJumpHeight = 0;
float jumpLimit = 1.7f;

// mushroom
bool mushroomVisible = false;
float mushroomHeight = 0;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//lightPos
glm::vec3 lightPos(-4.0f, 2.4f, 2.0f);

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    //glm::vec3 backpackPosition = glm::vec3(0.0f);
    //float backpackScale = 1.0f;
    PointLight pointLight;
    DirLight dirLight;
    SpotLight spotLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
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

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // build and compile shaders
    // -------------------------
    Shader ourShader("resources/shaders/model_shader.vs", "resources/shaders/model_shader.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader brickBoxShader("resources/shaders/box.vs", "resources/shaders/box.fs");
    Shader marioBoxShader("resources/shaders/box.vs", "resources/shaders/box.fs");
    Shader redDiamondShader("resources/shaders/shader.vs", "resources/shaders/shader.fs");
    Shader blueDiamondShader("resources/shaders/shader.vs", "resources/shaders/shader.fs");
    Shader greenDiamondShader("resources/shaders/shader.vs", "resources/shaders/shader.fs");
    Shader lightBlueDiamondShader("resources/shaders/shader.vs", "resources/shaders/shader.fs");
    Shader yellowDiamondShader("resources/shaders/shader.vs", "resources/shaders/shader.fs");
    Shader pinkDiamondShader("resources/shaders/shader.vs", "resources/shaders/shader.fs");

    float boxVertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    float skyboxVertices[] = {
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // box VAO
    unsigned int boxVBO, boxVAO;
    glGenVertexArrays(1, &boxVAO);
    glGenBuffers(1, &boxVBO);

    glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boxVertices), boxVertices, GL_STATIC_DRAW);

    glBindVertexArray(boxVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // load cubemap textures
    vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/skybox_tmp/right.jpg"),
                    FileSystem::getPath("resources/textures/skybox_tmp/left.jpg"),
                    FileSystem::getPath("resources/textures/skybox_tmp/top.jpg"),
                    FileSystem::getPath("resources/textures/skybox_tmp/bottom.jpg"),
                    FileSystem::getPath("resources/textures/skybox_tmp/front.jpg"),
                    FileSystem::getPath("resources/textures/skybox_tmp/back.jpg")
            };

    unsigned int cubemapTexture = loadCubemap(faces);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // load Mario cube textures
    stbi_set_flip_vertically_on_load(true);
    unsigned int questionambientMap  = loadTexture(FileSystem::getPath("resources/textures/mario_ambient.jpg").c_str());
    unsigned int questiondiffuseMap  = loadTexture(FileSystem::getPath("resources/textures/mario_cube.jpg").c_str());
    unsigned int questionspecularMap = loadTexture(FileSystem::getPath("resources/textures/mario_specular.jpg").c_str());

    //load brick cube textures
    unsigned int brickambientMap  = loadTexture(FileSystem::getPath("resources/textures/brick_ambient.jpg").c_str());
    unsigned int brickdiffuseMap  = loadTexture(FileSystem::getPath("resources/textures/brick_diffuse.jpg").c_str());
    unsigned int brickspecularMap = loadTexture(FileSystem::getPath("resources/textures/brick_specular.jpg").c_str());

    // Mario cube shader configuration
    marioBoxShader.use();
    marioBoxShader.setInt("material.ambient", 0);
    marioBoxShader.setInt("material.diffuse", 1);
    marioBoxShader.setInt("material.specular", 2);

    // Brick box shader configuration
    brickBoxShader.use();
    brickBoxShader.setInt("material.ambient", 3);
    brickBoxShader.setInt("material.diffuse", 4);
    brickBoxShader.setInt("material.specular", 5);
    stbi_set_flip_vertically_on_load(false);

    // load diamond textures
    unsigned int redDiamondTexture = loadTexture(
            FileSystem::getPath("resources/textures/diamonds/red-transparent.png").c_str());
    unsigned int blueDiamondTexture = loadTexture(
            FileSystem::getPath("resources/textures/diamonds/blue-transparent.png").c_str());
    unsigned int greenDiamondTexture = loadTexture(
            FileSystem::getPath("resources/textures/diamonds/green-transparent.png").c_str());
    unsigned int lightBlueDiamondTexture = loadTexture(
            FileSystem::getPath("resources/textures/diamonds/light-blue-transparent.png").c_str());
    unsigned int yellowDiamondTexture = loadTexture(
            FileSystem::getPath("resources/textures/diamonds/yellow-transparent.png").c_str());
    unsigned int pinkDiamondTexture = loadTexture(
            FileSystem::getPath("resources/textures/diamonds/pink-transparent.png").c_str());

    redDiamondShader.setInt("texture1", 0);
    blueDiamondShader.setInt("texture1", 0);
    greenDiamondShader.setInt("texture1", 0);
    lightBlueDiamondShader.setInt("texture1", 0);
    yellowDiamondShader.setInt("texture1", 0);
    pinkDiamondShader.setInt("texture1", 0);

    // load models
    // -----------

    Model islandModel("resources/objects/island/EO0AAAMXQ0YGMC13XX7X56I3L.obj");
    islandModel.SetShaderTextureNamePrefix("material.");

    Model coinModel("resources/objects/coin/Coin.obj");
    coinModel.SetShaderTextureNamePrefix("material.");

    Model mushroomModel("resources/objects/mushroom/693sxrp8upr3.obj");
    mushroomModel.SetShaderTextureNamePrefix("material.");

    Model marioModel("resources/objects/mario/1DNSCLY0D1YQZHJRH142C5GI0.obj");
    marioModel.SetShaderTextureNamePrefix("material.");

    Model shipModel("resources/objects/ship/FBRIPHH48VJVZ9GUIX3KK06PB.obj");
    shipModel.SetShaderTextureNamePrefix("material.");

    Model diamondModel("resources/objects/diamond/diamond.obj");
    diamondModel.SetShaderTextureNamePrefix("material.");

    // ugh
//    Model redDiamondModel("resources/objects/diamonds/red/diamond.obj");
//    redDiamondModel.SetShaderTextureNamePrefix("material.");

//    Model greenDiamondModel("resources/objects/diamonds/green/diamond.obj");
//    greenDiamondModel.SetShaderTextureNamePrefix("material.");

//    Model blueDiamondModel("resources/objects/diamonds/blue/diamond.obj");
//    blueDiamondModel.SetShaderTextureNamePrefix("material.");

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {

        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        //todo sortiranje providnih


        jumpCheck();
        mushroomCheck();


        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();

        glDisable(GL_CULL_FACE);

        //brick box
        //bind brick ambient map
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, brickambientMap);
        // bind specular map
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, brickdiffuseMap);
        // bind emission map
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, brickspecularMap);

        //light setting
        brickBoxShader.use();
        setLights(brickBoxShader);
        brickBoxShader.setFloat("material.shininess", 64.0f);

        //brick box projection/view
        brickBoxShader.setMat4("projection", projection);
        brickBoxShader.setMat4("view", view);

        glBindVertexArray(boxVAO);

        for(int i = 0; i < 3; i++){
            glm::mat4 modelBrickBox = glm::mat4(1.0f);
            float boxTranslation = (float)i * 1.0f;

            // make space for Mario box
            if(i == 2)
                boxTranslation += 1.0f;

            modelBrickBox = glm::translate(modelBrickBox, glm::vec3(-5.0f, -0.4f, 2.0f - boxTranslation));
            modelBrickBox = glm::rotate(modelBrickBox, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            modelBrickBox = glm::scale(modelBrickBox, glm::vec3(1.0f));

            brickBoxShader.setMat4("model", modelBrickBox);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        //mario box
        //bind brick ambient map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, questionambientMap);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, questiondiffuseMap);
        // bind emission map
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, questionspecularMap);

        marioBoxShader.use();
        setLights(marioBoxShader);
        marioBoxShader.setFloat("material.shininess", 64.0f);

        //mario box projection/view
        marioBoxShader.setMat4("projection", projection);
        marioBoxShader.setMat4("view", view);

        glBindVertexArray(boxVAO);
        glm::mat4 modelMarioBox = glm::mat4(1.0f);
        modelMarioBox = glm::translate(modelMarioBox, glm::vec3(-5.0f, -0.4f, 0.0f));
        modelMarioBox = glm::rotate(modelMarioBox, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMarioBox = glm::scale(modelMarioBox, glm::vec3(1.0f));
        brickBoxShader.setMat4("model", modelMarioBox);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // diamonds
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, redDiamondTexture);
        redDiamondShader.use();
        redDiamondShader.setMat4("projection", projection);
        redDiamondShader.setMat4("view", view);
        glm::mat4 modelRedDiamond = glm::mat4(1.0f);
        modelRedDiamond = glm::translate(modelRedDiamond, glm::vec3(-10.0f, 2.0f, 0.0f));
        //modelRedDiamond = glm::rotate(modelRedDiamond, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        modelRedDiamond = glm::scale(modelRedDiamond, glm::vec3(0.1f));
        redDiamondShader.setMat4("model", modelRedDiamond);
        diamondModel.Draw(redDiamondShader);


        glEnable(GL_CULL_FACE);

        // don't forget to enable shader before setting uniforms
        ourShader.use();
        setLights(ourShader);
        ourShader.setFloat("material.shininess", 32.0f);
        // view/projection transformations
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);





        // render the loaded models

        glm::mat4 modelIsland = glm::mat4(1.0f);
        modelIsland = glm::translate(modelIsland, glm::vec3 (0.0f, 0.0f, 0.0f));
        modelIsland = glm::scale(modelIsland, glm::vec3(10.0f));
        ourShader.setMat4("model", modelIsland);
        islandModel.Draw(ourShader);

        glm::mat4 modelCoin = glm::mat4(1.0f);
        modelCoin = glm::translate(modelCoin, glm::vec3 (-18.0f, 8.0f, 0.0f));
        modelCoin = glm::scale(modelCoin, glm::vec3(0.02f));
        modelCoin = glm::rotate(modelCoin, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        ourShader.setMat4("model", modelCoin);
        coinModel.Draw(ourShader);

        glm::mat4 modelMushroom = glm::mat4(1.0f);
        modelMushroom = glm::translate(modelMushroom, glm::vec3(-5.0f, -0.3f + mushroomHeight, 0.0f));
        modelMushroom = glm::rotate(modelMushroom, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMushroom = glm::scale(modelMushroom, glm::vec3(0.3f));
        ourShader.setMat4("model", modelMushroom);
        mushroomModel.Draw(ourShader);

        glm::mat4 modelMario = glm::mat4(1.0f);
        modelMario = glm::translate(modelMario, glm::vec3(-5.0f, -3.0f + currentJumpHeight, 0.2f));
        modelMario = glm::scale(modelMario, glm::vec3(0.4f));
        ourShader.setMat4("model", modelMario);
        marioModel.Draw(ourShader);

        glm::mat4 modelShip = glm::mat4(1.0f);
        modelShip = glm::translate(modelShip, glm::vec3(-18.0f, 0.0f, 0.0f));
        modelShip = glm::scale(modelShip, glm::vec3(10.0f));
        ourShader.setMat4("model", modelShip);
        shipModel.Draw(ourShader);

//        glDisable(GL_CULL_FACE);

//        glm::mat4 modelRedDiamond = glm::mat4(1.0f);
//        modelRedDiamond = glm::translate(modelRedDiamond, glm::vec3(-18.0f, 0.0f, 13.0f));
//        modelRedDiamond = glm::scale(modelRedDiamond, glm::vec3(0.1f));
//        ourShader.setMat4("model", modelRedDiamond);
//        redDiamondModel.Draw(ourShader);

//        glm::mat4 modelGreenDiamond = glm::mat4(1.0f);
//        modelGreenDiamond = glm::translate(modelGreenDiamond, glm::vec3(-18.0f, 0.0f, 9.0f));
//        modelGreenDiamond = glm::scale(modelGreenDiamond, glm::vec3(0.1f));
//        ourShader.setMat4("model", modelGreenDiamond);
//        greenDiamondModel.Draw(ourShader);

//        glm::mat4 modelBlueDiamond = glm::mat4(1.0f);
//        modelBlueDiamond = glm::translate(modelBlueDiamond, glm::vec3(-18.0f, 0.0f, 5.0f));
//        modelBlueDiamond = glm::scale(modelBlueDiamond, glm::vec3(0.1f));
//        ourShader.setMat4("model", modelBlueDiamond);
//        blueDiamondModel.Draw(ourShader);



//        glEnable(GL_CULL_FACE);


        if (programState->ImGuiEnabled)
            DrawImGui(programState);

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        jump = true;

    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        mushroomVisible = false;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);
        //ImGui::DragFloat3("Backpack position", (float*)&programState->backpackPosition);
        //ImGui::DragFloat("Backpack scale", &programState->backpackScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);
        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void setLights(Shader shaderName){
    shaderName.setVec3("light.position", lightPos);
    shaderName.setVec3("viewPos", programState->camera.Position);

    // directional light
    shaderName.setVec3("dirLight.direction", 0.0f, -1.0, 0.0f);
    shaderName.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    shaderName.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    shaderName.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    //pointlight properties
    shaderName.setVec3("pointLights[0].position", lightPos);
    shaderName.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    shaderName.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    shaderName.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    shaderName.setFloat("pointLights[0].constant", 1.0f);
    shaderName.setFloat("pointLights[0].linear", 0.09f);
    shaderName.setFloat("pointLights[0].quadratic", 0.032f);
    // spotLight
    shaderName.setVec3("spotLight.position", programState->camera.Position);
    shaderName.setVec3("spotLight.direction", programState->camera.Front);
    shaderName.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    shaderName.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    shaderName.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    shaderName.setFloat("spotLight.constant", 1.0f);
    shaderName.setFloat("spotLight.linear", 0.09f);
    shaderName.setFloat("spotLight.quadratic", 0.032f);
    shaderName.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    shaderName.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
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

void jumpCheck(){
    if(jump)
        currentJumpHeight += jumpSpeed;

    if(currentJumpHeight >= jumpLimit){
        jump = false;
        mushroomVisible = true;
    }

    if(jump == false && currentJumpHeight > 0)
        currentJumpHeight -= jumpSpeed;
}

void mushroomCheck(){
    if(mushroomVisible){
        if(mushroomHeight < 0.8f)
            mushroomHeight += 0.03;
    }
    else{
        if(mushroomHeight > 0)
            mushroomHeight -= 0.03;
    }
}



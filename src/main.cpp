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

#include <cmath>
#include <ctime>
#include <iostream>

#include "programState.h"
#include "renderer.h"
#include "utilities.h"
#include "character.h"
#include "scene.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
void stateCheck();
unsigned int loadCubemap(vector<std::string> faces);
void DrawImGui(ProgramState *programState);

ProgramState *programState;
Character *character = new Character();
Scene *scene = new Scene();
Renderer renderer;

// Shadows
bool shadows = true;

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool bloom = true;
bool hdr = true;
bool sharpenEffect = false;
float exposure = 0.7f;

// Camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

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

    // Glfw window creation
    //----------------------------------------------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Mario OpenGL", NULL, NULL);
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

    // Glad: load all OpenGL function pointers
    //----------------------------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

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

    // Configure global opengl state
    //----------------------------------------------------------
    glEnable(GL_DEPTH_TEST);

    // Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Build and compile shaders
    //----------------------------------------------------------
    Shader ourShader("resources/shaders/model/model_shader.vs", "resources/shaders/model/model_shader.fs");
    Shader skyboxShader("resources/shaders/skybox/skybox.vs", "resources/shaders/skybox/skybox.fs");
    Shader brickBoxShader("resources/shaders/basic/shader.vs", "resources/shaders/basic/shader.fs");
    Shader marioBoxShader("resources/shaders/basic/shader.vs", "resources/shaders/basic/shader.fs");
    Shader diamondShader("resources/shaders/diamond/diamondShader.vs", "resources/shaders/diamond/diamondShader.fs");
    Shader coinShader("resources/shaders/coin/coinInstancingShader.vs", "resources/shaders/coin/coinInstancingShader.fs");
    Shader starShader("resources/shaders/star/star.vs", "resources/shaders/star/star.fs");
    Shader roomShader("resources/shaders/room/room.vs", "resources/shaders/room/room.fs");
    Shader blurShader("resources/shaders/blur/blur.vs", "resources/shaders/blur/blur.fs");
    Shader bloomShader("resources/shaders/bloom/bloom.vs", "resources/shaders/bloom/bloom.fs");
    Shader effectShader("resources/shaders/sharpen/effect.vs", "resources/shaders/sharpen/effect.fs");
    Shader depthShader("resources/shaders/depth/depthShader.vs",
                       "resources/shaders/depth/depthShader.fs",
                       "resources/shaders/depth/depthShader.gs");

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

    // Skybox VAO
    //----------------------------------------------------------
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Box VAO
    //----------------------------------------------------------
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

    // Load cubemap textures
    //----------------------------------------------------------
    vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/skybox/right.jpg"),
                    FileSystem::getPath("resources/textures/skybox/left.jpg"),
                    FileSystem::getPath("resources/textures/skybox/top.jpg"),
                    FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
                    FileSystem::getPath("resources/textures/skybox/front.jpg"),
                    FileSystem::getPath("resources/textures/skybox/back.jpg")
            };

    unsigned int cubemapTexture = loadCubemap(faces);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // Load Mario cube textures
    //----------------------------------------------------------
    stbi_set_flip_vertically_on_load(true);
    unsigned int questionambientMap  = Renderer::loadTexture(FileSystem::getPath("resources/textures/mario_ambient.jpg").c_str(),true);
    unsigned int questiondiffuseMap  = Renderer::loadTexture(FileSystem::getPath("resources/textures/mario_cube.jpg").c_str(),true);
    unsigned int questionspecularMap = Renderer::loadTexture(FileSystem::getPath("resources/textures/mario_specular.jpg").c_str(),true);

    // Load brick cube textures
    //----------------------------------------------------------
    unsigned int brickambientMap  = Renderer::loadTexture(FileSystem::getPath("resources/textures/brick_ambient.jpg").c_str(),true);
    unsigned int brickdiffuseMap  = Renderer::loadTexture(FileSystem::getPath("resources/textures/brick_diffuse.jpg").c_str(),true);
    unsigned int brickspecularMap = Renderer::loadTexture(FileSystem::getPath("resources/textures/brick_specular.jpg").c_str(),true);

    // Load hidden room texture
    //----------------------------------------------------------
    unsigned int stoneTexture = Renderer::loadTexture(FileSystem::getPath("resources/textures/stone_texture.jpeg").c_str(), true); // note that we're loading the texture as an SRGB texture


    // Configuring floating point framebuffer
    //----------------------------------------------------------
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }


    // Create and attach depth buffer (renderbuffer)
    //----------------------------------------------------------
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // Ping-pong-framebuffer for blurring
    //----------------------------------------------------------
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    // Setting uniform in shaders for bloom
    //----------------------------------------------------------
    roomShader.use();
    roomShader.setInt("diffuseTexture", 0);
//    roomShader.setInt("depthMap", 1);

    blurShader.use();
    blurShader.setInt("image", 0);
    bloomShader.use();
    bloomShader.setInt("scene", 0);
    bloomShader.setInt("bloomBlur", 1);

    unsigned int effectFBO;
    unsigned int effectColorBuffer;
    glGenFramebuffers(1, &effectFBO);
    glGenTextures(1, &effectColorBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, effectFBO);
    glBindTexture(GL_TEXTURE_2D, effectColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, effectColorBuffer, 0);
    effectShader.use();
    effectShader.setInt("effectTexture", 0);

    // Mario cube shader configuration
    //----------------------------------------------------------
    marioBoxShader.use();
    marioBoxShader.setInt("material.ambient", 0);
    marioBoxShader.setInt("material.diffuse", 1);
    marioBoxShader.setInt("material.specular", 2);

    // Brick box shader configuration
    //----------------------------------------------------------
    brickBoxShader.use();
    brickBoxShader.setInt("material.ambient", 0);
    brickBoxShader.setInt("material.diffuse", 1);
    brickBoxShader.setInt("material.specular", 2);
    stbi_set_flip_vertically_on_load(false);

    // Diamond positions and textures
    //----------------------------------------------------------
    std::vector< std::pair<glm::vec3, unsigned int> > diamonds;

    unsigned int redDiamondTexture = Renderer::loadTexture(
            FileSystem::getPath("resources/textures/diamonds/red-transparent.png").c_str(),true);
    diamonds.push_back({glm::vec3(-19.0f, -4.0f, 2.0f), redDiamondTexture});

    unsigned int blueDiamondTexture = Renderer::loadTexture(
            FileSystem::getPath("resources/textures/diamonds/blue-transparent.png").c_str(),true);
    diamonds.push_back({glm::vec3(-20.0f, -4.0f, 4.0f), blueDiamondTexture});

    unsigned int greenDiamondTexture = Renderer::loadTexture(
            FileSystem::getPath("resources/textures/diamonds/green-transparent.png").c_str(),true);
    diamonds.push_back({glm::vec3(-19.0f, -4.0f, 6.0f), greenDiamondTexture});

    unsigned int lightBlueDiamondTexture = Renderer::loadTexture(
            FileSystem::getPath("resources/textures/diamonds/light-blue-transparent.png").c_str(),true);
    diamonds.push_back({glm::vec3(-17.0f, -4.0f, 6.0f), lightBlueDiamondTexture});

    unsigned int yellowDiamondTexture = Renderer::loadTexture(
            FileSystem::getPath("resources/textures/diamonds/yellow-transparent.png").c_str(),true);
    diamonds.push_back({glm::vec3(-16.0f, -4.0f, 4.0f), yellowDiamondTexture});

    unsigned int pinkDiamondTexture = Renderer::loadTexture(
            FileSystem::getPath("resources/textures/diamonds/pink-transparent.png").c_str(),true);
    diamonds.push_back({glm::vec3(-17.0f, -4.0f, 2.0f), pinkDiamondTexture});

    diamondShader.use();
    diamondShader.setInt("texture1", 0);


    // Mario textures
    //----------------------------------------------------------
    unsigned int marioTextureDefault = Renderer::loadTexture(
            FileSystem::getPath("resources/textures/mario/default.jpg").c_str(),true);
    unsigned int marioTextureGreen = Renderer::loadTexture(
            FileSystem::getPath("resources/textures/mario/green.jpg").c_str(),true);
    unsigned int marioTextureBlue = Renderer::loadTexture(
            FileSystem::getPath("resources/textures/mario/blue.jpg").c_str(),true);
    unsigned int marioTextureLightblue = Renderer::loadTexture(
            FileSystem::getPath("resources/textures/mario/lightblue.jpg").c_str(),true);
    unsigned int marioTextureYellow = Renderer::loadTexture(
            FileSystem::getPath("resources/textures/mario/yellow.jpg").c_str(),true);
    unsigned int marioTexturePink = Renderer::loadTexture(
            FileSystem::getPath("resources/textures/mario/pink.jpg").c_str(),true);

    ourShader.use();
    ourShader.setInt("texture1", 0);


    // Load models
    //----------------------------------------------------------
    Model islandModel("resources/objects/island/EO0AAAMXQ0YGMC13XX7X56I3L.obj");
    islandModel.SetShaderTextureNamePrefix("material.");

    Model mushroomModel("resources/objects/mushroom/693sxrp8upr3.obj");
    mushroomModel.SetShaderTextureNamePrefix("material.");

    Model marioModel("resources/objects/mario/1DNSCLY0D1YQZHJRH142C5GI0.obj");
    marioModel.SetShaderTextureNamePrefix("material.");

    Model shipModel("resources/objects/ship/FBRIPHH48VJVZ9GUIX3KK06PB.obj");
    shipModel.SetShaderTextureNamePrefix("material.");

    Model diamondModel("resources/objects/diamond/diamond.obj");
    diamondModel.SetShaderTextureNamePrefix("material.");

    Model coinModel("resources/objects/coin/Coin.obj");
    coinModel.SetShaderTextureNamePrefix("material.");

    Model pipeModel("resources/objects/pipe/pipe.obj");
    pipeModel.SetShaderTextureNamePrefix("material.");

    Model starModel("resources/objects/star/star.obj");
    coinModel.SetShaderTextureNamePrefix("material.");

    Model ghostModel("resources/objects/ghost/dzgtepw5cv4k.obj");
    ghostModel.SetShaderTextureNamePrefix("material.");

    Model yellowStarModel("resources/objects/marioStar/star.obj");
    yellowStarModel.SetShaderTextureNamePrefix("material.");

    Model redStarModel("resources/objects/redStar/star.obj");
    redStarModel.SetShaderTextureNamePrefix("material.");

    Model blueStarModel("resources/objects/blueStar/star.obj");
    blueStarModel.SetShaderTextureNamePrefix("material.");

    // Instancing
    //----------------------------------------------------------
    unsigned int coinAmount = 10;
    glm::mat4* modelMatrices;
    modelMatrices = new glm::mat4[coinAmount];

    for (unsigned int i = 0; i < coinAmount; i++){
        glm::mat4 modelCoin = glm::mat4(1.0f);

        if(i < 4){
            modelCoin = glm::translate(modelCoin, glm::vec3(-1.1f, 5.3f, -0.1f + i*1.9f));
            modelCoin = glm::rotate(modelCoin, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else if(i < 7)
            modelCoin = glm::translate(modelCoin, glm::vec3(0.35f + 1.95f * (i-4), 5.3f, 7.7f));
        else
            modelCoin = glm::translate(modelCoin, glm::vec3(0.35f + 1.95f * (i-7), 5.3f, -2.2f));

        modelCoin = glm::scale(modelCoin, glm::vec3(0.01f));

        modelMatrices[i] = modelCoin;
    }

    unsigned int coinBuffer;
    glGenBuffers(1, &coinBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, coinBuffer);
    glBufferData(GL_ARRAY_BUFFER, coinAmount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    for (unsigned int i = 0; i < coinModel.meshes.size(); i++){
        unsigned int coinVAO = coinModel.meshes[i].VAO;
        glBindVertexArray(coinVAO);

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) 0);
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (sizeof(glm::vec4)));
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *) (3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
        glVertexAttribDivisor(7, 1);
        glVertexAttribDivisor(8, 1);

        glBindVertexArray(0);
    }


    programState->camera.Position = glm::vec3(-14.63f, 0.28f, -7.27f);
    programState->camera.Front = glm::vec3(0.88f, -0.03f, 0.47f);


    // Render loop
    //----------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {

        // Per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        // --------------------
        processInput(window);

        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // View/projection transformations
        //----------------------------------------------------------
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();

        // Render a chosen character
        //----------------------------------------------------------
        ourShader.use();
        scene->setLights(ourShader, programState);
        ourShader.setFloat("material.shininess", 32.0f);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        if(character->currentCharacter == Character::mario){
            glActiveTexture(GL_TEXTURE0);

            if(character->marioColor == Utilities::red)
                glBindTexture(GL_TEXTURE_2D, marioTextureDefault);
            else if(character->marioColor == Utilities::green)
                glBindTexture(GL_TEXTURE_2D, marioTextureGreen);
            else if(character->marioColor == Utilities::blue)
                glBindTexture(GL_TEXTURE_2D, marioTextureBlue);
            else if(character->marioColor == Utilities::lightblue)
                glBindTexture(GL_TEXTURE_2D, marioTextureLightblue);
            else if(character->marioColor == Utilities::yellow)
                glBindTexture(GL_TEXTURE_2D, marioTextureYellow);
            else if(character->marioColor == Utilities::pink)
                glBindTexture(GL_TEXTURE_2D, marioTexturePink);

            renderer.renderMario(ourShader, marioModel, character->characterPosition, character->characterAngle);
            stateCheck();
        }
        else if(character->currentCharacter == Character::ghost){
            renderer.renderGhost(ourShader, ghostModel, character->characterPosition, character->characterAngle);
            scene->redStarCheck(*character);
            scene->blueStarCheck(*character);
        }


// Render models outside of the hidden room
//-----------------------------------------------------------------
if(!scene->inside){

        // Coin rendering (instancing)
        //----------------------------------------------------------
        coinShader.use();
        scene->coinSetLights(coinShader, programState);
        coinShader.setMat4("projection", projection);
        coinShader.setMat4("view", view);
        coinShader.setFloat("material.shininess", 32.0f);
        coinShader.setInt("texture_diffuse1", 0);
        coinShader.setInt("texture_specular1", 1);
        coinShader.setInt("texture_normal1", 2);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, coinModel.textures_loaded[0].id);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, coinModel.textures_loaded[1].id);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, coinModel.textures_loaded[2].id);

        for (unsigned int i = 0; i < coinModel.meshes.size(); i++)
        {
            glBindVertexArray(coinModel.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(
                    coinModel.meshes[i].indices.size()), GL_UNSIGNED_INT, 0, coinAmount);
            glBindVertexArray(0);
        }


        // Render other models
        //----------------------------------------------------------
        ourShader.use();
        scene->setLights(ourShader, programState);
        ourShader.setFloat("material.shininess", 32.0f);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        renderer.renderMushroom(ourShader, mushroomModel, scene->mushroomHeight);
        renderer.renderShip(ourShader, shipModel);

        glDisable(GL_CULL_FACE); // Face culling doesn't work for some models

        renderer.renderPipe(ourShader, pipeModel);
        renderer.renderIsland(ourShader, islandModel);
        if(!scene->yellowStarCatched)
            renderer.renderYellowStar(ourShader, yellowStarModel);

        if(!scene->blueStarCatched)
            renderer.renderBlueStar(ourShader, blueStarModel);

        if(!scene->redStarCatched)
            renderer.renderRedStar(ourShader, redStarModel);


        // Box rendering
        //----------------------------------------------------------

        // Brick box
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, brickambientMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, brickdiffuseMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brickspecularMap);

        brickBoxShader.use();
        scene->setLights(brickBoxShader, programState);
        brickBoxShader.setFloat("material.shininess", 32.0f);
        brickBoxShader.setMat4("projection", projection);
        brickBoxShader.setMat4("view", view);

        glBindVertexArray(boxVAO);
        for(int i = 0; i < 3; i++){
            glm::mat4 modelBrickBox = glm::mat4(1.0f);
            float boxTranslation = (float)i * 1.0f;

            // Make space for Mario box
            if(i == 2)
                boxTranslation += 1.0f;

            modelBrickBox = glm::translate(modelBrickBox, glm::vec3(-5.0f, -0.4f, 2.0f - boxTranslation));
            modelBrickBox = glm::rotate(modelBrickBox, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            modelBrickBox = glm::scale(modelBrickBox, glm::vec3(1.0f));

            brickBoxShader.setMat4("model", modelBrickBox);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Mario box
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, questionambientMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, questiondiffuseMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, questionspecularMap);

        marioBoxShader.use();
        scene->setLights(marioBoxShader, programState);
        marioBoxShader.setFloat("material.shininess", 32.0f);
        marioBoxShader.setMat4("projection", projection);
        marioBoxShader.setMat4("view", view);

        glBindVertexArray(boxVAO);
        glm::mat4 modelMarioBox = glm::mat4(1.0f);
        modelMarioBox = glm::translate(modelMarioBox, glm::vec3(-5.0f, -0.4f, 0.0f));
        modelMarioBox = glm::rotate(modelMarioBox, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMarioBox = glm::scale(modelMarioBox, glm::vec3(1.0f));
        marioBoxShader.setMat4("model", modelMarioBox);
        glDrawArrays(GL_TRIANGLES, 0, 36);



        // Transparent box texture (diamond textures are used)
        //----------------------------------------------------------
        unsigned int transparentBoxTexture;

        if(scene->boxColor == Utilities::green)
            transparentBoxTexture = greenDiamondTexture;
        else if(scene->boxColor == Utilities::blue)
            transparentBoxTexture = blueDiamondTexture;
        else if(scene->boxColor == Utilities::lightblue)
            transparentBoxTexture = lightBlueDiamondTexture;
        else if(scene->boxColor == Utilities::pink)
            transparentBoxTexture = pinkDiamondTexture;
        else if(scene->boxColor == Utilities::yellow)
            transparentBoxTexture = yellowDiamondTexture;
        else
            transparentBoxTexture = redDiamondTexture; // won't happen

        std::pair<glm::vec3, unsigned int> transparentBox = {scene->transparentBoxPosition, transparentBoxTexture};

        // Sort and render diamonds and the box (blending)
        //----------------------------------------------------------

        //      distance         translation   texture
        std::map<float, std::pair<glm::vec3, unsigned int>> diamondsSorted;

        for(std::pair<glm::vec3, unsigned int> diamond : diamonds){
            float distance = glm::length(programState->camera.Position - diamond.first);
            diamondsSorted[distance] = diamond;
        }
        // Add the transparent box to the map so it can be sorted too
        float boxDistance = glm::length(programState->camera.Position - scene->transparentBoxPosition);
        diamondsSorted[boxDistance] = transparentBox;

        for(auto diamond = diamondsSorted.rbegin(); diamond != diamondsSorted.rend(); diamond++){
            if(diamond->second.first == scene->transparentBoxPosition){
                diamondShader.use();
                diamondShader.setMat4("projection", projection);
                diamondShader.setMat4("view", view);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, transparentBoxTexture);
                glBindVertexArray(boxVAO);
                glm::mat4 modelBox = glm::mat4(1.0f);
                modelBox = glm::translate(modelBox, scene->transparentBoxPosition);
                modelBox = glm::scale(modelBox, glm::vec3(1.7f));
                diamondShader.setMat4("model", modelBox);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            else{
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, diamond->second.second);
                diamondShader.use();
                diamondShader.setMat4("projection", projection);
                diamondShader.setMat4("view", view);
                glm::mat4 modelDiamond = glm::mat4(1.0f);
                modelDiamond = glm::translate(modelDiamond, diamond->second.first);
                modelDiamond = glm::rotate(modelDiamond, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
                modelDiamond = glm::scale(modelDiamond, glm::vec3(0.05f));
                diamondShader.setMat4("model", modelDiamond);
                diamondModel.Draw(diamondShader);
            }
        }

// Render the hidden room
//------------------------------------------------------------------
}else if(scene->inside){

        glDisable(GL_CULL_FACE);

        // Hidden room rendering
        //----------------------------------------------------------
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, stoneTexture);

        roomShader.use();
        roomShader.setVec3("lights.Position", scene->roomLightPosition);
        roomShader.setVec3("lights.Color", scene->roomLightColor);
        roomShader.setVec3("viewPos", programState->camera.Position);

        roomShader.setMat4("projection", projection);
        roomShader.setMat4("view", view);

        renderer.renderRoomScene(roomShader);

        ourShader.use();
        scene->setLights(ourShader, programState);
        ourShader.setFloat("material.shininess", 32.0f);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        renderer.renderRoomPipe(ourShader, pipeModel);

        starShader.use();
        starShader.setMat4("projection", projection);
        starShader.setMat4("view", view);
        starShader.setVec3("lightColor", scene->roomLightColor);

        if(!scene->starCatched)
            renderer.renderStar(starShader, starModel);

        glEnable(GL_CULL_FACE);
}


        // Draw skybox as last
        //----------------------------------------------------------
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix()));
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
        skyboxShader.setMat4("projection", projection);
        // Skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // Blur bright fragments with two-pass Gaussian Blur
        //----------------------------------------------------------
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        blurShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            renderer.renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, effectFBO);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        bloomShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        bloomShader.setInt("hdr", hdr);
        bloomShader.setInt("bloom", bloom);
        bloomShader.setFloat("exposure", exposure);
        renderer.renderQuad();

        // Sharpen effect
        //----------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        effectShader.use();
        effectShader.setBool("effect", sharpenEffect);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, effectColorBuffer);

        renderer.renderQuad();


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

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
        character->characterAngle += 2.0f;
        if(character->characterAngle >= 360.0f)
            character->characterAngle -= 360.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
        character->characterAngle -= 2.0f;
        if(character->characterAngle <= -360.0f)
            character->characterAngle += 360.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !scene->boxRising){
        character->characterPosition.x += character->characterSpeed * (float)sin(glm::radians(character->characterAngle));
        character->characterPosition.z += character->characterSpeed * (float)cos(glm::radians(character->characterAngle));
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && !scene->boxRising){
        character->characterPosition.x -= character->characterSpeed * (float)sin(glm::radians(character->characterAngle));
        character->characterPosition.z -= character->characterSpeed * (float)cos(glm::radians(character->characterAngle));
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
        if(character->currentCharacter == Character::ghost)
            character->characterPosition.y += 0.05;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
        if(character->currentCharacter == Character::ghost)
            character->characterPosition.y -= 0.05;
    }
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
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS){
        if(character->currentCharacter == Character::mario && !scene->boxRising)
            character->jump = true;
    }

    if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS){
        if(character->currentCharacter == Character::mario)
            scene->mushroomVisible = false;
    }

    if(key == GLFW_KEY_1 && action == GLFW_PRESS){
        if(bloom){
            bloom = false;
            std::cout << "Bloom: off" << '\n';
        }
        else{
            bloom = true;
            std::cout << "Bloom: on" << '\n';
        }
    }

    if(key == GLFW_KEY_2 && action == GLFW_PRESS){
        if(hdr){
            hdr = false;
            std::cout << "HDR: off" << '\n';
        }
        else{
            hdr = true;
            std::cout << "HDR: on" << '\n';
        }
    }

    if(key == GLFW_KEY_3 && action == GLFW_PRESS){
        if(sharpenEffect){
            sharpenEffect = false;
            std::cout << "Sharpen effect: off" << '\n';
        }
        else{
            sharpenEffect = true;
            std::cout << "Sharpen effect: on" << '\n';
        }
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS)
        scene->spotlightOn = !scene->spotlightOn;

    if(key == GLFW_KEY_Q && action == GLFW_PRESS){
        if(exposure > 0.1f)
            exposure -= 0.1;

        std::cout << "Exposure: " << exposure << '\n';
    }

    if(key == GLFW_KEY_E && action == GLFW_PRESS){
        if(exposure < 5.0f)
            exposure += 0.1;

        std::cout << "Exposure: " << exposure << '\n';
    }

    if(key == GLFW_KEY_C && action == GLFW_PRESS){
        if(!scene->inside){
            if(character->currentCharacter == Character::mario)
                character->currentCharacter = Character::ghost;
            else
                character->currentCharacter = Character::mario;

            character->characterPosition = glm::vec3(-5.0f, -3.0f, 0.2f);
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

void stateCheck()
{
    character->marioColorCheck();
    character->jumpCheck(*scene);
    character->fallCheck();
    scene->mushroomCheck();
    scene->roomCheck(*character, programState);
    scene->starCheck(*character);

    if(character->marioColor == scene->boxColor)
        scene->boxCheck(*character);
}
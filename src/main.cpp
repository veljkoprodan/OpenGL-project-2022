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

unsigned int loadTexture(char const * path, bool gammaCorrection);

void renderQuad();

void renderCube();

void checkMarioColor();

bool isOnPoint(float x, float z, float delta);

bool spotlightOn = false;

// Mario color
enum enumMarioColor {red, green, blue, lightblue, yellow, pink};
enumMarioColor marioColor = red;

// Mario jump
bool jump = false;
float jumpSpeed = 0.1;
float currentJumpHeight = 0;
float jumpLimit = 1.7f;

// Mario position
glm::vec3 marioPosition = glm::vec3(-5.0f, -3.0f, 0.2f);

//star catching
bool isStarCatched = false;
void catchStar();

// Mario movement
float marioAngle = 180.0f;
float marioSpeed = 0.15f;

// mushroom
bool mushroomVisible = false;
float mushroomHeight = 0;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool bloom = true;
bool hdr = true;
bool effect = false;
float exposure = 1.0f;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//lightPos
glm::vec3 lightPos(-4.0f, 2.4f, 2.0f);
glm::vec3 roomLightPosition(0.0f,  117.3f, 25.5f);
glm::vec3 roomLightColor(50.0f, 50.0f, 50.0f);

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
    Shader ourShader("resources/shaders/shader.vs", "resources/shaders/shader.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader brickBoxShader("resources/shaders/shader.vs", "resources/shaders/shader.fs");
    Shader marioBoxShader("resources/shaders/shader.vs", "resources/shaders/shader.fs");
    Shader diamondShader("resources/shaders/testDiamondShader.vs", "resources/shaders/testDiamondShader.fs");
    Shader coinShader("resources/shaders/coinInstancingShader.vs", "resources/shaders/coinInstancingShader.fs");
    Shader starShader("resources/shaders/star.vs", "resources/shaders/star.fs");
    Shader roomShader("resources/shaders/room.vs", "resources/shaders/room.fs");
    Shader blurShader("resources/shaders/blur.vs", "resources/shaders/blur.fs");
    Shader bloomShader("resources/shaders/bloom.vs", "resources/shaders/bloom.fs");
    Shader effectShader("resources/shaders/effect.vs", "resources/shaders/effect.fs");

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
    unsigned int questionambientMap  = loadTexture(FileSystem::getPath("resources/textures/mario_ambient.jpg").c_str(),true);
    unsigned int questiondiffuseMap  = loadTexture(FileSystem::getPath("resources/textures/mario_cube.jpg").c_str(),true);
    unsigned int questionspecularMap = loadTexture(FileSystem::getPath("resources/textures/mario_specular.jpg").c_str(),true);

    //load brick cube textures
    unsigned int brickambientMap  = loadTexture(FileSystem::getPath("resources/textures/brick_ambient.jpg").c_str(),true);
    unsigned int brickdiffuseMap  = loadTexture(FileSystem::getPath("resources/textures/brick_diffuse.jpg").c_str(),true);
    unsigned int brickspecularMap = loadTexture(FileSystem::getPath("resources/textures/brick_specular.jpg").c_str(),true);

    //load room texture
    unsigned int stoneTexture = loadTexture(FileSystem::getPath("resources/textures/stone_texture.jpeg").c_str(), true); // note that we're loading the texture as an SRGB texture

    //configuring floating point framebuffer
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
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
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
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

    //setting uniform in shaders for bloom
    roomShader.use();
    roomShader.setInt("diffuseTexture", 0);
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
    marioBoxShader.use();
    marioBoxShader.setInt("material.ambient", 0);
    marioBoxShader.setInt("material.diffuse", 1);
    marioBoxShader.setInt("material.specular", 2);

    // brick box shader configuration
    brickBoxShader.use();
    brickBoxShader.setInt("material.ambient", 0);
    brickBoxShader.setInt("material.diffuse", 1);
    brickBoxShader.setInt("material.specular", 2);
    stbi_set_flip_vertically_on_load(false);

    // diamond positions and textures
    std::vector< std::pair<glm::vec3, unsigned int> > diamonds;

    unsigned int redDiamondTexture = loadTexture(
            FileSystem::getPath("resources/textures/diamonds/red-transparent.png").c_str(),true);
    diamonds.push_back({glm::vec3(-19.0f, -4.0f, 2.0f), redDiamondTexture});

    unsigned int blueDiamondTexture = loadTexture(
            FileSystem::getPath("resources/textures/diamonds/blue-transparent.png").c_str(),true);
    diamonds.push_back({glm::vec3(-20.0f, -4.0f, 4.0f), blueDiamondTexture});

    unsigned int greenDiamondTexture = loadTexture(
            FileSystem::getPath("resources/textures/diamonds/green-transparent.png").c_str(),true);
    diamonds.push_back({glm::vec3(-19.0f, -4.0f, 6.0f), greenDiamondTexture});

    unsigned int lightBlueDiamondTexture = loadTexture(
            FileSystem::getPath("resources/textures/diamonds/light-blue-transparent.png").c_str(),true);
    diamonds.push_back({glm::vec3(-17.0f, -4.0f, 6.0f), lightBlueDiamondTexture});

    unsigned int yellowDiamondTexture = loadTexture(
            FileSystem::getPath("resources/textures/diamonds/yellow-transparent.png").c_str(),true);
    diamonds.push_back({glm::vec3(-16.0f, -4.0f, 4.0f), yellowDiamondTexture});

    unsigned int pinkDiamondTexture = loadTexture(
            FileSystem::getPath("resources/textures/diamonds/pink-transparent.png").c_str(),true);
    diamonds.push_back({glm::vec3(-17.0f, -4.0f, 2.0f), pinkDiamondTexture});

    diamondShader.use();
    diamondShader.setInt("texture1", 0);


    // Mario textures
    unsigned int marioTextureDefault = loadTexture(
            FileSystem::getPath("resources/textures/mario/default.jpg").c_str(),true);
    unsigned int marioTextureGreen = loadTexture(
            FileSystem::getPath("resources/textures/mario/green.jpg").c_str(),true);
    unsigned int marioTextureBlue = loadTexture(
            FileSystem::getPath("resources/textures/mario/blue.jpg").c_str(),true);
    unsigned int marioTextureLightblue = loadTexture(
            FileSystem::getPath("resources/textures/mario/lightblue.jpg").c_str(),true);
    unsigned int marioTextureYellow = loadTexture(
            FileSystem::getPath("resources/textures/mario/yellow.jpg").c_str(),true);
    unsigned int marioTexturePink = loadTexture(
            FileSystem::getPath("resources/textures/mario/pink.jpg").c_str(),true);

    ourShader.use();
    ourShader.setInt("texture1", 0);


    // load models
    // -----------

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

    // instancing
    unsigned int coinAmount = 10;
    glm::mat4* modelMatrices;
    modelMatrices = new glm::mat4[coinAmount];
    float offset = 3.0f;

    for (unsigned int i = 0; i < coinAmount; i++){
        glm::mat4 modelCoin = glm::mat4(1.0f);

        if(i < 4)
            modelCoin = glm::translate(modelCoin, glm::vec3(-1.1f, 5.3f, -0.1f + i*1.9f));
        else if(i < 7)
            modelCoin = glm::translate(modelCoin, glm::vec3(0.35f + 1.95f * (i-4), 5.3f, 7.7f));
        else
            modelCoin = glm::translate(modelCoin, glm::vec3(0.35f + 1.95f * (i-7), 5.3f, -2.2f));

        modelCoin = glm::rotate(modelCoin, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
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

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }


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

        jumpCheck();
        mushroomCheck();
        checkMarioColor();
        catchStar();

        //teleport to room and back
        if(isOnPoint(-1.6f,-8.2f, 0.6f)){
            marioPosition = glm::vec3 (2.5f, 117.5f, 22.0f);
            programState->camera.Position = glm::vec3(-1.4f, 123.5f,26.5f);
        }

        if(isOnPoint(2.5f, 22.0f, 0.6f) && isStarCatched){
            marioPosition = glm::vec3 (-5.0f, -3.0f, 0.2f);
            programState->camera.Position = glm::vec3(-5.9f, -3.6f, -3.0f);
            isStarCatched = false;
        }

        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();

        coinShader.use();
        setLights(coinShader);
        coinShader.setFloat("material.shininess", 128.0f);
        coinShader.setMat4("projection", projection);
        coinShader.setMat4("view", view);
        coinShader.setInt("texture_diffuse1", 0);
        coinShader.setInt("texture_specular1", 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, coinModel.textures_loaded[0].id);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, coinModel.textures_loaded[1].id);

        // instancing
        for (unsigned int i = 0; i < coinModel.meshes.size(); i++)
        {
            glBindVertexArray(coinModel.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(
                    coinModel.meshes[i].indices.size()), GL_UNSIGNED_INT, 0, coinAmount);
            glBindVertexArray(0);
        }


        ourShader.use();
        setLights(ourShader);
        ourShader.setFloat("material.shininess", 32.0f);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        glm::mat4 modelMushroom = glm::mat4(1.0f);
        modelMushroom = glm::translate(modelMushroom, glm::vec3(-5.0f, -0.3f + mushroomHeight, 0.0f));
        modelMushroom = glm::rotate(modelMushroom, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMushroom = glm::scale(modelMushroom, glm::vec3(0.3f));
        ourShader.setMat4("model", modelMushroom);
        mushroomModel.Draw(ourShader);

        glm::mat4 modelShip = glm::mat4(1.0f);
        modelShip = glm::translate(modelShip, glm::vec3(-18.0f, 0.0f, 0.0f));
        modelShip = glm::scale(modelShip, glm::vec3(10.0f));
        ourShader.setMat4("model", modelShip);
        shipModel.Draw(ourShader);



        glDisable(GL_CULL_FACE);

        glm::mat4 modelIsland = glm::mat4(1.0f);
        modelIsland = glm::translate(modelIsland, glm::vec3 (0.0f, 0.0f, 0.0f));
        modelIsland = glm::scale(modelIsland, glm::vec3(10.0f));
        ourShader.setMat4("model", modelIsland);
        islandModel.Draw(ourShader);

        glm::mat4 modelPipe = glm::mat4(1.0f);
        modelPipe = glm::translate(modelPipe, glm::vec3(-5.9f, -3.6f, -3.0f));
        modelPipe = glm::scale(modelPipe, glm::vec3(0.5f));
        ourShader.setMat4("model", modelPipe);
        pipeModel.Draw(ourShader);

        modelPipe = glm::mat4(1.0f);
        modelPipe = glm::translate(modelPipe, glm::vec3( -1.0f,  116.5f, 26.5f)); // translate it down so it's at the center of the scene
        modelPipe = glm::scale(modelPipe, glm::vec3(0.5f, 0.5f, 0.5f));	// it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", modelPipe);
        pipeModel.Draw(ourShader);

        // Mario
        glActiveTexture(GL_TEXTURE0);

        if(marioColor == red)
            glBindTexture(GL_TEXTURE_2D, marioTextureDefault);
        else if(marioColor == green)
            glBindTexture(GL_TEXTURE_2D, marioTextureGreen);
        else if(marioColor == blue)
            glBindTexture(GL_TEXTURE_2D, marioTextureBlue);
        else if(marioColor == lightblue)
            glBindTexture(GL_TEXTURE_2D, marioTextureLightblue);
        else if(marioColor == yellow)
            glBindTexture(GL_TEXTURE_2D, marioTextureYellow);
        else if(marioColor == pink)
            glBindTexture(GL_TEXTURE_2D, marioTexturePink);

        //render star
        starShader.use();

        starShader.setMat4("projection", projection);
        starShader.setMat4("view", view);
        starShader.setVec3("lightColor", roomLightColor);

        glm::mat4 modelStar = glm::mat4(1.0f);
        modelStar = glm::translate(modelStar, glm::vec3(0.0f,115.0f,28.0f)); // translate it down so it's at the center of the scene
        modelStar = glm::rotate(modelStar,(float)glfwGetTime(), glm::vec3(0.0f,1.0f,0.0f));
        modelStar = glm::scale(modelStar, glm::vec3(5.0f, 5.0f, 5.0f));	// it's a bit too big for our scene, so scale it down
        starShader.setMat4("model", modelStar);
        if(!isStarCatched)
            starModel.Draw(ourShader);

        ourShader.use();
        setLights(ourShader);
        ourShader.setFloat("material.shininess", 32.0f);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        glm::mat4 modelMario = glm::mat4(1.0f);
        modelMario = glm::translate(modelMario, marioPosition);
        modelMario = glm::rotate(modelMario, glm::radians(marioAngle - 180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMario = glm::scale(modelMario, glm::vec3(0.4f));
        ourShader.setMat4("model", modelMario);
        marioModel.Draw(ourShader);

        //room rendering
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, stoneTexture);

        roomShader.use();
        roomShader.setVec3("lights.Position", roomLightPosition);
        roomShader.setVec3("lights.Color", roomLightColor);
        roomShader.setVec3("viewPos", programState->camera.Position);
        roomShader.setInt("inverse_normals", true);

        roomShader.setMat4("projection", projection);
        roomShader.setMat4("view", view);
        glm::mat4 modelRoom = glm::mat4(1.0f);
        modelRoom = glm::translate(modelRoom, glm::vec3(0.0f, 120.0f, 25.0));
        modelRoom = glm::scale(modelRoom, glm::vec3(5.5f, 3.5f, 5.5f));
        roomShader.setMat4("model", modelRoom);
        renderCube();

        //brick box
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, brickambientMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, brickdiffuseMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brickspecularMap);

        brickBoxShader.use();
        setLights(brickBoxShader);
        brickBoxShader.setFloat("material.shininess", 32.0f);
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
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, questionambientMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, questiondiffuseMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, questionspecularMap);

        marioBoxShader.use();
        setLights(marioBoxShader);
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


        // diamonds

        //      distance         translation   texture
        std::map<float, std::pair<glm::vec3, unsigned int>> diamondsSorted;

        for(std::pair<glm::vec3, unsigned int> diamond : diamonds){
            float distance = glm::length(programState->camera.Position - diamond.first);
            diamondsSorted[distance] = diamond;
        }

        for(auto diamond = diamondsSorted.rbegin(); diamond != diamondsSorted.rend(); diamond++){
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

        glEnable(GL_CULL_FACE);

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

        glDisable(GL_CULL_FACE);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. blur bright fragments with two-pass Gaussian Blur
        // --------------------------------------------------
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        blurShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            renderQuad();
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
        renderQuad();

        //effect
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        effectShader.use();
        effectShader.setBool("effect", effect);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, effectColorBuffer);

        renderQuad();

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
        marioAngle += 2.0f;
        if(marioAngle >= 360.0f)
            marioAngle -= 360.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
        marioAngle -= 2.0f;
        if(marioAngle <= -360.0f)
            marioAngle += 360.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
        marioPosition.x += marioSpeed * (float)sin(glm::radians(marioAngle));
        marioPosition.z += marioSpeed * (float)cos(glm::radians(marioAngle));
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
        marioPosition.x -= marioSpeed * (float)sin(glm::radians(marioAngle));
        marioPosition.z -= marioSpeed * (float)cos(glm::radians(marioAngle));
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
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS)
        spotlightOn = !spotlightOn;

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        jump = true;

    if(key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
        mushroomVisible = false;

    if(key == GLFW_KEY_1 && action == GLFW_PRESS)
        bloom = !bloom;

    if(key == GLFW_KEY_2 && action == GLFW_PRESS)
        hdr = !hdr;

    if(key == GLFW_KEY_3 && action == GLFW_PRESS)
        effect = !effect;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
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

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
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

void setLights(Shader shader){
    shader.setVec3("light.position", lightPos);
    shader.setVec3("viewPos", programState->camera.Position);

    // directional light
    shader.setVec3("dirLight.direction", 0.0f, -1.0, 0.0f);
    shader.setVec3("dirLight.ambient", 0.01f, 0.01f, 0.01f);
    shader.setVec3("dirLight.diffuse", 0.08f, 0.08f, 0.08f);
    shader.setVec3("dirLight.specular", 0.1f, 0.1f, 0.1f);
    //pointlight properties
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

unsigned int loadTexture(char const * path, bool gammaCorrection)
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

void jumpCheck(){
    if(jump){
        currentJumpHeight += jumpSpeed;
        marioPosition.y += jumpSpeed;
    }

    if(currentJumpHeight >= jumpLimit){
        jump = false;
        if(isOnPoint(-5.0f, 0.0f, 0.5f))
            mushroomVisible = true;
    }

    if(jump == false && currentJumpHeight > 0){
        currentJumpHeight -= jumpSpeed;
        marioPosition.y -= jumpSpeed;
    }
}

void checkMarioColor(){
    if(isOnPoint(-19.0f, 2.0f, 0.6f))
        marioColor = red;
    else if(isOnPoint(-20.0f, 4.0f, 0.6f))
        marioColor = blue;
    else if(isOnPoint(-19.0f, 6.0f, 0.6f))
        marioColor = green;
    else if(isOnPoint(-17.0f, 6.0f, 0.6f))
        marioColor = lightblue;
    else if(isOnPoint(-16.0f, 4.0f, 0.6f))
        marioColor = yellow;
    else if(isOnPoint(-17.0f, 2.0f, 0.6f))
        marioColor = pink;
}

bool isOnPoint(float x, float z, float delta){
    return(marioPosition.x > x-delta && marioPosition.x < x+delta
           && marioPosition.z > z-delta && marioPosition.z < z + delta);
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

void catchStar(){
    if(isOnPoint(0.0f, 25.0f, 0.5f))
        isStarCatched = true;
}
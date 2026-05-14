/*
*Proyecto Final
*Fecha de Entrega: 13/05/26
*319271108
*/

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <cmath> // Para std::abs

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <irrKlang.h>
using namespace irrklang;

// Model loading classes
#include <shader_m.h>
#include <camera.h>
#include <model.h>
#include <animatedmodel.h>
#include <light.h>
#include <cubemap.h>

// Functions
bool Start();
bool Update();
void InitSounds();

// Helpers de Iluminación
void SetLightUniformInt(Shader* shader, const char* propertyName, size_t lightIndex, int value);
void SetLightUniformFloat(Shader* shader, const char* propertyName, size_t lightIndex, float value);
void SetLightUniformVec4(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec4 value);
void SetLightUniformVec3(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec3 value);

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// Globals
GLFWwindow* window;

// Window size
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// Camera (Posición inicial cerca del policía, ligeramente elevada)
Camera camera(glm::vec3(16.0f, 8.98f, 2.5f));

// Mouse
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Variables de Estado (Día/Noche, Cámara, TP)
bool esDia = true;
bool freeCamera = false;
float fixedCameraY = 8.98f; // Altura fija para el modo piso

bool keyTPressed = false;
bool keyCPressed = false;
bool key1Pressed = false;
bool key2Pressed = false;

#pragma region Transformaciones
glm::vec3 expositor1_position(-1.0f, 6.98f, -1.0f);
glm::vec3 expositor2_position(-14.8f, 6.98f, -1.4f);
glm::vec3 expositor3_position(-7.6f, 6.98f, 1.4f);
glm::vec3 expositor4_position(6.5f, 6.98f, 1.4f);
glm::vec3 policia_position(16.0f, 6.98f, 0.5f);
glm::vec3 miku_position(-1.0f, 14.8f, -1.0f);

glm::vec3 scale_expositors(0.012f, 0.012f, 0.012f);
glm::vec3 scale_expositors_small(0.0012f, 0.0012f, 0.0012f);
glm::vec3 scale_expositors_xs(0.0009f, 0.0009f, 0.0009f);
glm::vec3 scale_miku(0.0005f, 0.0005f, 0.0005f);
#pragma endregion

// Shaders
Shader* phongShader;
Shader* dynamicShader;
Shader* cubemapShader;

// Static models
Model* house;
Model* escenario;

// Animated models
AnimatedModel* expositor1;
AnimatedModel* expositor2;
AnimatedModel* expositor3;
AnimatedModel* policia;
AnimatedModel* miku;

// Audio
ISoundEngine* SoundEngine = createIrrKlangDevice();
ISound* musicaFondo = nullptr;

// Cubemaps
CubeMap* skyboxdia = nullptr;
CubeMap* skyboxnoche = nullptr;

int main()
{
    InitSounds();

    if (!Start())
        return -1;

    while (!glfwWindowShouldClose(window))
    {
        if (!Update())
            break;
    }

    glfwTerminate();
    return 0;
}

bool Start()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto Final", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);

    phongShader = new Shader("Shader/11_PhongShaderMultLights.vs", "Shader/11_PhongShaderMultLights.fs");
    dynamicShader = new Shader("Shader/09_vertex_skinning.vs", "Shader/09_fragment_skinning.fs");
    cubemapShader = new Shader("Shader/10_vertex_cubemap.vs", "Shader/10_fragment_cubemap.fs");

    dynamicShader->setBonesIDs(MAX_RIGGING_BONES);

    // Load models
    house = new Model("models/Lobby/lobby.obj");
    escenario = new Model("models/Lobby/escenario.obj");

    miku = new AnimatedModel("models/Lobby/miku.fbx");
    expositor1 = new AnimatedModel("models/Lobby/expositor_1.fbx");
    expositor2 = new AnimatedModel("models/Lobby/expositor_2.fbx");
    expositor3 = new AnimatedModel("models/Lobby/expositor_3.fbx");
    policia = new AnimatedModel("models/Lobby/poli.fbx");

    // Load cubemap DÍA
    std::vector<std::string> dayFaces = {
        "cubemap/dia/px.png", "cubemap/dia/nx.png",
        "cubemap/dia/py.png", "cubemap/dia/ny.png",
        "cubemap/dia/pz.png", "cubemap/dia/nz.png"
    };
    skyboxdia = new CubeMap();
    skyboxdia->loadCubemap(dayFaces);

    // Load cubemap NOCHE 
    std::vector<std::string> nightFaces = {
        "cubemap/noche/px.png", "cubemap/noche/nx.png",
        "cubemap/noche/py.png", "cubemap/noche/ny.png",
        "cubemap/noche/pz.png", "cubemap/noche/nz.png"
    };
    skyboxnoche = new CubeMap();
    skyboxnoche->loadCubemap(nightFaces);

    return true;
}

void InitSounds()
{
    // Iniciamos la música pausada por defecto
    musicaFondo = SoundEngine->play2D("sound/bosque-120184.mp3", true, true, true);
}

bool Update()
{
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    // --- SISTEMA DE AUDIO POR RANGO (MIKU) ---
    float distanciaXMiku = std::abs(camera.Position.x - miku_position.x);
    float distanciaZMiku = std::abs(camera.Position.z - miku_position.z);

    if (distanciaXMiku <= 10.0f && distanciaZMiku <= 10.0f) {
        if (musicaFondo && musicaFondo->getIsPaused()) {
            musicaFondo->setIsPaused(false);
        }
    }
    else {
        if (musicaFondo && !musicaFondo->getIsPaused()) {
            musicaFondo->setIsPaused(true);
        }
    }
    // ------------------------------------------

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
        (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
    glm::mat4 view = camera.GetViewMatrix();

    // Cubemap
    {
        if (esDia) skyboxdia->drawCubeMap(*cubemapShader, projection, view);
        else skyboxnoche->drawCubeMap(*cubemapShader, projection, view);
    }
    glUseProgram(0);

    // Static objects (Afectados por luz)
    {
        phongShader->use();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        phongShader->setMat4("projection", projection);
        phongShader->setMat4("view", view);
        phongShader->setVec3("eye", camera.Position);

        phongShader->setVec4("MaterialAmbientColor", glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));
        phongShader->setVec4("MaterialDiffuseColor", glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
        phongShader->setVec4("MaterialSpecularColor", glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
        phongShader->setFloat("transparency", 1.0f);

        if (esDia) {
            phongShader->setInt("numLights", 1);
            SetLightUniformVec3(phongShader, "Position", 0, glm::vec3(0.0f, 150.0f, 0.0f));
            SetLightUniformVec3(phongShader, "Direction", 0, glm::vec3(0.0f, -1.0f, 0.0f));
            SetLightUniformVec4(phongShader, "Color", 0, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

            // CORRECCIÓN MAGNA: Power más bajo y distance en 1.0f
            SetLightUniformVec4(phongShader, "Power", 0, glm::vec4(1.5f, 1.5f, 1.5f, 1.0f));
            SetLightUniformInt(phongShader, "alphaIndex", 0, 100);
            SetLightUniformFloat(phongShader, "distance", 0, 1.0f);
        }
        else {
            phongShader->setInt("numLights", 4);
            glm::vec4 warmColor(1.0f, 0.7f, 0.3f, 1.0f);

            // CORRECCIÓN MAGNA: Power ajustado
            glm::vec4 warmPower(1.2f, 1.2f, 1.2f, 1.0f);

            // Luz 1
            SetLightUniformVec3(phongShader, "Position", 0, glm::vec3(-5.0f, 5.0f, -5.0f));
            SetLightUniformVec4(phongShader, "Color", 0, warmColor);
            SetLightUniformVec4(phongShader, "Power", 0, warmPower);
            SetLightUniformFloat(phongShader, "distance", 0, 1.0f); // distance a 1.0f
            SetLightUniformInt(phongShader, "alphaIndex", 0, 10);

            // Luz 2
            SetLightUniformVec3(phongShader, "Position", 1, glm::vec3(5.0f, 5.0f, -5.0f));
            SetLightUniformVec4(phongShader, "Color", 1, warmColor);
            SetLightUniformVec4(phongShader, "Power", 1, warmPower);
            SetLightUniformFloat(phongShader, "distance", 1, 1.0f); // distance a 1.0f
            SetLightUniformInt(phongShader, "alphaIndex", 1, 10);

            // Luz 3
            SetLightUniformVec3(phongShader, "Position", 2, glm::vec3(-5.0f, 5.0f, 5.0f));
            SetLightUniformVec4(phongShader, "Color", 2, warmColor);
            SetLightUniformVec4(phongShader, "Power", 2, warmPower);
            SetLightUniformFloat(phongShader, "distance", 2, 1.0f); // distance a 1.0f
            SetLightUniformInt(phongShader, "alphaIndex", 2, 10);

            // Luz 4
            SetLightUniformVec3(phongShader, "Position", 3, glm::vec3(5.0f, 5.0f, 5.0f));
            SetLightUniformVec4(phongShader, "Color", 3, warmColor);
            SetLightUniformVec4(phongShader, "Power", 3, warmPower);
            SetLightUniformFloat(phongShader, "distance", 3, 1.0f); // distance a 1.0f
            SetLightUniformInt(phongShader, "alphaIndex", 3, 10);
        }

        glm::mat4 model = glm::mat4(1.0f);
        phongShader->setMat4("model", model);
        house->Draw(*phongShader);

        glm::mat4 modelEscenario = glm::mat4(1.0f);
        phongShader->setMat4("model", modelEscenario);
        escenario->Draw(*phongShader);
    }
    glUseProgram(0);

    // Miku 
    {
        miku->UpdateAnimation(deltaTime);
        dynamicShader->use();
        dynamicShader->setMat4("projection", projection);
        dynamicShader->setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, miku_position);
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, scale_miku);
        dynamicShader->setMat4("model", model);
        dynamicShader->setMat4("gBones", MAX_RIGGING_BONES, miku->gBones);
        miku->Draw(*dynamicShader);
    }
    glUseProgram(0);

    // Expositor 1
    {
        expositor1->UpdateAnimation(deltaTime);
        dynamicShader->use();
        dynamicShader->setMat4("projection", projection);
        dynamicShader->setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, expositor1_position);
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, scale_expositors);
        dynamicShader->setMat4("model", model);
        dynamicShader->setMat4("gBones", MAX_RIGGING_BONES, expositor1->gBones);
        expositor1->Draw(*dynamicShader);
    }
    glUseProgram(0);

    // Expositor 2
    {
        expositor2->UpdateAnimation(deltaTime);
        dynamicShader->use();
        dynamicShader->setMat4("projection", projection);
        dynamicShader->setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, expositor2_position);
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, scale_expositors_xs);
        dynamicShader->setMat4("model", model);
        dynamicShader->setMat4("gBones", MAX_RIGGING_BONES, expositor2->gBones);
        expositor2->Draw(*dynamicShader);
    }
    glUseProgram(0);

    // Expositor 3
    {
        expositor3->UpdateAnimation(deltaTime);
        dynamicShader->use();
        dynamicShader->setMat4("projection", projection);
        dynamicShader->setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, expositor3_position);
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, scale_expositors_xs);
        dynamicShader->setMat4("model", model);
        dynamicShader->setMat4("gBones", MAX_RIGGING_BONES, expositor3->gBones);
        expositor3->Draw(*dynamicShader);
    }
    glUseProgram(0);

    // Policia
    {
        policia->UpdateAnimation(deltaTime);
        dynamicShader->use();
        dynamicShader->setMat4("projection", projection);
        dynamicShader->setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, policia_position);
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, scale_expositors_small);
        dynamicShader->setMat4("model", model);
        dynamicShader->setMat4("gBones", MAX_RIGGING_BONES, policia->gBones);
        policia->Draw(*dynamicShader);
    }
    glUseProgram(0);

    glfwSwapBuffers(window);
    glfwPollEvents();
    return true;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Controles de Modo (Día/Noche con T)
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !keyTPressed) {
        esDia = !esDia;
        keyTPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
        keyTPressed = false;
    }

    // Controles de Cámara (Libre/Piso con C)
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !keyCPressed) {
        freeCamera = !freeCamera;
        if (!freeCamera) fixedCameraY = camera.Position.y;
        keyCPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
        keyCPressed = false;
    }

    // Teletransporte a Miku (Tecla 1)
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !key1Pressed) {
        camera.Position = glm::vec3(miku_position.x, miku_position.y + 1.0f, miku_position.z + 2.5f);
        fixedCameraY = camera.Position.y;
        key1Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) {
        key1Pressed = false;
    }

    // Teletransporte al Policía (Tecla 2)
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !key2Pressed) {
        camera.Position = glm::vec3(policia_position.x, policia_position.y + 2.0f, policia_position.z + 2.5f);
        fixedCameraY = camera.Position.y;
        key2Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) {
        key2Pressed = false;
    }

    // Movimiento
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // Controles de altura manual (Solo en modo libre)
    if (freeCamera) {
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera.ProcessKeyboard(UP_DIR, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWN_DIR, deltaTime);
    }
    else {
        camera.Position.y = fixedCameraY;
    }

    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;

    lastX = (float)xpos;
    lastY = (float)ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}

// Helpers Iluminación
void SetLightUniformInt(Shader* shader, const char* propertyName, size_t lightIndex, int value) {
    std::string uniformName = "allLights[" + std::to_string(lightIndex) + "]." + propertyName;
    shader->setInt(uniformName.c_str(), value);
}

void SetLightUniformFloat(Shader* shader, const char* propertyName, size_t lightIndex, float value) {
    std::string uniformName = "allLights[" + std::to_string(lightIndex) + "]." + propertyName;
    shader->setFloat(uniformName.c_str(), value);
}

void SetLightUniformVec4(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec4 value) {
    std::string uniformName = "allLights[" + std::to_string(lightIndex) + "]." + propertyName;
    shader->setVec4(uniformName.c_str(), value);
}

void SetLightUniformVec3(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec3 value) {
    std::string uniformName = "allLights[" + std::to_string(lightIndex) + "]." + propertyName;
    shader->setVec3(uniformName.c_str(), value);
}
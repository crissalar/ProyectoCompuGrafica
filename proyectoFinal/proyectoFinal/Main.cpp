/*
*Proyecto Final
*Fecha de Entrega: 20/05/26
*319271108
*/

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <irrKlang.h>
using namespace irrklang;

#include <shader_m.h>
#include <camera.h>
#include <model.h>
#include <animatedmodel.h>
#include <light.h>
#include <cubemap.h>

bool Start();
bool Update();
void InitSounds();

void SetLightUniformInt(Shader* shader, const char* propertyName, size_t lightIndex, int value);
void SetLightUniformFloat(Shader* shader, const char* propertyName, size_t lightIndex, float value);
void SetLightUniformVec4(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec4 value);
void SetLightUniformVec3(Shader* shader, const char* propertyName, size_t lightIndex, glm::vec3 value);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);

GLFWwindow* window;

const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

Camera camera(glm::vec3(16.0f, 8.98f, 2.5f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool mouseCapturado = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool esDia = true;
bool freeCamera = false;
float fixedCameraY = 8.98f;

bool keyTPressed = false;
bool keyCPressed = false;
bool key1Pressed = false;
bool key2Pressed = false;

bool keyFPressed = false;
bool puertaAbierta = false;
float puertaRotacionActual = 0.0f;
float puertaRotacionObjetivo = 0.0f;

AnimatedModel* visitante;
glm::vec3 visitante_pos_inicial(-15.0f, 6.98f, 0.0f);
glm::vec3 visitante_pos_final(10.0f, 6.98f, 0.0f);
glm::vec3 visitante_position = visitante_pos_inicial;
float visitante_velocidad = 2.0f;
bool visitante_va_hacia_adelante = true;

#pragma region Transformaciones
glm::vec3 expositor1_position(-1.0f, 6.98f, -1.0f);
glm::vec3 expositor2_position(-14.8f, 6.98f, -1.4f);
glm::vec3 expositor3_position(-7.6f, 6.98f, 1.4f);
glm::vec3 expositor4_position(6.5f, 6.98f, 1.4f);
glm::vec3 policia_position(16.0f, 6.98f, 0.5f);
glm::vec3 miku_position(-1.0f, 14.8f, -1.0f);

glm::vec3 scale_puerta(1.0f, 1.0f, 1.0f);
glm::vec3 scale_expositors(0.012f, 0.012f, 0.012f);
glm::vec3 scale_expositors_small(0.002f, 0.002f, 0.002f);
glm::vec3 scale_expositors_xs(0.0009f, 0.0009f, 0.0009f);
glm::vec3 scale_miku(0.0005f, 0.0005f, 0.0005f);
glm::vec3 scale_visitante(0.005f, 0.005f, 0.005f);
#pragma endregion

Shader* phongShader;
Shader* dynamicShader;
Shader* cubemapShader;

Model* house;
Model* escenario;
Model* puerta;

AnimatedModel* expositor1;
AnimatedModel* expositor2;
AnimatedModel* expositor3;
AnimatedModel* policia;
AnimatedModel* miku;

ISoundEngine* SoundEngine = createIrrKlangDevice();
ISound* musicaFondo = nullptr;

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
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

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

    house = new Model("models/Lobby/lobby.obj");
    escenario = new Model("models/Lobby/escenario.obj");
    puerta = new Model("models/Lobby/puerta.obj");

    miku = new AnimatedModel("models/Lobby/miku.fbx");
    visitante = new AnimatedModel("models/Lobby/kid_walking.fbx");
    expositor1 = new AnimatedModel("models/Lobby/expositor_1.fbx");
    expositor2 = new AnimatedModel("models/Lobby/expositor_2.fbx");
    expositor3 = new AnimatedModel("models/Lobby/expositor_3.fbx");
    policia = new AnimatedModel("models/Lobby/poli.fbx");

    std::vector<std::string> dayFaces = {
        "cubemap/dia/px.png", "cubemap/dia/nx.png",
        "cubemap/dia/py.png", "cubemap/dia/ny.png",
        "cubemap/dia/pz.png", "cubemap/dia/nz.png"
    };
    skyboxdia = new CubeMap();
    skyboxdia->loadCubemap(dayFaces);

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
    musicaFondo = SoundEngine->play2D("sound/bosque-120184.mp3", true, true, true);
}

bool Update()
{
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    // Lógica Puerta
    float rotSpeed = 5.0f;
    puertaRotacionActual += (puertaRotacionObjetivo - puertaRotacionActual) * rotSpeed * deltaTime;

    // Audio dinámico
    float distanciaMiku = glm::length(camera.Position - miku_position);
    float distanciaMaxima = 20.0f;
    float distanciaOptima = 5.0f;

    if (camera.Position.y < 11.0f || distanciaMiku > distanciaMaxima) {
        if (musicaFondo && !musicaFondo->getIsPaused())
            musicaFondo->setIsPaused(true);
    }
    else {
        if (musicaFondo && musicaFondo->getIsPaused())
            musicaFondo->setIsPaused(false);

        if (distanciaMiku <= distanciaOptima) {
            musicaFondo->setVolume(1.0f);
        }
        else {
            float rangoAtenuacion = distanciaMaxima - distanciaOptima;
            float distanciaExcedente = distanciaMiku - distanciaOptima;
            float volumenCalculado = 1.0f - (distanciaExcedente / rangoAtenuacion);
            if (volumenCalculado < 0.0f) volumenCalculado = 0.0f;
            musicaFondo->setVolume(volumenCalculado);
        }
    }

    // Visitante
    visitante->UpdateAnimation(deltaTime);
    glm::vec3 direccion = glm::normalize(visitante_pos_final - visitante_pos_inicial);
    float distanciaTotal = glm::length(visitante_pos_final - visitante_pos_inicial);

    if (visitante_va_hacia_adelante) {
        visitante_position += direccion * visitante_velocidad * deltaTime;
        if (glm::length(visitante_position - visitante_pos_inicial) >= distanciaTotal) {
            visitante_position = visitante_pos_final;
            visitante_va_hacia_adelante = false;
        }
    }
    else {
        visitante_position -= direccion * visitante_velocidad * deltaTime;
        if (glm::length(visitante_position - visitante_pos_final) >= distanciaTotal) {
            visitante_position = visitante_pos_inicial;
            visitante_va_hacia_adelante = true;
        }
    }

    // Renderizado
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

    // Static objects
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
            SetLightUniformVec4(phongShader, "Power", 0, glm::vec4(1.5f, 1.5f, 1.5f, 1.0f));
            SetLightUniformInt(phongShader, "alphaIndex", 0, 100);
            SetLightUniformFloat(phongShader, "distance", 0, 1.0f);
        }
        else {
            phongShader->setInt("numLights", 4);
            glm::vec4 warmColor(1.0f, 0.7f, 0.3f, 1.0f);
            glm::vec4 warmPower(1.2f, 1.2f, 1.2f, 1.0f);

            SetLightUniformVec3(phongShader, "Position", 0, glm::vec3(-5.0f, 5.0f, -5.0f));
            SetLightUniformVec4(phongShader, "Color", 0, warmColor);
            SetLightUniformVec4(phongShader, "Power", 0, warmPower);
            SetLightUniformFloat(phongShader, "distance", 0, 1.0f);
            SetLightUniformInt(phongShader, "alphaIndex", 0, 10);

            SetLightUniformVec3(phongShader, "Position", 1, glm::vec3(5.0f, 5.0f, -5.0f));
            SetLightUniformVec4(phongShader, "Color", 1, warmColor);
            SetLightUniformVec4(phongShader, "Power", 1, warmPower);
            SetLightUniformFloat(phongShader, "distance", 1, 1.0f);
            SetLightUniformInt(phongShader, "alphaIndex", 1, 10);

            SetLightUniformVec3(phongShader, "Position", 2, glm::vec3(-5.0f, 5.0f, 5.0f));
            SetLightUniformVec4(phongShader, "Color", 2, warmColor);
            SetLightUniformVec4(phongShader, "Power", 2, warmPower);
            SetLightUniformFloat(phongShader, "distance", 2, 1.0f);
            SetLightUniformInt(phongShader, "alphaIndex", 2, 10);

            SetLightUniformVec3(phongShader, "Position", 3, glm::vec3(5.0f, 5.0f, 5.0f));
            SetLightUniformVec4(phongShader, "Color", 3, warmColor);
            SetLightUniformVec4(phongShader, "Power", 3, warmPower);
            SetLightUniformFloat(phongShader, "distance", 3, 1.0f);
            SetLightUniformInt(phongShader, "alphaIndex", 3, 10);
        }

        // Lobby
        glm::mat4 model = glm::mat4(1.0f);
        phongShader->setMat4("model", model);
        house->Draw(*phongShader);

        // Escenario
        glm::mat4 modelEscenario = glm::mat4(1.0f);
        phongShader->setMat4("model", modelEscenario);
        escenario->Draw(*phongShader);

        // Puerta
        glm::vec3 pivoteBisagra(19.957f, 13.948f, -3.5561f);
        glm::mat4 modelPuerta = glm::mat4(1.0f);
        modelPuerta = glm::translate(modelPuerta, pivoteBisagra);
        modelPuerta = glm::rotate(modelPuerta, glm::radians(puertaRotacionActual), glm::vec3(0.0f, 1.0f, 0.0f));
        modelPuerta = glm::translate(modelPuerta, -pivoteBisagra);
        modelPuerta = glm::scale(modelPuerta, scale_puerta);
        phongShader->setMat4("model", modelPuerta);
        puerta->Draw(*phongShader);
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

    // Visitante
    {
        dynamicShader->use();
        dynamicShader->setMat4("projection", projection);
        dynamicShader->setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, visitante_position);
        float anguloRotacion = visitante_va_hacia_adelante ? 90.0f : -90.0f;
        model = glm::rotate(model, glm::radians(anguloRotacion), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, scale_visitante);
        dynamicShader->setMat4("model", model);
        dynamicShader->setMat4("gBones", MAX_RIGGING_BONES, visitante->gBones);
        visitante->Draw(*dynamicShader);
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
    // ESC: liberar mouse o cerrar ventana
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (mouseCapturado) {
            mouseCapturado = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else {
            glfwSetWindowShouldClose(window, true);
        }
    }

    // ✅ F - Puerta (ANTES del guard)
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !keyFPressed) {
        puertaAbierta = !puertaAbierta;
        puertaRotacionObjetivo = puertaAbierta ? -90.0f : 0.0f;
        keyFPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        keyFPressed = false;
    }

    // ✅ T - Día/Noche (ANTES del guard)
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !keyTPressed) {
        esDia = !esDia;
        keyTPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
        keyTPressed = false;
    }

    // ✅ C - Cámara libre/fija (ANTES del guard)
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !keyCPressed) {
        freeCamera = !freeCamera;
        if (!freeCamera) fixedCameraY = camera.Position.y;
        keyCPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
        keyCPressed = false;
    }

    // Guard: si el mouse no está capturado, no procesar movimiento
    if (!mouseCapturado) return;

    // TP a Miku (Tecla 1)
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !key1Pressed) {
        camera.Position = glm::vec3(miku_position.x, miku_position.y + 1.0f, miku_position.z + 2.5f);
        fixedCameraY = camera.Position.y;
        key1Pressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) {
        key1Pressed = false;
    }

    // TP al Policía (Tecla 2)
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

    // Altura (solo en modo libre)
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        if (!mouseCapturado) {
            mouseCapturado = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!mouseCapturado) return;

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
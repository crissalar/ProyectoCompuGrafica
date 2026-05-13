/*
*
* Proyecto final
*/

#include <iostream>
#include <stdlib.h>

// GLAD: Multi-Language GL/GLES/EGL/GLX/WGL Loader-Generator
// https://glad.dav1d.de/
#include <glad/glad.h>

// GLFW: https://www.glfw.org/
#include <GLFW/glfw3.h>

// GLM: OpenGL Math library
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

// Functions
bool Start();
bool Update();
void InitSounds();

// Definición de callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// Gobals
GLFWwindow* window;

// Tamaño en pixeles de la ventana
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// Definición de cámara (posición en XYZ)
Camera camera(glm::vec3(0.0f, 2.0f, 10.0f));

// Controladores para el movimiento del mouse
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Variables para la velocidad de reproducción
// de la animación
float deltaTime = 0.0f;
float lastFrame = 0.0f;

#pragma Transformaciones
// posiciones
//espositor 1
glm::vec3 expositor1_position(-1.0f, 6.98f, -1.0f);
//espositor 2
glm::vec3 expositor2_position(-14.8f, 6.98f, -1.4f);
//espositor 3
glm::vec3 expositor3_position(-7.6f, 6.98f, 1.4f);
//espositor 4
glm::vec3 expositor4_position(6.5f, 6.98f, 1.4f);
//policia
glm::vec3 policia_position(16.0f, 6.98f, 0.5f);
// miku
glm::vec3 miku_position(-1.0f, 14.8f, -1.0f);

//	Escalados
glm::vec3 scale_expositors(0.012f, 0.012f, 0.012f);           // expositor 1 (escala original)
glm::vec3 scale_expositors_small(0.0012f, 0.0012f, 0.0012f);  // policia
glm::vec3 scale_expositors_xs(0.0009f, 0.0009f, 0.0009f);     // expositor 2 y 3 (0.0012 * 0.75)
glm::vec3 scale_miku(0.0005f, 0.0005f, 0.0005f);

// globales
glm::vec3 forwardView(0.0f, 0.0f, 1.0f);
float     scaleV = 0.005f;
float     rotateCharacter = 0.0f;
float     rotateTable = 0.0f;

// Shaders
Shader* staticShader;
Shader* dynamicShader;

// Modelos estáticos
Model* house;
Model* escenario;

// Modelos dinámicos
AnimatedModel* expositor1;
AnimatedModel* expositor2;
AnimatedModel* expositor3;
AnimatedModel* policia;
AnimatedModel* miku;

// Audio
ISoundEngine* SoundEngine = createIrrKlangDevice();
ISound* musicaFondo = nullptr;

// Entrada a función principal
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

bool Start() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "FBX Model Loading", NULL, NULL);
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

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	glEnable(GL_DEPTH_TEST);

	dynamicShader = new Shader("Shader/09_vertex_skinning.vs", "Shader/09_fragment_skinning.fs");
	staticShader = new Shader("Shader/10_vertex_simple.vs", "Shader/10_fragment_simple.fs");

	dynamicShader->setBonesIDs(MAX_RIGGING_BONES);

	// Carga de modelos
	{
		house = new Model("models/Lobby/lobby.obj");
		escenario = new Model("models/Lobby/escenario.obj");

		miku = new AnimatedModel("models/Lobby/miku.fbx");
		expositor1 = new AnimatedModel("models/Lobby/expositor_1.fbx");
		expositor2 = new AnimatedModel("models/Lobby/expositor_2.fbx");
		expositor3 = new AnimatedModel("models/Lobby/expositor_3.fbx");
		policia = new AnimatedModel("models/Lobby/poli.fbx");
	}
	return true;
}

void InitSounds() {
	musicaFondo = SoundEngine->play2D("sound/bosque-120184.mp3", true, false, true);
}

bool Update() {
	float currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	processInput(window);

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
	glm::mat4 view = camera.GetViewMatrix();

	// Objetos estáticos
	{
		staticShader->use();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		staticShader->setMat4("projection", projection);
		staticShader->setMat4("view", view);

		// Casa
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		staticShader->setMat4("model", model);
		house->Draw(*staticShader);

		// Escenario
		glm::mat4 modelEscenario = glm::mat4(1.0f);
		modelEscenario = glm::translate(modelEscenario, glm::vec3(0.0f, 0.0f, 0.0f));
		modelEscenario = glm::rotate(modelEscenario, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelEscenario = glm::scale(modelEscenario, glm::vec3(1.0f, 1.0f, 1.0f));
		staticShader->setMat4("model", modelEscenario);
		escenario->Draw(*staticShader);
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

	// Expositor 1 (escala original)
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

	// Expositor 2 (escala reducida)
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

	// Expositor 3 (escala reducida)
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

	// Policia (escala reducida)
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

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP_DIR, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN_DIR, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		// rotateTable += 0.05f;
	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	{
		// rotateTable -= 0.05f;
	}
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
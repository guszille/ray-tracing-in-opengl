// Ray Tracing In OpenGL.

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "sources/graphics/vao.h"
#include "sources/graphics/vbo.h"
#include "sources/graphics/ibo.h"
#include "sources/graphics/shader.h"
#include "sources/graphics/texture.h"

#include "sources/utils/camera.h"
#include "sources/utils/debug.h"

// Global variables.
int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 720;

int OUTPUT_TEXTURE_WIDTH = 1280;
int OUTPUT_TEXTURE_HEIGHT = 720;

float FIELD_OF_VIEW = 45.0f;
float WINDOW_ASPECT_RATIO = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
float CAMERA_TRANSLATION_SPEED = 7.5f;
float CAMERA_SENSITIVITY = 0.05f;
float CURSOR_POS_X = (float)WINDOW_WIDTH / 2.0f;
float CURSOR_POS_Y = (float)WINDOW_HEIGHT / 2.0f;

bool CURSOR_ATTACHED = false;

float DELTA_TIME = 0.0f;
float LAST_FRAME = 0.0f;

float CURR_TIME = 0.0f;
float LAST_TIME = 0.0f;

unsigned int FRAMES_COUNTER = 0;

ShaderProgram* renderScreenQuadSP;
ShaderProgram* renderOutputTexSP;

Texture* outputTex;

VAO* quadVAO;
VBO* quadVBO;

Camera camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

// GLFW window callbacks.
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int key, int scanCode, int action, int mods);
void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos);
void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);

void processInput(GLFWwindow* window);

void getApplicationLimitations()
{
	int maxComputeWorkGroupCount[3];
	int maxComputeWorkGroupSize[3];
	int maxComputeWorkGroupInvocations;
	int maxVertexAttributes;

	for (int i = 0; i < 3; i++)
	{
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, i, &maxComputeWorkGroupCount[i]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, i, &maxComputeWorkGroupSize[i]);
	}

	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxComputeWorkGroupInvocations);

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttributes);

	std::cout << "-------------------------------" << std::endl;

	std::cout << "Compute OpenGL limitations:" << std::endl;

	std::cout << "\tMaximum number of work groups in X dimension: " << maxComputeWorkGroupCount[0] << std::endl;
	std::cout << "\tMaximum number of work groups in Y dimension: " << maxComputeWorkGroupCount[1] << std::endl;
	std::cout << "\tMaximum number of work groups in Z dimension: " << maxComputeWorkGroupCount[2] << std::endl;

	std::cout << "\tMaximum size of a work group in X dimension: " << maxComputeWorkGroupSize[0] << std::endl;
	std::cout << "\tMaximum size of a work group in Y dimension: " << maxComputeWorkGroupSize[1] << std::endl;
	std::cout << "\tMaximum size of a work group in Z dimension: " << maxComputeWorkGroupSize[2] << std::endl;

	std::cout << "\tNumber of invocations in a single local work group that may be dispatched to a compute shader: " << maxComputeWorkGroupInvocations << std::endl;

	std::cout << "Vertex OpenGL limitations:" << std::endl;

	std::cout << "\tMaximum number of vertex attributes: " << maxVertexAttributes << std::endl;
}

void setupApplication()
{
	float quadVertices[] = {
		// positions		 // texture coords
		-1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		-1.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		 1.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  0.0f,  1.0f,  0.0f
	};

	renderScreenQuadSP = new ShaderProgram("sources/shaders/render_screen_quad_vs.glsl", "sources/shaders/render_screen_quad_fs.glsl");
	renderOutputTexSP = new ShaderProgram("sources/shaders/render_output_tex_rt_cs.glsl");

	renderScreenQuadSP->bind();
	renderScreenQuadSP->setUniform1i("u_texture", 0);

	outputTex = new Texture(OUTPUT_TEXTURE_WIDTH, OUTPUT_TEXTURE_HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT);

	outputTex->bind(0);
	outputTex->bindImage(0, GL_READ_WRITE, GL_RGBA32F);

	quadVAO = new VAO();
	quadVBO = new VBO(quadVertices, sizeof(quadVertices));

	quadVAO->bind();
	quadVBO->bind();

	quadVAO->setVertexAttribute(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));
	quadVAO->setVertexAttribute(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	quadVAO->unbind();
	quadVBO->unbind();
}

void render(float currentFrame)
{
	renderOutputTexSP->bind();

	renderOutputTexSP->setUniform3f("u_view_position", camera.getPosition());
	renderOutputTexSP->setUniformMatrix4fv("u_view_matrix", camera.getViewMatrix());
	renderOutputTexSP->setUniform1f("u_fov", glm::radians(FIELD_OF_VIEW));

	glDispatchCompute((unsigned int)OUTPUT_TEXTURE_WIDTH, (unsigned int)OUTPUT_TEXTURE_HEIGHT, 1);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT); // Make sure writing to image has finished before read.

	renderOutputTexSP->unbind();

	glClearColor(0.25f, 0.5f, 0.25f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderScreenQuadSP->bind();
	quadVAO->bind();

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	quadVAO->unbind();
	renderScreenQuadSP->unbind();
}

void showFramesPerSecond(GLFWwindow* window)
{
	CURR_TIME = (float)glfwGetTime();
	FRAMES_COUNTER += 1;

	float delta = CURR_TIME - LAST_TIME;

	if (delta >= 1.0f / 30.0f)
	{
		std::string FPS = std::to_string((int)((1.0f / delta) * FRAMES_COUNTER));
		std::string ms = std::to_string((delta / FRAMES_COUNTER) * 1000.0f);
		std::string newTitle = "RT OpenGL - [" + FPS + " FPS / " + ms + " ms]";

		glfwSetWindowTitle(window, newTitle.c_str());

		LAST_TIME = CURR_TIME;
		FRAMES_COUNTER = 0;
	}
}

int main()
{
	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW!" << std::endl;

		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "RT OpenGL", NULL, NULL);

	if (!window)
	{
		std::cout << "Failed to create GLFW context/window!" << std::endl;
		glfwTerminate();

		return -1;
	}

	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetCursorPosCallback(window, cursorPositionCallback);
	glfwSetScrollCallback(window, scrollCallback);

	glfwSwapInterval(0);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD!" << std::endl;
		glfwTerminate();

		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	int contextFlags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);

	if (contextFlags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		std::cout << "OpenGL DEBUG context initialized!" << std::endl;

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

		glDebugMessageCallback(checkGLDebugMessage, NULL);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
	}

	getApplicationLimitations();
	setupApplication();

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = (float)glfwGetTime();

		DELTA_TIME = currentFrame - LAST_FRAME;
		LAST_FRAME = currentFrame;

		processInput(window);
		showFramesPerSecond(window);

		render(currentFrame);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	WINDOW_WIDTH = width;
	WINDOW_HEIGHT = height;

	WINDOW_ASPECT_RATIO = (float)width / (float)height;

	glViewport(0, 0, width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scanCode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) // Window should close.
	{
		glfwSetWindowShouldClose(window, true);
	}
}

void cursorPositionCallback(GLFWwindow* window, double xPos, double yPos)
{
	float x = (float)xPos;
	float y = (float)yPos;

	if (!CURSOR_ATTACHED)
	{
		CURSOR_POS_X = x;
		CURSOR_POS_Y = y;

		CURSOR_ATTACHED = true;
	}

	float xOffset = x - CURSOR_POS_X;
	float yOffset = CURSOR_POS_Y - y;

	CURSOR_POS_X = x;
	CURSOR_POS_Y = y;

	xOffset *= CAMERA_SENSITIVITY;
	yOffset *= CAMERA_SENSITIVITY;

	camera.processRotation(xOffset, yOffset);
}

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	FIELD_OF_VIEW = FIELD_OF_VIEW - (float)yOffset;
	FIELD_OF_VIEW = std::min(std::max(FIELD_OF_VIEW, 1.0f), 45.0f);
}

void processInput(GLFWwindow* window)
{
	float realCameraSpeed = CAMERA_TRANSLATION_SPEED * DELTA_TIME;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.processTranslation(Camera::Direction::FORWARD, realCameraSpeed);

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.processTranslation(Camera::Direction::BACKWARD, realCameraSpeed);

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.processTranslation(Camera::Direction::RIGHT, realCameraSpeed);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.processTranslation(Camera::Direction::LEFT, realCameraSpeed);
}

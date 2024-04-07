#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <assimp/cimport.h> 
#include <assimp/scene.h>
#include <assimp/postprocess.h> 

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "glm/ext.hpp"

#include <iostream>
#include <string>
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h>
#include <vector> 
#include <list>
#include <algorithm>

#include "src/Shader.h"
#include "src/Model.h"
#include "src/Utilities.h"

#include <limits>

GLFWwindow* window;
void WindowResizingHandler(GLFWwindow* window, int width, int height);
void FramebufferHandler(GLFWwindow* window, int width, int height);
void MouseButtonHandler(GLFWwindow* window, int button, int action, int mods);
void MouseHandler(GLFWwindow* window, double xposIn, double yposIn);
void processInput(GLFWwindow* window);

double lastFrameTime = glfwGetTime();
double currentTime;
float deltaTime = currentTime - lastFrameTime;
int frameCount = 0;

Camera camera;
Mouse mouse;
glm::mat4 model;

int main(void) {
	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return NULL;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);

	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello", NULL, NULL);
	if (!window)
	{
		std::cerr << "Failed to open GLFW window." << std::endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);

	glfwSetWindowSizeCallback(window, WindowResizingHandler);
	glfwSetCursorPosCallback(window, MouseHandler);
	glfwSetMouseButtonCallback(window, MouseButtonHandler);
	WindowResizingHandler(window, WINDOW_WIDTH, WINDOW_HEIGHT);

	if (glewInit())
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return NULL;
	}
	glfwSwapInterval(0);

	Shader phongShader("./src/shaders/phong.vert", "./src/shaders/phong.frag");


	//Setup IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");


#pragma region FrameBuffer

	Shader FBO_Shader("./src/shaders/fbo.vert", "./src/shaders/fbo_bilateral.frag");
	FBO_Shader.Activate();
	glUniform1i(glGetUniformLocation(FBO_Shader.ID, "screen_texture"), 0);

	float FBO_Vertices[] = {
		// Coords    // texCoords
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f,

		 1.0f,  1.0f,  1.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f
	};

	GLuint f_VAO, f_VBO;
	glGenVertexArrays(1, &f_VAO);
	glGenBuffers(1, &f_VBO);
	glBindVertexArray(f_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, f_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(FBO_Vertices), &FBO_Vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	GLuint FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	GLuint FBOTexture;
	glGenTextures(1, &FBOTexture);
	glBindTexture(GL_TEXTURE_2D, FBOTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBOTexture,0);


	GLuint RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);



#pragma endregion
	
	Object Vampire("Vampire");
	Vampire.AddMesh("src/3dobjects/vampire/vampire.dae");
	Vampire.Load();
	Vampire.AddBaseMap("src/3dobjects/vampire/vampire.png");

#pragma region Inspector Controls
	glm::vec3 finalPosition(0.0,0.0,0.0f);
	glm::vec3 finalScale(1.0f);
	glm::vec3 finalRotation(0.0f);
	static ImVec4 finalMeshColor = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); 

	//Light Information
	static ImVec4 LightColor = ImVec4(1.0, 1.0, 1.0, 1.0);
	static ImVec4 AmbientColor = ImVec4(1.0, 1.0, 1.0, 1.0);
	glm::vec3 LightPosition(0.0, 5.0, 0.0);
	static float LightPower = 1.0f;
	static float AmbientPower = 0.2f;
	static float DiffusePower = 0.2f;
	static float PointLightRadius = 1.f;

	//Post Processing Effects : 
	static float GaussianStrength = 4.f;
	
	//
	bool isAnimate = false;
	int lineNumber = 0;

#pragma endregion

	while (!glfwWindowShouldClose(window))
	{	
#pragma region Basic OpenGL Setup
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		currentTime = glfwGetTime();
		frameCount++;

		deltaTime = currentTime - lastFrameTime;
		if (currentTime - lastFrameTime >= 1.0)
		{
			glfwSetWindowTitle(window, std::to_string(frameCount).c_str());
			frameCount = 0;
			lastFrameTime = currentTime;
		}

		
#pragma endregion

		processInput(window);
		phongShader.Activate();

#pragma region Set Uniforms
		//Update Transforms
		//UpdateShader With Camera and Light Information
		glUniform3f(glGetUniformLocation(phongShader.ID, "LightPosition"), LightPosition.x, LightPosition.y, LightPosition.z);
		glUniform3f(glGetUniformLocation(phongShader.ID, "LightColor"), LightColor.x, LightColor.y, LightColor.z);
		glUniform3f(glGetUniformLocation(phongShader.ID, "AmbientColor"), AmbientColor.x, AmbientColor.y, AmbientColor.z);
		glUniform1f(glGetUniformLocation(phongShader.ID, "LightPower"), LightPower);
		glUniform1f(glGetUniformLocation(phongShader.ID, "AmbientPower"), AmbientPower);
		glUniform1f(glGetUniformLocation(phongShader.ID, "DiffusePower"), DiffusePower);
		glUniform1f(glGetUniformLocation(phongShader.ID, "PointLightRadius"), PointLightRadius);
		glUniform3f(glGetUniformLocation(phongShader.ID, "Color"), finalMeshColor.x, finalMeshColor.y, finalMeshColor.z);
#pragma endregion

		Vampire.Display(camera, phongShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		FBO_Shader.Activate();
		glUniform1f(glGetUniformLocation(FBO_Shader.ID, "GaussianStrength"), GaussianStrength);
		glBindVertexArray(f_VAO);
		glDisable(GL_DEPTH_TEST);
		glBindTexture(GL_TEXTURE_2D, FBOTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

#pragma region ImGui Window
		if (true) {
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::Begin("Inspector");

			if (ImGui::CollapsingHeader("Light")) {
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::SeparatorText("Basic");
				ImGui::DragFloat3("Light Position", glm::value_ptr(LightPosition), 1, -1000.0f, 1000.0f);
				ImGui::ColorEdit3("Color", (float*)&LightColor);
				ImGui::ColorEdit3("Ambient Light Color", (float*)&AmbientColor);
				ImGui::DragFloat("Power", &LightPower, 0.001, 0.0f, 1.0f);
				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::DragFloat("Ambient Light", &AmbientPower, 0.001, 0.0f, 1.0f);
				ImGui::DragFloat("Diffuse Light", &DiffusePower, 0.001, 0.0f, 1.0f);
				ImGui::DragFloat("Point Light Radius", &PointLightRadius, 0.01, 0.1f);
				ImGui::Spacing();
				ImGui::Spacing();
			}

			Vampire.AddToUI();

			ImGui::End();
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
		
#pragma endregion

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void MouseButtonHandler(GLFWwindow* window, int button, int action, int mods) {
}

void WindowResizingHandler(GLFWwindow* window, int width, int height)
{
	width = width;
	height = height;
	glViewport(0, 0, width, height);
}

void FramebufferHandler(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void MouseHandler(GLFWwindow* window, double xPos, double yPos) {
	int x = static_cast<int>(xPos);
	int y = static_cast<int>(yPos);

	mouse.x = x;
	mouse.y = y;
}

void processInput(GLFWwindow* window)
{
	float cameraSpeed = 0.01f;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.position += cameraSpeed * camera.front;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.position-= cameraSpeed  * camera.front;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.position -= glm::normalize(cross(camera.front, camera.up)) * cameraSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.position += glm::normalize(cross(camera.front, camera.up)) * cameraSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		camera.position += cameraSpeed * camera.up;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		camera.position -= cameraSpeed * camera.up;
	}

	camera.view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
}


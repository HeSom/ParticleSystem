#include "app.h"

#include <iostream>
#include <sstream>

#include "particleSystem.h"

#define SCROLL_SPEED 6.0f
#define ROTATION_SPEED 3.0f

glm::vec3 camera = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float timeSinceLastFrame = 0.0f;
float lastFrame = 0.0f;

App::App(GLFWwindow* window)
{
	this->window = window;
}

App* App::create()
{
	GLFWwindow* window;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Particle System", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "Failed to crea<te GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		std::cout << "Could not load OpenGL functions!" << std::endl;
		return nullptr;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glViewport(0, 0, WIDTH, HEIGHT);
	App* app = new App(window);
	return app;
}

void App::free(App* app)
{
	delete app;
}

int App::exec()
{
	Config config;
	ParticleSystem* system = new ParticleSystem(config);
	int frames = 0;
	float lastFPSdump = 0;
	while (!glfwWindowShouldClose(this->window))
	{
		++frames;
		float currentFrame = glfwGetTime();
		
		timeSinceLastFrame = currentFrame - lastFrame;
		lastFrame = currentFrame;
		if (currentFrame - lastFPSdump >= 1.0) {
			double fps = double(frames) / timeSinceLastFrame;
			std::stringstream ss;
			ss << "Particle System | " << fps << " FPS";

			glfwSetWindowTitle(window, ss.str().c_str());
			frames = 0;
			lastFPSdump = currentFrame;
		}
		processInput();
		system->update(timeSinceLastFrame);
		system->render(camera);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	delete system;
	glfwTerminate();
	return 1;
}

void App::processInput()
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	float cameraSpeed = timeSinceLastFrame * ROTATION_SPEED;

	glm::vec3 cameraRight = glm::normalize(glm::cross(glm::normalize(camera), glm::vec3(0.0f, 1.0f, 0.0f)));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, glm::normalize(camera)));

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
		camera = glm::vec4(camera, 1.0f) * glm::rotate(glm::mat4(1.0f), -cameraSpeed, cameraRight);
	
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera = glm::vec4(camera, 1.0f) * glm::rotate(glm::mat4(1.0f), +cameraSpeed, cameraRight);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera = glm::vec4(camera, 1.0f) * glm::rotate(glm::mat4(1.0f), cameraSpeed, cameraUp);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera = glm::vec4(camera, 1.0f) * glm::rotate(glm::mat4(1.0f), cameraSpeed, -cameraUp);
	
}

//Callback functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	float cameraSpeed = timeSinceLastFrame * SCROLL_SPEED * (float)yoffset;
	camera -= cameraSpeed * camera;
}



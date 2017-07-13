#pragma once

#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include "globals.h"
#include "glm\glm.hpp"
class App
{
private:
	App(GLFWwindow* window);
public:
	GLFWwindow* window;
	static App* create();
	static void free(App* app);
	void processInput();

	int exec();
};

//Callback functions

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
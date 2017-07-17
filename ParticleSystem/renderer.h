#pragma once
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include "globals.h"

class Renderer
{
private:
	GLuint vbo;
	GLuint vao;
	GLuint shaderProgram;

	GLuint viewMatrixLocation;
	GLuint projectionMatrixLocation;
	GLuint cameraPosLocation;
	GLuint screenWidthLocation;

	GLuint moonModelMatrixLocation;
	GLuint moonViewMatrixLocation;
	GLuint moonProjectionMatrixLocation;
	GLuint moonCameraLocation;
	GLuint moonColorLocation;

	size_t numberParticles;
	size_t moonElements;

	GLuint moonVBO;
	GLuint moonNormalsVBO;
	GLuint moonElementBuffer;
	GLuint moonVAO;
	GLuint moonShader;

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
public:
	Renderer(size_t numberParticles);
	int init();
	void cleanUp();
	void render(glm::vec3 camera, float moonRotation);
	GLuint getVBO() { return this->vbo; }
	size_t getNumberParticles() { return numberParticles; }
};
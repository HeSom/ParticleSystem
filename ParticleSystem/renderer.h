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
	GLuint moonLightPositionLocation;
	GLuint moonLightColorLocation;

	size_t numberParticles;
	size_t moonElements;

	GLuint moonVBO;
	GLuint moonNormalsVBO;
	GLuint moonElementBuffer;
	GLuint moonVAO;
	GLuint moonShader;

	glm::vec3 moonPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 lightPosition = glm::vec3(1.0f, -10.0f, 3.0f);
	glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
public:
	Renderer(size_t numberParticles);
	int init();
	void cleanUp();
	void render(glm::vec3 camera, float moonRotation);
	GLuint getVBO() { return this->vbo; }
	size_t getNumberParticles() { return numberParticles; }
	glm::vec3 getMoonPosition() { return moonPosition; }
};
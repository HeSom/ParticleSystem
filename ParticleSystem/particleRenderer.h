#pragma once
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include "globals.h"

class ParticleRenderer
{
private:
	GLuint vbo;
	GLuint vao;
	GLuint shaderProgram;

	GLuint viewMatrixLocation;
	GLuint projectionMatrixLocation;
	GLuint cameraPosLocation;
	GLuint screenWidhtLocation;

	size_t numberParticles;

	glm::vec4 color;

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
public:
	ParticleRenderer(glm::vec4 color);
	int init();
	void cleanUp();
	void render(glm::vec3 camera);
	GLuint getVBO() { return this->vbo; }
	size_t getNumberParticles() { return numberParticles; }
};
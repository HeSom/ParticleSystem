#include "particleRenderer.h"
#include "stdio.h"
#include "glslShader.h"
#include "glm\gtc\type_ptr.hpp"
#include <vector>

#define VERIFY(x) if(!x) {printf("Problem with compiling Shader in %s, line %d", __FILE__, __LINE__);return 1;}

std::vector<float> positions;
GLuint colorVBO;

ParticleRenderer::ParticleRenderer(glm::vec4 color)
{
	this->color = color;
}

double fRand(double fMin, double fMax)
{
	double f = (double)rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);
}

int ParticleRenderer::init()
{
	srand(42);
	for (float x = 1; x < 3; x += 0.5){
		for (float y = 1; y < 3; y += 0.5) {
			for (float z = 1; z < 3; z += 0.5) {
				positions.push_back(x + fRand(0.0f, 0.9f));
				positions.push_back(y + fRand(0.0f, 0.9f));
				positions.push_back(z + fRand(0.0f, 0.9f));
			}
		}
	}

	numberParticles = positions.size() / 3;

	std::vector<float> colors;
	for (int i = 0; i < numberParticles; ++i) {
		colors.push_back(fRand(0.0f, 1.0f));
		colors.push_back(fRand(0.0f, 1.0f));
		colors.push_back(fRand(0.0f, 1.0f));
		colors.push_back(1.0f);
	}

	//Shaders
	GLuint vertexShader = compileShader("vertexshader.glsl", GL_VERTEX_SHADER);
	GLuint fragmentShader = compileShader("fragmentshader.glsl", GL_FRAGMENT_SHADER);
	VERIFY(vertexShader);
	VERIFY(fragmentShader);
	this->shaderProgram = linkShaders(vertexShader, fragmentShader);
	VERIFY(this->shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//Buffers
	glGenVertexArrays(1, &(this->vao));
	glBindVertexArray(this->vao);

	//---------------positions--------------
	glGenBuffers(1, &(this->vbo));
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, positions.size()*sizeof(float), positions.data(), GL_DYNAMIC_COPY);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	
	glEnableVertexAttribArray(0);
	//-----------End of positions-----------

	//---------------colors-----------------
	glGenBuffers(1, &colorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	glEnableVertexAttribArray(1);

	//------------End of colors-------------
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Uniform locations
	viewMatrixLocation = glGetUniformLocation(shaderProgram, "view");
	projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projection");
	cameraPosLocation = glGetUniformLocation(shaderProgram, "cameraPos");
	screenWidhtLocation = glGetUniformLocation(shaderProgram, "screenWidth");


	return 0;
}

void ParticleRenderer::cleanUp()
{
	glDeleteProgram(this->shaderProgram);
	glDeleteBuffers(1, &(this->vbo));
	glDeleteBuffers(1, &colorVBO);
	glDeleteVertexArrays(1, &(this->vao));
}

void ParticleRenderer::render(glm::vec3 camera)
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(this->shaderProgram);
	glm::vec3 cameraDirection = glm::normalize(camera);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
	glm::mat4 view = glm::lookAt(camera, glm::vec3(0.0f, 0.0f, 0.0f), cameraUp);
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));
	glUniform3f(cameraPosLocation, camera.x, camera.y, camera.z);
	glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(this->proj));
	glBindVertexArray(this->vao);

	GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	GLint width = vp[3];
	glUniform1i(screenWidhtLocation, width);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glDrawArrays(GL_POINTS, 0, positions.size()/3);
	glBindVertexArray(0);
	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
}


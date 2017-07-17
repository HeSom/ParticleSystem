#include "renderer.h"
#include "stdio.h"
#include "glslShader.h"
#include "glm\gtc\type_ptr.hpp"
#include <vector>

#define VERIFY(x) if(!x) {printf("Problem with compiling Shader in %s, line %d", __FILE__, __LINE__);return 1;}

#include "sphere.h"

std::vector<float> positions;
GLuint colorVBO;

Renderer::Renderer(size_t numberParticles)
{
	this->numberParticles = numberParticles;
}

double fRand(double fMin, double fMax)
{
	double f = (double)rand() / RAND_MAX;
	return fMin + f * (fMax - fMin);
}

int Renderer::init()
{
	srand(42);
	for (int i = 0; i < numberParticles; ++i) {
		positions.push_back(fRand(0.5f, 1.5f));
		positions.push_back(fRand(0.5f, 1.0f));
		positions.push_back(fRand(0.5f, 1.5f));
	}

	//positions.push_back(1.0f);
	//positions.push_back(2.0f);
	//positions.push_back(1.0f);
	//positions.push_back(1.0f);
	//positions.push_back(1.0f);
	//positions.push_back(1.0f);

	/*positions.push_back(0.2f);
	positions.push_back(0.2f);
	positions.push_back(0.2f);
	positions.push_back(0.2f);
	positions.push_back(0.4f);
	positions.push_back(0.25f);*/
	

	numberParticles = positions.size() / 3;

	std::vector<float> colors;
	for (int i = 0; i < numberParticles; ++i) {
		colors.push_back(fRand(0.0f, 1.0f));
		colors.push_back(fRand(0.0f, 1.0f));
		colors.push_back(fRand(0.0f, 1.0f));
		colors.push_back(1.0f);
	}

	std::vector<float> spherePositions;
	std::vector<float> sphereNormals;
	std::vector<float> sphereTexcoords;
	std::vector<unsigned int> sphereIndices;
	geom_sphere(spherePositions, sphereNormals, sphereTexcoords, sphereIndices, 50, 20);
	this->moonElements = sphereIndices.size();
	glm::vec4 moonColor(1.0f, 1.0f, 1.0f, 1.0f);

	//Shaders
	GLuint vertexShader = compileShader("vertexshader.glsl", GL_VERTEX_SHADER);
	GLuint fragmentShader = compileShader("fragmentshader.glsl", GL_FRAGMENT_SHADER);
	VERIFY(vertexShader);
	VERIFY(fragmentShader);
	this->shaderProgram = linkShaders(vertexShader, fragmentShader);
	VERIFY(this->shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	GLuint moonVertexShader = compileShader("moonvertexshader.glsl", GL_VERTEX_SHADER);
	GLuint moonFragmentShader = compileShader("moonfragmentshader.glsl", GL_FRAGMENT_SHADER);
	VERIFY(moonVertexShader);
	VERIFY(moonFragmentShader);
	this->moonShader = linkShaders(moonVertexShader, moonFragmentShader);
	VERIFY(this->moonShader);
	glDeleteShader(moonVertexShader);
	glDeleteShader(moonFragmentShader);

	//Particles
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
	//End of particles

	//Moon
	//Buffers
	glGenVertexArrays(1, &(this->moonVAO));
	glBindVertexArray(moonVAO);

	//---------------positions--------------
	glGenBuffers(1, &(this->moonVBO));
	glBindBuffer(GL_ARRAY_BUFFER, this->moonVBO);
	glBufferData(GL_ARRAY_BUFFER, spherePositions.size() * sizeof(float), spherePositions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);
	//-----------End of positions-----------

	//---------------normals--------------
	glGenBuffers(1, &(this->moonNormalsVBO));
	glBindBuffer(GL_ARRAY_BUFFER, this->moonNormalsVBO);
	glBufferData(GL_ARRAY_BUFFER, sphereNormals.size() * sizeof(float), sphereNormals.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(float), nullptr);
	glEnableVertexAttribArray(1);
	//-----------End of normals-----------

	//---------------indices--------------
	glGenBuffers(1, &(this->moonElementBuffer));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->moonElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
	//-----------End of indices-----------

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Uniform locations
	viewMatrixLocation = glGetUniformLocation(shaderProgram, "view");
	projectionMatrixLocation = glGetUniformLocation(shaderProgram, "projection");
	cameraPosLocation = glGetUniformLocation(shaderProgram, "cameraPos");
	screenWidthLocation = glGetUniformLocation(shaderProgram, "screenWidth");

	moonModelMatrixLocation = glGetUniformLocation(moonShader, "model");
	moonViewMatrixLocation = glGetUniformLocation(moonShader, "view");
	moonProjectionMatrixLocation = glGetUniformLocation(moonShader, "projection");
	moonCameraLocation = glGetUniformLocation(moonShader, "cameraPos");
	moonColorLocation = glGetUniformLocation(moonShader, "color");

	//Enable gl functions
	glEnable(GL_DEPTH_TEST);

	return 0;
}

void Renderer::cleanUp()
{
	glDeleteProgram(this->shaderProgram);
	glDeleteBuffers(1, &(this->vbo));
	glDeleteBuffers(1, &colorVBO);
	glDeleteVertexArrays(1, &(this->vao));
}

void Renderer::render(glm::vec3 camera, float moonRotation)
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Render particles
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
	glUniform1i(screenWidthLocation, width);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glDrawArrays(GL_POINTS, 0, positions.size()/3);
	glBindVertexArray(0);
	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

	//render moon
	glUseProgram(moonShader);
	glm::mat4 modelMatrix(1.0f);
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(5.0f, 5.0f, 5.0f));
	modelMatrix = glm::rotate(modelMatrix, moonRotation, glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-5.0f, -5.0f, -5.0f));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(5.0f, -10.0f, 5.0f));
	glUniformMatrix4fv(moonModelMatrixLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(moonViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));
	glUniform3f(moonCameraLocation, camera.x, camera.y, camera.z);
	glUniformMatrix4fv(moonProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(this->proj));
	glUniform3f(moonColorLocation, 1.0f, 1.0f, 1.0f);
	glBindVertexArray(this->moonVAO);
	glDrawElements(GL_TRIANGLES, moonElements, GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
}


#pragma once

#include <iostream>
#include <vector>
#include <glm\glm.hpp>

#include "renderer.h"

struct Config
{
	int numberParticles;
	glm::vec4 particleColor = glm::vec4(0.0f, 0.8f, 1.0f, 1.0f);

};

class ParticleSystem
{
private:
	Renderer* renderer;
public:
	Config config;

	ParticleSystem(Config config)
	{
		this->config = config;
		renderer = new Renderer(this->config.numberParticles);
		renderer->init();
	}

	~ParticleSystem()
	{
		renderer->cleanUp();
		delete renderer;
	}
	void update(float timeSinceLastFrame);
	void render(glm::vec3 camera);
};
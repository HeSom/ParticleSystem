#pragma once

#include <iostream>
#include <vector>
#include <glm\glm.hpp>

#include "particleRenderer.h"

struct Config
{
	int numberParticles = 100*100*100;
	glm::vec4 particleColor = glm::vec4(0.0f, 0.8f, 1.0f, 1.0f);

};

class ParticleSystem
{
private:
	ParticleRenderer* renderer;
public:
	Config config;

	ParticleSystem(Config config)
	{
		this->config = config;
		renderer = new ParticleRenderer(this->config.particleColor);
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
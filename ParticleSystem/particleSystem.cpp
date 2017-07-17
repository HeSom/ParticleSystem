#include "particleSystem.h"
#include "kernels.h"

void ParticleSystem::update(float timeSinceLastFrame)
{
	simulate(renderer->getVBO(), renderer->getNumberParticles(), renderer->getMoonPosition(), timeSinceLastFrame);
}

void ParticleSystem::render(glm::vec3 camera, float moonRotation)
{
	renderer->render(camera, moonRotation);
}

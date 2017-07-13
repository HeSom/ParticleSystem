#include "particleSystem.h"
#include "kernels.h"

void ParticleSystem::update(float timeSinceLastFrame)
{
	integrate(renderer->getVBO(), renderer->getNumberParticles(), timeSinceLastFrame);
}

void ParticleSystem::render(glm::vec3 camera)
{
	renderer->render(camera);
}

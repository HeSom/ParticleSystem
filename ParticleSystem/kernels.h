#pragma once
#include <glad\glad.h>
#include "cuda_gl_interop.h"
#include "glm\glm.hpp"

void simulate(GLuint vbo, size_t numParticles, glm::vec3 moonPosition, float dt);
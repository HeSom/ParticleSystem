#pragma once
#include <glad\glad.h>
#include "cuda_gl_interop.h"

void integrate(GLuint vbo, size_t numParticles, float dt);
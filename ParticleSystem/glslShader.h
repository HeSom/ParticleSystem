#pragma once
#include <iostream>
#include <string>
#include <glad\glad.h>


GLuint compileShader(std::string path, GLenum shader_type);

GLuint linkShaders(GLuint vertexShader, GLuint fragmentShader);
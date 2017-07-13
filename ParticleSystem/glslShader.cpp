#include "glslShader.h"
#include <fstream>
#include <sstream>

void readFile(std::string path, std::string& content)
{
	std::ifstream t(path);
	std::stringstream buffer;
	buffer << t.rdbuf();
	content = buffer.str();
}

GLuint compileShader(std::string path, GLenum shader_type)
{
	GLuint shader = 0;

	shader = glCreateShader(shader_type);
	std::string src;

	readFile(path, src);
	const char* c_src = src.c_str();
	glShaderSource(shader, 1, &c_src, nullptr);
	glCompileShader(shader);

	//error handling
	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
		std::cout << "in " << path << std::endl;
		return 0;
	}
	return shader;
}

GLuint linkShaders(GLuint vertexShader, GLuint fragmentShader)
{
	GLuint shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	int success;
	char infoLog[512];
	glGetShaderiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
		std::cout << "ERROR::SHADER::LINKING_FAILED\n" << infoLog << std::endl;
		return 0;
	}
	return shaderProgram;
}

#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normal;

out vec3 vNormal;
out vec4 worldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
	worldPos = model * vec4(aPos, 1.0f);
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
	vec4 n = mat4(transpose(inverse(model))) * vec4(normal, 1.0f);
	vNormal = n.xyz;
}
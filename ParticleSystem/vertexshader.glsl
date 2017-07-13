#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;
uniform int screenWidth;

float pointRadius = 0.5;

void main()
{
	float cameraDistance = distance(aPos.xyz, cameraPos);
	float pointScale = (cameraDistance / 100);

	vec4 viewPos = view * vec4(aPos, 1.0f);

	vec4 projCorner = projection * vec4(0.5*pointRadius, 0.5*pointRadius, viewPos.z, viewPos.w);
	gl_Position = projection * viewPos;
	gl_PointSize = screenWidth * projCorner.x / projCorner.w;
}
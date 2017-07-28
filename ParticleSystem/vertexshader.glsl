#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 color;

out vec4 outColor;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;
uniform int screenWidth;

float pointDiameter = 0.12;

void main()
{
	vec4 viewPos = view * vec4(aPos, 1.0f);

	vec4 projCorner = projection * vec4(0.5*pointDiameter, 0.5*pointDiameter, viewPos.z, viewPos.w);
	gl_Position = projection * viewPos;
	gl_PointSize = screenWidth * projCorner.x / projCorner.w;
	outColor = color;
}
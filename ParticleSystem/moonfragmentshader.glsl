#version 330 core

in vec3 vNormal;
in vec4 worldPos;

out vec4 FragColor;

uniform vec3 color;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 cameraPos;

vec3 kd = vec3(0.5f, 0.5f, 0.5f);
vec3 ks = vec3(0.5f, 0.5f, 0.5f);
vec3 ambient = vec3(0.2f, 0.2f, 0.2f);
float n = 32.0f;

void main()
{
	vec3 normal = normalize(vNormal);
	vec3 l = normalize(lightPosition - worldPos.xyz);
	vec3 viewDir = normalize(cameraPos - worldPos.xyz);
	vec3 r = reflect(-l, normal);
	vec3 diffuse = lightColor * kd * max(dot(l, normal), 0);
	vec3 specular = lightColor * ks * pow(max(dot(r, viewDir), 0), n);

	FragColor = vec4((ambient + diffuse + specular) * color, 1.0f);
}
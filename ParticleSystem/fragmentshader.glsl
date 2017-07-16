#version 330 core
/*
See https://mmmovania.blogspot.de/2011/01/point-sprites-as-spheres-in-opengl33.html
to get spheres from GL_POINTS
*/
in vec4 outColor;
out vec4 FragColor;

//uniform vec4 color;

void main()
{
	vec3 normal;
	normal.xy = gl_PointCoord * 2.0 - vec2(1.0);
	float mag = dot(normal.xy, normal.xy);
	if(mag > 1.0f) discard;
	normal.z = sqrt(1.0f - mag);

	float diffuse = max(0.0f, dot(vec3(0.0f, 0.0f, 1.0f), normal)); //hard coded lightning direction :(


	FragColor = outColor * diffuse;
}
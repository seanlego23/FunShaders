#version 460

struct Camera {
	vec3 loc;
	vec3 lookAt;
	float fov;
};

uniform vec2 resolution;


out vec4 FragColor;
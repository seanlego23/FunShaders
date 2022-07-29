#version 460

#define PART_SKY	0
#define PART_SET	1
#define PART_INC	2

#define SQRT_2 		0.7071067812

struct Camera {
	vec3 loc;
	vec3 lookAt;
	vec3 up;
	vec3 right;
	float fov;
};

struct DirLight {
	vec3 dir;
	vec3 amb;
	vec3 diff;
	vec3 spec;
};

uniform vec2 resolution;
uniform float time;
uniform float elapsedTime;
uniform float zoom;
uniform float zoomRaw;

uniform Camera camera;

layout(binding = 0) uniform isampler2D part_tex;
layout(binding = 1) uniform sampler2D normal_tex;
layout(binding = 2) uniform isampler2D mask_tex;

out vec4 FragColor;

const vec3 sky = vec3(.53, .81, .92);
const vec3 black = vec3(0.0);
const vec3 bowl = vec3(0.5, 0.0, 0.0);

const vec3 up = vec3(0.0, 0.0, 1.0);
const DirLight light1 = DirLight(vec3(-SQRT_2, 0.0, SQRT_2), vec3(0.2), vec3(0.75), vec3(1.0));

vec3 getLightColor(in vec3 norm, in vec3 col) {
	vec3 ambient = 0.2 * col;
	float diff = max(dot(-light1.dir, norm), 0.0);
	vec3 diffuse = diff * col;
	vec3 viewDir = normalize(camera.lookAt - camera.loc);
	vec3 halfwayDir = normalize(light1.dir + viewDir);
	float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
	vec3 specular = vec3(1.0) * spec;
	return ambient + diffuse + specular;
}

void main() {	
	vec3 up = vec3(0.0, 0.0, 1.0);
	vec3 cd = normalize(camera.lookAt - camera.loc);
	vec3 cx = normalize(camera.right);
	vec3 cy = normalize(camera.up);
	mat4 view = mat4(cx, 0.0, cy, 0.0, cd, 0.0, 0.0, 0.0, 0.0, 1.0);
	
	vec2 p = gl_FragCoord.xy;
	int partID[9];
	int mask[9];
	partID[8] = texture(part_tex, p / resolution).x;
	mask[8] = texture(mask_tex, p / resolution).x;
	
	vec2 offset = vec2(-1.0);
	vec2 dOffset = vec2(1.0, 0.0);
	for (int i = 0; i < 8; i++, offset += dOffset) {
		partID[i] = texture(part_tex, (p + offset) / resolution).x;
		mask[i] = texture(mask_tex, (p + offset) / resolution).x;
		if (i % 2 == 0 && i > 0)
			dOffset = dOffset.yx;
		if (i % 4 == 0 && i > 0)
			dOffset = -dOffset;
	}

	vec3 partCol[9];
	for (int i = 0; i < 9; i++) {
		partCol[i] = black;
		if (partID[i] == PART_SKY)
			partCol[i] = sky;
		if (partID[i] == PART_INC) {
			partCol[i] = mask[i] == 1 ? bowl : sky;
		}
	}
	
	offset = vec2(-0.5);
	dOffset = vec2(0.5, 0.0);
	vec3 col = partCol[8];
	for (int i = 0; i < 8; i++, offset += dOffset) {
		col = (col * (i + 1) + getLightColor(texture(normal_tex, (p + offset) / resolution).xyz, mix(partCol[8], partCol[i], 0.5))) / (i + 2);
		if (i % 2 == 0 && i > 0)
			dOffset = dOffset.yx;
		if (i % 4 == 0 && i > 0)
			dOffset = -dOffset;
	}
	
	FragColor = vec4(col, 1.0);
}